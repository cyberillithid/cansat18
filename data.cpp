#include "data.h"
#include "mexception.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#define _SIZE 54
#define _TYPE 42
DataPkg::DataPkg() {}

DataPkg::DataPkg(char* buf, size_t len) {
	if (len < _SIZE) throw mexception("Size error");
	if (buf[0] != _TYPE) throw mexception("Wrong type");
	gps_mode = buf[1];
	double *pars = (double*)(buf+2);
	lat = pars[0];
	lon = pars[1];
	alt = pars[2];
	speed = pars[3];
	climb = pars[4];
	gpstime = pars[5];
	time = *(int*)(buf+50);
}

size_t DataPkg::toBytes(char* buf, size_t buflen) {
	if (buflen < _SIZE) return 0;
	buf[0] = _TYPE; //type
	buf[1] = gps_mode;
	double pars[] = {lat, lon, alt, speed, climb, gpstime}; //6*8 = 48
	memcpy(buf+2, pars, sizeof(pars)); 
	memcpy(buf+2+sizeof(pars), &time, sizeof(time));
	return _SIZE;
}

const char* GPS_MODES[] = {"NOT_SEEN", "NO_FIX", "2D", "3D"};

void DataPkg::print() {
	printf("GPS Mode: %s\n", GPS_MODES[gps_mode]);
	printf("Coordinates: %.4f, %.4f, %.4f\n", lat, lon, alt);
	printf("Speed: %.4f m/s hor, %.4f m/s vert\n", speed, climb);
	printf("Pi Time: %s", ctime((const time_t*)&time));
	time_t f = (int)(gpstime);
	printf("GPS Time: %.2f // %s", gpstime, ctime(&f));
	
}
