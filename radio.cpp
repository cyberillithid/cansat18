/*
 * radio.cpp
 * 
 */

#include <iostream>
#include <stdio.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "mexception.h"

void pthrow(char* s) {
	perror(s);
	throw mexception(s);
}

class LoRa {
	//SPIDevice [implement later?]
private:
	int fd;
	
	static uint8_t mode = SPI_MODE_0;
	static uint8_t bits = 8;
	static uint32_t speed = 500000;
	static uint16_t S_delay;
	
	void spiInit(){
		int ret;
		/*
		 * spi mode
		 */
		ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
		if (ret == -1)
			pabort("can't set spi mode");

		ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
		if (ret == -1)
			pabort("can't get spi mode");

		/*
		 * bits per word
		 */
		ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
		if (ret == -1)
			pabort("can't set bits per word");

		ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
		if (ret == -1)
			pabort("can't get bits per word");

		/*
		 * max speed hz
		 */
		ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
		if (ret == -1)
			pabort("can't set max speed hz");

		ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
		if (ret == -1)
			pabort("can't get max speed hz");

		printf("spi mode: %d\n", mode);
		printf("bits per word: %d\n", bits);
		printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	}
	
	uint8_t readReg(uint8_t reg){
		uint8_t tx[] = {reg, 0};
		uint8_t rx[] = {0, 0};
		transfer(fd, tx, 2, rx);
		return rx[1];
	}
	
	void writeReg(uint8_t reg, uint8_t val) {
		uint8_t tx[] = {reg | 0x80, val};
		uint8_t rx[] = {0,0};
		transfer(fd, tx, 2, rx);
	}
	
public:
	LoRa(const char* devAddr) { //"spidev1.0"
		fd = open(devAddr, O_RDWR);
		if (fd < 0)
			pthrow("can't open device");
		spiInit(fd);
	}
	uint8_t getVersion() {
		return readReg(0x42);
	}
};

void INThandler(int sig){
  printf("Bye.\n");
  exit(0);
}

void setup() {
	LoRa lora("/dev/spidev0.0");
	printf("LoRa Version: %d\n", lora.getVersion());
}

void mainloop() {
	
}

int main(int argc, char *argv[])
{
	//parse options

	// we catch the CTRL-C key
	signal(SIGINT, INThandler);
	setup();
	mainloop();
	
	return 0;
}

