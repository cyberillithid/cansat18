#include "Magnet_HMC5883L.h"

Magnet_HMC5883L::Magnet_HMC5883L(I2CBus& b): I2CDev(b, 0x1E) {
	whoami_reg = 0xA; whoami_val = 'H'; //IRA
}
bool Magnet_HMC5883L::fetchData(Vec3D* v) {
	//DRXA - MSB, DRXB - LSB
	int16_t raw[3];
	mb_read(0x03, 6, (uint8_t*)&raw);
	v->x = (int16_t)(__builtin_bswap16(raw[0]))*resol;
	v->y = (int16_t)(__builtin_bswap16(raw[2]))*resol;
	v->z = (int16_t)(__builtin_bswap16(raw[1]))*resol;
	return true;
}
bool Magnet_HMC5883L::hasData() {
	return ((b_read(0x9) & 1) == 1);
}
void Magnet_HMC5883L::setup() {
	I2CDev::setup();
	//TODO: IRB, IRC?
	b_write(0x2, 0); //Continuous Measurement Mode
	resol = 0.92;
	rate = 15;
}

/*
 
 
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

*/
