#include <stdlib.h>
#include <stdint.h>
struct DataPkg {
	// gps data block
	// 252 byte
	uint8_t gps_mode; //1 byte
	double lat, lon, alt, speed, climb; //40 byte
	uint32_t time; //4 byte
	double gpstime;
	// T [block], `C
	// p
	// phi, r
	// custom text
	//static DataPkg fromBytes(char* pkg, size_t len);
	DataPkg();
	DataPkg(char* pkg, size_t len);
	size_t toBytes(char* buf, size_t buflen);
	void print();
};

