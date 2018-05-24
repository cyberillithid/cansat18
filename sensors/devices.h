#pragma once
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <thread> 

typedef std::unique_lock<std::mutex> mtx_lock;

/// A small utility class for linux io-devices for multithreaded use
class IODev {
private:
	int fd; /// File descriptor
public:
	IODev (const char* dev, int flags); /// cf. man 2 open
	std::mutex mutx;
	int access(mtx_lock* lock); /// claim access
};

class I2CBus : public IODev {
private:
	uint8_t last_addr;
public:
	I2CBus (const char* dev);
	mtx_lock* use(uint8_t addr);
};

class I2CDev {
private:
	I2CBus& i2cbus;
	uint8_t dev_addr;
	mtx_lock* lk;
	int fd;
protected:
	uint8_t whoami_reg, whoami_val;
public:
	I2CDev(I2CBus& bus, uint8_t addr);
	virtual void setup(); //TODO: rename to verify
	
	void start();
	void stop();
	
	void mb_read(uint8_t reg_st, uint8_t cnt, uint8_t* buf);
	void mb_write(uint8_t reg_st, uint8_t cnt, uint8_t* buf);
	uint8_t b_read(uint8_t reg);
	void b_write(uint8_t reg, uint8_t val);
};
