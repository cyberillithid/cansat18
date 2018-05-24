#include "Accel_ADXL345.h"
#include <math.h>

Accel_ADXL345::Accel_ADXL345(I2CBus& bus): I2CDev(bus, 0x53) {
	whoami_reg = 0; whoami_val = 0xE5;
}
bool Accel_ADXL345::fetchData(Vec3D* v) {
	int16_t raw[3];
	mb_read(0x32, 6, (uint8_t*)(&raw));
	v->x = raw[0]*resol;
	v->y = raw[1]*resol;
	v->z = raw[2]*resol;
	return true;
}
bool Accel_ADXL345::hasData() {
	return ((b_read(0x30) & 0x80) == 0x80);
}
/// avg data in mg [milli-*g*]
void Accel_ADXL345::setOffsets(Vec3D& d) {
	double scale = 15.6; // mg/LSB
	int8_t ds[3];
	ds[0] = (int8_t)round(-d.x/scale);
	ds[1] = (int8_t)round(-d.y/scale);
	ds[2] = (int8_t)round(-d.z/scale);
	// write to OFSX, OFSY, OFSZ [0x1E, 0x1F, 0x20]
	mb_write(0x1E, 3, (uint8_t*)ds);
}
void Accel_ADXL345::setup() {
	//selftest?
	I2CDev::setup();
	b_write(0x2D, 0); //POWER_CTL -> standby
	b_write(0x2C, 0x0B); //BW_RATE reg; [t7, t8 - hz]
	rate = 200; //mb int? but 6.25 Hz and lower..
	b_write(0x31, 9); //DATA_FORMAT -> [self-test 0x80? 8 - full-res]
					// 0x1 -- +-4g
	b_write(0x38, 0);
	
	b_write(0x2D, 8); //POWER_CTL -> measure
	resol = 1000./256.; //mid.				
}

/*


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

*/
