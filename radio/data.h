#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "../sensors/sens.h" 
//TODO: better naming?

struct DataPkg {
	// gps data block
	// 252 byte
	uint8_t gps_mode; //1 byte
	double lat, lon, alt, speed, climb; //40 byte
	double gpstime;
	Vec3D magn, accel;
	// Pi data block
	uint32_t time; //4 byte
	uint32_t temp; //4 byte
	uint32_t battery; //volt. incl.
	// fetch LoRa temp?
	// IMU/MPU data block
	uint32_t pressure;
	uint32_t bmpTemp;
	//magical DHT/DS
	uint32_t hum_tempDHT;
	uint32_t tempDS;
	// phi, r
	// custom text
	//static DataPkg fromBytes(char* pkg, size_t len);
	DataPkg();
	DataPkg(char* pkg, size_t len);
	size_t toBytes(char* buf, size_t buflen);
	void print();
};

struct Timed3D{
    struct timespec ts;
    uint8_t data;
};