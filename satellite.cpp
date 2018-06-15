#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "radio/lora.h"

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <libgpsmm.h>

#include <math.h>
#include <csignal>
#include <thread>
#include <deque>
#include <atomic>
#include <mutex>
#include "radio/data.h"
#include "satellite.h"

std::atomic<bool> gps_stop, gps_new, gps_hasData;
std::mutex mGpsFix;
gps_fix_t fix;

int gps_thread() {
    gpsmm gps_rec("localhost", DEFAULT_GPSD_PORT);
	gps_stop = false;
	gps_new = false;
	gps_hasData = false;
    if (gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
        std::cerr << "No GPSD running.\n";
        gps_stop = true;
        return 1;
    }
	
    while (!gps_stop) {
        struct gps_data_t* newdata;

        if (!gps_rec.waiting(50000000))
          continue;

        if ((newdata = gps_rec.read()) == NULL) {
            std::cerr << "Read error.\n";
            //return 1;
        } else {
			if ((newdata->status == 0) || (newdata->fix.mode < 2)) {
				gps_hasData = false;
			} else {
				gps_new = true;
				gps_hasData = true;
				//std::cout << "FIX: ";
				std::unique_lock<std::mutex> lk(mGpsFix);
				fix = newdata->fix;
			}
            //PROCESS(newdata);
        }
    }
    return 0;
}

uint32_t fetchTemp() {
	FILE *temperatureFile;
	uint32_t T;
	temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
	if (temperatureFile == NULL)
	return 0; //print some message
	fscanf (temperatureFile, "%uld", &T);
	fclose (temperatureFile);
	return T;
}

Satellite::Satellite() : i2cbus("/dev/i2c-1"),
		magneto(i2cbus),
		accel(i2cbus), gyro(i2cbus), baro(i2cbus),
		t1(gps_thread)
{
	radio_stop=false;
	accel.setup();
	magneto.setup();
	t2 = new std::thread(&Satellite::radio_thread, this);
}
void Satellite::loop() {
		while (!gps_stop) {
		if (!gps_hasData) {
			std::cout << "NO FIX\n";
			while (!gps_hasData && !gps_stop) {
				usleep(10000);
			}
		}
		if (gps_new) {
			std::unique_lock<std::mutex> lk(mGpsFix);
			std::cout << fix.latitude << ", " << fix.longitude << ", ";
			std::cout << fix.altitude << "; @" << time(NULL) << "+" << fix.time - time(NULL) << "\n";
			gps_new = false;
		}
		usleep(10000);
	}
}
Satellite::~Satellite() {
	t1.join();
	t2->join();
	delete t2;
}

DataPkg Satellite::buildPacket() {
	DataPkg ret;
	if (gps_hasData) {
		std::unique_lock<std::mutex> lk (mGpsFix);
		ret.gps_mode = fix.mode;
		ret.lat = fix.latitude;
		ret.lon = fix.longitude;
		ret.alt = fix.altitude;
		ret.speed = fix.speed;
		ret.climb = fix.climb;
		ret.gpstime = fix.time;
	} else {
		ret.gps_mode = 0;
		ret.lat = ret.lon = ret.alt = ret.speed = ret.climb = NAN;
		ret.gpstime = 0;
	}
	ret.time = time(NULL);
	ret.temp = fetchTemp();
	baro.fetchData();
	ret.pressure = baro.getPress();
	ret.bmpTemp = baro.getTemp();
	magneto.fetchData(&ret.magn);
	while (!accel.hasData());
	accel.fetchData(&ret.accel);
	//printf("\t%.2f\n", ret.gpstime);
	return ret;
}

const uint8_t radioDst = RCVR_ADDR;

int Satellite::radio_thread() {
	LoRa* lora;
	lora = new LoRa("/dev/spidev0.0", SNDR_ADDR);
	printf("LoRa Version: %d\n", lora->getVersion());
	char buf[252];
	while (!radio_stop) {
		//std::unique_lock<std::mutex> lk(mRadioPkg);
		DataPkg pkg = buildPacket();
		size_t slen = pkg.toBytes(buf, 252);
		if (lora->sendPacketTimeout(radioDst, buf, slen, 4000)) {
			printf("Packet sent successfully\n");
		} else {
			printf("Packet send failure\n");
		}
		usleep(100000);
	}
	return 0;
}
