#include "data.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdexcept>

const int _DBL_CNT=6;
const int _3D_CNT=2;
const int _INT_CNT=5;
const int _dblOff = 2;
const int _intOff = _dblOff + sizeof(double)*_DBL_CNT;
const int _3dOff = _intOff + sizeof(uint32_t)*_INT_CNT;
const int _finOff = _3dOff + sizeof(double)*3*_3D_CNT;
const int _SIZE=_finOff;

#define _TYPE 42

DataPkg::DataPkg() {}

Vec3D VecFromArr(double* p){
	Vec3D v;
	v.x = p[0];
	v.y = p[1];
	v.z = p[2];
	return v;
}

void VecToArr(Vec3D& v, double* p){
	p[0] = v.x;
	p[1] = v.y;
	p[2] = v.z;
}

DataPkg::DataPkg(char* buf, size_t len) {
	if (len < _SIZE) throw std::runtime_error("Size error");
	if (buf[0] != _TYPE) throw std::runtime_error("Wrong type");
	gps_mode = buf[1];
	if (gps_mode > 3) throw std::runtime_error("Incorrect GPS mode");
	double *pars = (double*)(buf+_dblOff);
	lat = pars[0];
	lon = pars[1];
	alt = pars[2];
	speed = pars[3];
	climb = pars[4];
	gpstime = pars[5];
	
	uint32_t* parInts = (uint32_t*)(buf+_intOff);
	time = parInts[0];
	temp = parInts[1];
	pressure = parInts[2];
	bmpTemp = parInts[3];
	battery = parInts[4];
	
	double* parVecs = (double*)(buf+_3dOff);
	magn = VecFromArr(parVecs);
	accel = VecFromArr(parVecs+3);
	//memcpy(magn, buf+_3dOff, sizeof(magn));
}

size_t DataPkg::toBytes(char* buf, size_t buflen) {
	if (buflen < _SIZE) return 0;
	buf[0] = _TYPE; //type
	buf[1] = gps_mode;
	double pars[_DBL_CNT] = {lat, lon, alt, speed, climb, gpstime}; //6*8 = 48
	uint32_t parInts[_INT_CNT] = {time, temp, pressure, bmpTemp, battery};
	double vecs[_3D_CNT*3];
	VecToArr(magn, vecs); VecToArr(accel, vecs+3);
	memcpy(buf+_dblOff, pars, sizeof(pars)); 
	memcpy(buf+_intOff, parInts, sizeof(parInts));
	memcpy(buf+_3dOff, vecs, sizeof(vecs));
	return _SIZE;
}

const char* GPS_MODES[] = {"NOT_SEEN", "NO_FIX", "2D", "3D"};

void DataPkg::print() {
	printf("GPS Mode: %s\n", GPS_MODES[gps_mode]);
	printf("Coordinates: %.4f, %.4f, %.4f\n", lat, lon, alt);
	printf("Speed: %.4f m/s hor, %.4f m/s vert\n", speed, climb);
	float volt = (uint16_t)(battery)*.078125/1000;
	float cap = (battery>>16)/256.;
	printf("Battery capacity: %.1f%%, voltage: %.1f V\n", cap, volt);
	printf("Pi Time: %s", ctime((const time_t*)&time));
	time_t f = (int)(gpstime);
	printf("GPS Time: %.3lf + %s\n", (gpstime-f), ctime(&f));
	printf("Temperature: %.3lf @Pi, %.1lf @BMP\n", (temp*0.001), (bmpTemp*0.1));
	printf("Pressure: %.2lf hPa\n", (pressure*0.01));
	printf("Magnetic field: X: %.2fGa\tY: %.2fGa\tZ: %.2fGa\n", magn.x*0.001, magn.y*0.001, magn.z*0.001);
	printf("Acceleration: X: %.2fg\tY: %.2fg\tZ: %.2fg\n", accel.x*0.001, 
			accel.y*0.001, accel.z*0.001);
}
