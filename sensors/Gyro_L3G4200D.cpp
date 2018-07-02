#include "Gyro_L3G4200D.h"

Gyro_L3G4200D::Gyro_L3G4200D(I2CBus& bus) : 
	I2CDev(bus, 0x69)
{
	whoami_reg = 0xF; whoami_val = 0xD3;
}
uint8_t Gyro_L3G4200D::getTemp() {
	return b_read(0x26);
}
bool Gyro_L3G4200D::hasData() {
	return ((b_read(0x27) & 0x8) == 8);
}

void Gyro_L3G4200D::getRaw(void* d){
	memcpy(d, raw, 6);
}

bool Gyro_L3G4200D::fetchData(Vec3D* v) {
	mb_read(0x28, 6, (uint8_t*)raw);
	v->x = raw[0]*rate;
	v->y = raw[1]*rate;
	v->z = raw[2]*rate;
	return true;
}

void Gyro_L3G4200D::setup() {
	I2CDev::setup();
	b_write(0x20, 0xF); //CTRL_REG_1 -- ODR 100 hz, CutOff 12.5, all on
	rate = 8.75l; //default 250 dps
}

/*
 
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

*/
