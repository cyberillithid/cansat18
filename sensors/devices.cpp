
#include <exception>
//linux IO [open, ...]
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// ioctl (i2c magic)
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
//stderr
#include <errno.h>
#include <stdio.h>
//memcpy
#include <string.h>

#include "devices.h"

IODev::IODev(const char* dev, int flags) {
	fd = open(dev, flags);
		if (fd < 0) {
			perror("Bus open error"); //TODO: make it into exception
			throw std::runtime_error(dev);
		}
}

int IODev::access(mtx_lock* lock) {
	//printf("%d %x %x", lock->owns_lock(), lock->mutex(), &mutx);
	if (lock->owns_lock() && (lock->mutex() == &mutx)) return fd;
	return 0;
}

I2CBus::I2CBus(const char* dev):
	IODev(dev, O_RDWR), last_addr(0) {}
	
//TODO: make it error-resistant (exception hierarchy?)
mtx_lock* I2CBus::use(uint8_t addr) {
	mtx_lock* lk = new mtx_lock(mutx);
	if (last_addr != addr) {
		int fd = access(lk);
		if (fd == 0) 
			throw std::runtime_error("Multithreading error");
		if (ioctl(fd, I2C_SLAVE, addr) < 0) 
			throw std::runtime_error("Slave select error");
	}
	return lk;
}

I2CDev::I2CDev(I2CBus& bus, uint8_t addr) : i2cbus(bus), dev_addr(addr) {
	lk = nullptr;
}

void I2CDev::start() {
	if (lk != nullptr) return;
	lk = i2cbus.use(dev_addr);
	fd = i2cbus.access(lk);
}

void I2CDev::stop() {
	if (lk == nullptr) return;
	fd = 0;
	delete lk;
	lk = nullptr;
}

void I2CDev::setup() {
	if (b_read(whoami_reg) != whoami_val)
		throw std::runtime_error("Incorrect device");
}
void I2CDev::mb_read(uint8_t reg_st, uint8_t cnt, uint8_t* buf) {
	bool clr = (lk == nullptr);
	if (clr) start();
	uint8_t s = reg_st;
	try {
		if (write(fd, &s, 1) != 1)
			throw std::runtime_error("I2C register select error");
		if (read(fd,buf,cnt) != cnt)
			throw std::runtime_error("I2C multibyte read error");
	} catch (std::exception& e) {
		//printf("FD %d", fd);
		perror(e.what());
		if (clr) stop();
		throw e;
	}
	if (clr) stop();
}
void I2CDev::mb_write(uint8_t reg_st, uint8_t cnt, uint8_t* buf) {
	if (cnt > 128)
		throw std::length_error("Too long I2C multibyte write request");
	uint8_t tbuf[130];
	tbuf[0] = reg_st;
	memcpy(tbuf+1,buf,cnt);
	bool clr = (lk == nullptr);
	if (clr) start();
	try {
		if (::write(fd, tbuf, cnt+1) != cnt+1)
			throw std::runtime_error("I2C multibyte write error");
	} catch (std::exception& e) {
		perror(e.what());
		if (clr) stop();
		throw e;
	}
	if (clr) stop();
}
uint8_t I2CDev::b_read(uint8_t reg) {
	uint8_t buf;
	mb_read(reg, 1, &buf);
	return buf;
}
void I2CDev::b_write(uint8_t reg, uint8_t val) {
	mb_write(reg, 1, &val);
}
