#include "data.h"
#include "mexception.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

const int _DBL_CNT=6;
const int _3D_CNT=1;
const int _INT_CNT=4;
const int _dblOff = 2;
const int _intOff = _dblOff + sizeof(double)*_DBL_CNT;
const int _3dOff = _intOff + sizeof(uint32_t)*_INT_CNT;
const int _finOff = _3dOff + sizeof(double)*3*_3D_CNT;
const int _SIZE=_finOff;

#define _TYPE 42

DataPkg::DataPkg() {}

DataPkg::DataPkg(char* buf, size_t len) {
	if (len < _SIZE) throw mexception("Size error");
	if (buf[0] != _TYPE) throw mexception("Wrong type");
	gps_mode = buf[1];
	if (gps_mode > 3) throw mexception("Incorrect GPS mode");
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
	
	memcpy(magn, buf+_3dOff, sizeof(magn));
}

size_t DataPkg::toBytes(char* buf, size_t buflen) {
	if (buflen < _SIZE) return 0;
	buf[0] = _TYPE; //type
	buf[1] = gps_mode;
	double pars[_DBL_CNT] = {lat, lon, alt, speed, climb, gpstime}; //6*8 = 48
	uint32_t parInts[_INT_CNT] = {time, temp, pressure, bmpTemp};
	memcpy(buf+_dblOff, pars, sizeof(pars)); 
	memcpy(buf+_intOff, parInts, sizeof(parInts));
	memcpy(buf+_3dOff, magn, sizeof(magn));
	return _SIZE;
}

const char* GPS_MODES[] = {"NOT_SEEN", "NO_FIX", "2D", "3D"};

void DataPkg::print() {
	printf("GPS Mode: %s\n", GPS_MODES[gps_mode]);
	printf("Coordinates: %.4f, %.4f, %.4f\n", lat, lon, alt);
	printf("Speed: %.4f m/s hor, %.4f m/s vert\n", speed, climb);
	printf("Pi Time: %s\n", ctime((const time_t*)&time));
	time_t f = (int)(gpstime);
	printf("GPS Time: %s + %.3lf\n", ctime(&f), (gpstime-f));
	printf("Temperature: %.3lf @Pi, %.1lf @BMP\n", (temp*0.001), (bmpTemp*0.1));
	printf("Pressure: %.2lf hPa\n", (pressure*0.01));
	printf("Magnetic field: %lfGa, %lfGa, %lfGa\n", magn[0], magn[1], magn[2]);
}
