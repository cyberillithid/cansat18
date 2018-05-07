#include "data.h"
#include "mexception.h"
#include <stdio.h>
#include <time.h>
#define _SIZE 54

DataPkg::DataPkg() {}

DataPkg::DataPkg(char* buf, size_t len) {
	if (len < _SIZE) throw new mexception("Size error");
	if (buf[0] != 42) throw new mexception("Wrong type");
	gps_mode = buf[1];
	lat = *(double*)(buf+2);
	lon = *(double*)(buf+10);
	alt = *(double*)(buf+18);
	speed = *(double*)(buf+26);
	climb = *(double*)(buf+34);
	time = *(int*)(buf+42);
	gpstime = *(double*)(buf+46);
}

size_t DataPkg::toBytes(char* buf, size_t buflen) {
	if (buflen < _SIZE) return 0;
	buf[0] = 42; //type
	buf[1] = gps_mode;
	*(double*)(buf+2) = lat;
	*(double*)(buf+10) = lon;
	*(double*)(buf+18) = alt;
	*(double*)(buf+26) = speed;
	*(double*)(buf+34) = climb;
	*(int*)(buf+42) = time;
	*(double*)(buf+46) = gpstime;
	return _SIZE;
}

const char* GPS_MODES[] = {"NOT_SEEN", "NO_FIX", "2D", "3D"};

void DataPkg::print() {
	printf("GPS Mode: %s\n", GPS_MODES[gps_mode]);
	printf("Coordinates: %.4f, %.4f, %.4f\n", lat, lon, alt);
	printf("Speed: %.4f m/s hor, %.4f m/s vert\n", speed, climb);
	printf("Pi Time: %s", ctime((const time_t*)&time));
	time_t f = (int)(gpstime);
	printf("GPS Time: %s", ctime(&f));
	
}
