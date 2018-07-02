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
#include "sensors/DHT22.h"

#ifdef ALUMEN
#define HASDS false
#else
#define HASDS true
#endif

std::atomic<bool> gps_stop, gps_new, gps_hasData;
std::mutex mGpsFix;
gps_fix_t fix;

std::atomic<uint32_t> dhtData;

void dht_thread(){
	DHT22 dht(2); //wPi pin 2 == BCM 27 == pin 13
	while (!gps_stop) {
		if (dht.fetch())
			dhtData = __builtin_bswap32(dht.get());
	}
}

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

uint32_t fetchTempDS(){
	if (!HASDS) return 0;
	FILE *devFile;
	uint32_t T;
	devFile = fopen ("/sys/bus/w1/devices/28-80000000eb1a/w1_slave", "r");
	if (devFile == NULL)
	return 0; //print some message
	char crcConf[5];
	fscanf(devFile, "%*x %*x %*x %*x %*x %*x %*x %*x %*x : crc=%*x %s", crcConf);
	if (strncmp(crcConf, "YES", 3) == 0)
		fscanf(devFile, "%*x %*x %*x %*x %*x %*x %*x %*x %*x t=%5d", &T);
	fclose(devFile);
	return T;
}

Satellite::Satellite(bool isLo) : isLoHF(isLo), i2cbus("/dev/i2c-1"),
		magneto(i2cbus),
		accel(i2cbus), gyro(i2cbus), baro(i2cbus),
		thrGPS(gps_thread), thrDHT(dht_thread),
		bat(i2cbus, 0x36)
{
	radio_stop=false;
	accel.setup();
	magneto.setup();
	thrRadio = new std::thread(&Satellite::radio_thread, this);
	thrSens = new std::thread(&Satellite::sensors_thread, this);
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
	thrGPS.join();
	thrRadio->join();
	thrDHT.join();
	thrSens->join();
	delete thrRadio;
	delete thrSens;
}

/*
Gyro: 6b per 100 Hz
*/

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
	ret.tempDS = fetchTempDS();

	baro.fetchData();
	ret.pressure = baro.getPress();
	ret.bmpTemp = baro.getTemp();
	magneto.fetchData(&ret.magn);

	ret.accel = acc;
	
	uint16_t volt, cap;
	bat.mb_read(2, 2, (uint8_t*)&volt);
	bat.mb_read(4, 2, (uint8_t*)&cap);
	ret.battery = (uint32_t)(__builtin_bswap16(cap)) << 16 | __builtin_bswap16(volt);
	//printf(stderr, "%x\n", ret.battery);
	//printf("\t%.2f\n", ret.gpstime);
	return ret;
}

const uint8_t radioDst = RCVR_ADDR;


int Satellite::sensors_thread() {
	FILE* accfile = fopen("~/acc.dat", "wb");
	FILE* gyrfile = fopen("~/gyr.dat", "wb");
	Timed3D data;
	while (!radio_stop) {
		clock_gettime(CLOCK_REALTIME, &(data.ts));
		if (accel.hasData())
		{
			accel.fetchData(&(data.data));
			acc = data.data;
			fwrite(&data, sizeof(Timed3D), 1, accfile);
		}
		if (gyro.hasData()) {
			gyro.fetchData(&(data.data));
			fwrite(&data, sizeof(Timed3D), 1, gyrfile);
		}
	}
	fclose(accfile);
	fclose(gyrfile);
	return 0;
}

int Satellite::radio_thread() {
	LoRa* lora;
	FILE* logfile = fopen("~/radio.log", "wb");
	if (isLoHF)
		lora = new LoRa("/dev/spidev0.0", SNDR_ADDR, CH_433_80);
	else
		lora = new LoRa("/dev/spidev0.0", SNDR_ADDR, CH_866_80);
	printf("LoRa Version: %d\n", lora->getVersion());
	time_t t = time(NULL);
	fprintf(stderr, "Started work at %s", ctime(&t));
	fflush(stdout);
	char buf[252];
	int cnt = 0;
	while (!radio_stop) {
		//std::unique_lock<std::mutex> lk(mRadioPkg);
		DataPkg pkg = buildPacket();
		size_t slen = pkg.toBytes(buf, 252);
		fwrite(&pkg, sizeof(DataPkg), 1, logfile);
		if (lora->sendPacketTimeout(radioDst, buf, slen, 4000)) {
			//printf("Packet sent successfully\n");
			cnt++;
		} else {
			t = time(NULL);
			fprintf(stderr, "Packet send failure after %d successes at %s", cnt, ctime(&t));
			cnt=0;
		}
		usleep(100000);
	}
	fclose(logfile);
	return 0;
}
