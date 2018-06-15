#pragma once
#include "sensors/Magnet_HMC5883L.h"
#include "sensors/Accel_ADXL345.h"
#include "sensors/Gyro_L3G4200D.h"
#include "sensors/Baro_BMP085.h"


class Satellite {
private:
	I2CBus i2cbus;
	Magnet_HMC5883L magneto;
	Accel_ADXL345 accel;
	Gyro_L3G4200D gyro;
	Baro_BMP085 baro;
	std::thread t1, *t2;
	I2CDev bat;
	
	std::atomic<bool> radio_stop;
	
	int radio_thread();
	DataPkg buildPacket();
public:
	Satellite();
	~Satellite();
	void loop();
};
