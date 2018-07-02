#include "Baro_BMP085.h"
#include <unistd.h>

Baro_BMP085::Baro_BMP085(I2CBus& bus) : I2CDev(bus, 0x77) {
		whoami_reg=0xD0; whoami_val=0x55;
		//TODO: move it to setup
		int16_t raw[11];
		mb_read(0xAA, 22, (uint8_t*)(&raw));
		for (int i = 0; i<3; i++)
			AC1[i] = (int16_t)(__builtin_bswap16(raw[i]));
		for (int i = 0; i<3; i++)
			AC4[i] = (int16_t)(__builtin_bswap16(raw[i+3]));
		B1 = (int16_t)(__builtin_bswap16(raw[6]));
		B2 = (int16_t)(__builtin_bswap16(raw[7]));
		MB = (int16_t)(__builtin_bswap16(raw[8]));
		MC = (int16_t)(__builtin_bswap16(raw[9]));
		MD = (int16_t)(__builtin_bswap16(raw[10]));
		temp=0; pres=0;
		oss = 0;
	}
bool Baro_BMP085::fetchData() {
	b_write(0xF4, 0x2E);
	usleep(5000);
	uint8_t arr[4];
	mb_read(0xF6, 2, arr);
	int16_t UT = (int16_t)(__builtin_bswap16(*(uint16_t*)arr));
	b_write(0xF4, 0x34 + (oss<<6));
	usleep(1000*(2 + (3 << oss)));
	mb_read(0xF6, 3, arr+1);
	arr[0] = 0;
	int32_t UP = ((int32_t)(__builtin_bswap32(*(uint32_t*)arr))) >> (8-oss);
	
	long X1 = ((UT - AC4[2]) * AC4[1]) >> 15;
	long X2 = (MC << 11) / (X1 + MD);
	long B5 = X1 + X2;
	temp = (B5 + 8) >> 4;
	
	long B6 = B5 - 4000;
	X1 = (B2 * ( (B6 * B6) >> 12) )>>11;
	X2 = (AC1[1] * B6) >> 11;
	long X3 = X1 + X2;
	long B3 = ( (((AC1[0] * 4) + X3) << oss )+2) / 4;
	
	X1 = (AC1[2] * B6) >> 13;
	X2 = (B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	unsigned long B4 = (AC4[0] * (uint32_t)(X3 + 32768)) >> 15;
	unsigned long B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oss );
	long p;
	if (B7 < 0x80000000) {
		p = (B7 * 2) / B4;
	} else {
		p = (B7 / B4) * 2;
	}
	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;
	pres = p + ((X1 + X2 + (int32_t)3791)>>4);
	return true;
}
long Baro_BMP085::getTemp() const {return temp;}
long Baro_BMP085::getPress() const {return pres;}

/*


int baro_test() {
	// 0.1 sec of data [cf Hz]
	// avg -> minus -> setOffset
	Baro baro;
	printf("Config done\n");
	while (true) {
		baro.fetchData();
		printf("Pressure %ld Pa, temperature %ld dC\n", baro.getPress(), baro.getTemp());
		fflush(stdout);
		//std::cout <<
		delay(1000);
	}
	return 0;
}

*/
