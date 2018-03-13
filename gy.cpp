#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <exception>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
//deprecated -- to lurk for more (cf'd from Arduino lib?)
class GY80 {
public:

	GY80() {
		//Wire.begin()
		//m_init()
		//  m_set_scale(GY80_m_scale_8_1 = 7)
		//  100./230, 100./205 z
		write(dev_m, m_cfgB, 7<<5);
		//  m_set_mode(GY80_m_continuous)
		write(dev_m, m_mode, 0);
		m_scale = 100./230;
		m_scale_z = 100./205;
		//a_init()
		write(dev_a, a_pwr, 0);
		write(dev_a, a_pwr, 16);
		write(dev_a, a_pwr, 8);
		//  a_set_scale (scale_16 = 3)
		uint8_t* a_prev = read(dev_a, a_fmt, 1);
		uint8_t fmt = (*a_prev & ~(0xF)) | 11;
		write(dev_a, a_fmt, fmt);
		delete[] a_prev;
		// a_set_bw (bw_12_5 6.25 Hz bandwidth = 7)
		
		//g_init()
		//p_init()
	}
private:
//addresses
	static const uint8_t dev_m = 0x1E;
	static const uint8_t dev_a = 0x53;
	static const uint8_t dev_g = 0x69;
	static const uint8_t dev_p = 0x77;
//regs
	static const uint8_t m_cfgB = 0x01;
	static const uint8_t m_mode = 0x02;
	static const uint8_t a_pwr = 0x2D;
	static const uint8_t a_fmt = 0x31;
	float m_scale;// = 100./230;
	float m_scale_z;// = 100./205;
	void write(uint8_t device, uint8_t addr, uint8_t data) {
		//something
	}
	uint8_t* read (uint8_t device, uint8_t addr, size_t length) {
		uint8_t* ret = new uint8_t[length];
		//something
		return ret;
	}
};

class mexception : public std::exception {
private:
	const char* w;
public:
	mexception(const char* v) : w(v) {}
	virtual const char* what() const _GLIBCXX_USE_NOEXCEPT {return w;}
};

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

int main () {
	//gyro_test();
	return accel_test();
}
