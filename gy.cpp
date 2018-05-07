#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lora.h"
#include "mexception.h"
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
#include "data.h"

class IODev {
private:
	int fd;
	bool in_use; //TODO: if we go to multithread, here shall be MUTEX
public:
	IODev(const char* dev, int flags) {
		fd = open(dev, flags);
		if (fd < 0) {
			perror("Bus open error"); //TODO: make it into exception
			throw mexception(dev);
		}
		in_use = false;
	}
	bool used() const {return in_use;}
	int access() {
		if (!in_use) {
			in_use = true;
			return fd;
		}
		return 0;
	}
	void release() {in_use = false;}
};	

class I2CDev {
private:
	static IODev d;
	static uint8_t lastaddr;
	uint8_t a;
protected:
	uint8_t whoami_reg, whoami_val;
public:
	I2CDev(uint8_t addr) {
		a=addr;
	}
	//static void setI2C(IODev dev) { d = dev; }
	//TODO: multibyte read
	uint8_t read(uint8_t reg) { 
		uint8_t buf;
		mb_read(reg, 1, &buf);
		return buf;
	}
	void mb_read(uint8_t reg_st, uint8_t cnt, uint8_t* buf) {
		if (d.used()) throw mexception("No multithread");
		int fd = d.access();
		uint8_t s = reg_st; //data
		try {
			if (lastaddr != a)
				if (ioctl(fd, I2C_SLAVE, a) < 0)
					throw mexception("slave select");
			if (::write(fd,&s,1)!=1) //byte
				throw mexception("write");
			if (::read(fd,buf,cnt)!=cnt) //bytes
				throw mexception("read");
		}
		catch (mexception & e){ 
			perror(e.what());
			d.release();
			throw e;
		}
		d.release();
		//printf("read from %x#%x val %x", a,reg_st,*buf);
	}	
	//TODO: multibyte write
	void write(uint8_t reg, uint8_t val) { 
		mb_write(reg, 1, &val);
	}
	void mb_write(uint8_t reg_st, uint8_t cnt, uint8_t* buf) {
		if (cnt>128) throw mexception("Too much data");
		if (d.used()) throw mexception("No multithread");
		int fd = d.access();
		uint8_t s[128]; // = {reg, val}; //data
		s[0] = reg_st;
		::memcpy(s+1, buf, cnt);
		try {
			if (lastaddr != a)
				if (ioctl(fd, I2C_SLAVE, a) < 0)
					throw mexception("slave select");
			if (::write(fd,&s,cnt+1)!=cnt+1) //byte
				throw mexception("write");
		}
		catch (mexception & e){ 
			perror(e.what());
			d.release();
			throw e;
		}
		d.release();
	}
	
	
	virtual void setup() {
		if (read(whoami_reg)!=whoami_val) 
			throw mexception("Incorrect device");
	}
};
//TODO: think later on interfaces

//STM L3G4200D
class Gyro: /*public Gyroscope,*/ public I2CDev {
private:
	// limits ?
	// consts ?
	double rate;
public:
	Gyro() : I2CDev(0x69) {
		whoami_reg = 0xF; whoami_val = 0xD3;
	}
	uint8_t getTemp() {
		return read(0x26);
	}
	bool hasData() {
		return ((read(0x27) & 0x8) == 8);
	}
	void fetchData(double* arr) {
		//todo: mb-read
		int16_t x = read(0x29)<<8 | read(0x28);
		int16_t y = read(0x2B)<<8 | read(0x2A);
		int16_t z = read(0x2D)<<8 | read(0x2C);
		//printf("%x %x %x",x,y,z);
		arr[0] = x*rate;
		arr[1] = y*rate;
		arr[2] = z*rate;
		
	}
	virtual void setup() {
		I2CDev::setup();
		write(0x20, 0xF); //CTRL_REG_1 -- ODR 100 hz, CutOff 12.5, all on
		rate = 8.75l; //default 250 dps
	}
};

//ADXL345
//notes:
// Freefall, tap thresholds & events [mb use, mb test?]
class Accel: /*public Accelerometer,*/ public I2CDev {
private:
	//limits: 2..16 g
	//noise: pm40; static pm 250 
	//TODO: note on diff 'tw XY and Z
	double resol; //256 LSB per g
	// I2C -> max for fast 800 Hz, slow 200 Hz
	double rate; // .1 Hz -> 3200 Hz; <6.3 & >1600 - diff; 
public:
	Accel(): I2CDev(0x53) {
		whoami_reg = 0; whoami_val = 0xE5;
	}
	// gettemp()
	void fetchData(double* arr) {
		int16_t raw[3];
		mb_read(0x32, 6, (uint8_t*)(&raw));
		for (int i = 0; i<3; i++) arr[i]=raw[i]*resol;
	}
	bool hasData() {
		return ((read(0x30) & 0x80) == 0x80);
	}
	/// avg data in mg [milli-*g*]
	void setOffsets(double x, double y, double z) {
		double scale = 15.6; // mg/LSB
		int8_t dx = (int8_t)round(-x/scale);
		int8_t dy = (int8_t)round(-y/scale);
		int8_t dz = (int8_t)round(-z/scale);
		// write to OFSX, OFSY, OFSZ
		write (0x1E, dx);
		write (0x1F, dy);
		write (0x20, dz);
	}
	virtual void setup() {
		//selftest?
		I2CDev::setup();
		write(0x2D, 0); //POWER_CTL -> standby
		write(0x2C, 0x0B); //BW_RATE reg; [t7, t8 - hz]
		rate = 200; //mb int? but 6.25 Hz and lower..
		write(0x31, 9); //DATA_FORMAT -> [self-test 0x80? 8 - full-res]
						// 0x1 -- +-4g
		write(0x38, 0);
		
		write(0x2D, 8); //POWER_CTL -> measure
		resol = 1000./256.; //mid.				
	}
};

//HMC5883L
class Magnet: /*public Magnetometer, */ public I2CDev {
private:
	//limits: 1..8 gauss
	double resol; //230 .. 1730 LSb / gauss; default 0.92 mG / LSb
	double rate; //default 15 Hz
	//res: 1-2 deg accuracy
public:
	Magnet(): I2CDev(0x1E) {
		whoami_reg = 0xA; whoami_val = 'H'; //IRA
	}
	void fetchData(double* arr) {
		//DRXA - MSB, DRXB - LSB
		int16_t raw[3];
		mb_read(0x03, 6, (uint8_t*)&raw);
		arr[0] = (int16_t)(__builtin_bswap16(raw[0]))*resol;
		arr[1] = (int16_t)(__builtin_bswap16(raw[2]))*resol;
		arr[2] = (int16_t)(__builtin_bswap16(raw[1]))*resol;
		
	}
	bool hasData() {
		return ((read(0x9) & 1) == 1);
	}
	virtual void setup() {
		I2CDev::setup();
		//TODO: IRB, IRC?
		write(0x2, 0); //Continuous Measurement Mode
		resol = 0.92;
		rate = 15;
	}
};

//BMP085
//TODO: LATER (MAYBE)
class Baro: /*public Barometer, */ public I2CDev {
private:
	//limits: 300-1100 hPa (+9..-0.5km)
public:
	Baro() : I2CDev(0x77) {
		//whoami_reg
	}
};

IODev I2CDev::d("/dev/i2c-1", O_RDWR);
uint8_t I2CDev::lastaddr;

int gyro_test() {
	//I2CDev::setI2C(IODev("/dev/i2c-1", O_RDWR));
	Gyro gyro;
	gyro.setup();
	double x, y, z, xe, ye, ze;
	x=y=z=0;
	double deltas[3];
	gyro.fetchData(deltas);
	printf("X: %.2f\nY: %.2f\nZ: %.2f\n", deltas[0]*0.001, deltas[1]*0.001, deltas[2]*0.001);
	xe = deltas[0]; ye = deltas[1]; ze = deltas[2];
	printf("Temp: %d\n", gyro.getTemp());
	while (true) {
	for (int i = 0; i < 10; i++){
		while (!gyro.hasData());
		gyro.fetchData(deltas);
		if (fabs(deltas[0])>10000)
			x += (deltas[0]-xe)*0.01; //100Hz
		if (fabs(deltas[1])>10000)
			y += (deltas[1]-ye)*0.01;
		if (fabs(deltas[2])>10000)
			z += (deltas[2]-ze)*0.01;
	}
	printf("X: %.2f\tY: %.2f\tZ: %.2f\n", x*0.001, y*0.001, z*0.001);
}
	return 0;
}

//TODO: Vec3D or sth like this

int accel_test() {
	// 0.1 sec of data [cf Hz]
	// avg -> minus -> setOffset
	Accel accel;
	accel.setup();
	double deltas[3];
	double avgs[3];
	//double xe, ye, ze;
	for (int i = 0; i < 20; i++) {
		while (!accel.hasData());
		accel.fetchData(deltas);
		for (int j = 0; j < 3; j++) avgs[j] += deltas[j];
	}
	for (int j = 0; j < 3; j++) avgs[j] /= 20.;
	accel.setOffsets(avgs[0], avgs[1], avgs[2]-1);
	while (true) {
		for (int i = 0; i<200; i++){
			while (!accel.hasData());
			accel.fetchData(deltas);
		}
		printf("X: %.2fg\tY: %.2fg\tZ: %.2fg\n", deltas[0]*0.001, deltas[1]*0.001, deltas[2]*0.001);
	}
	return 0;
}

int magnet_test() {
	// 0.1 sec of data [cf Hz]
	// avg -> minus -> setOffset
	Magnet magnet;
	magnet.setup();
	double deltas[3];
	while (true) {
		for (int i = 0; i<5; i++){
			while (magnet.hasData());
			while (!magnet.hasData());
			magnet.fetchData(deltas);
		}
		printf("X: %.2fGa\tY: %.2fGa\tZ: %.2fGa\n", deltas[0]*0.001, deltas[1]*0.001, deltas[2]*0.001);
	}
	return 0;
}


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



DataPkg buildPacket() {
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
	}
	ret.time = time(NULL);
	
	return ret;
}

std::atomic<bool> radio_stop;
const uint8_t radioDst = RCVR_ADDR;
//std::mutex mRadioPkg;

int radio_thread() {
	LoRa* lora;
	lora = new LoRa("/dev/spidev0.0", SNDR_ADDR);
	printf("LoRa Version: %d\n", lora->getVersion());
	char buf[252];
	while (!radio_stop) {
		//std::unique_lock<std::mutex> lk(mRadioPkg);
		DataPkg pkg = buildPacket();
		size_t slen = pkg.toBytes(buf, 252);
		if (lora->sendPacketTimeout(radioDst, buf, slen, 3)) {
			printf("Packet sent successfully\n");
		} else {
			printf("Packet send failure\n");
		}
		delay(1000);
	}
	return 0;
}

void INThandler(int sig){
  printf("Bye.\n");
  gps_stop = true;
 // exit(0);
}

int main () {
	//return gyro_test();
	//return magnet_test();
	//return radio_test();
	signal(SIGINT, INThandler);
	std::thread t1(gps_thread);
	std::thread t2(radio_thread);
	while (!gps_stop) {
		if (!gps_hasData) {
			std::cout << "NO FIX\n";
			while (!gps_hasData && !gps_stop) {
				delay(1000);
			}
		}
		if (gps_new) {
			std::unique_lock<std::mutex> lk(mGpsFix);
			std::cout << fix.latitude << ", " << fix.longitude << ", ";
			std::cout << fix.altitude << "\n";
			gps_new = false;
		}
		delay(1000);
	}
	t1.join();
	return 0;	
}
