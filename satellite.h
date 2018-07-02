#pragma once
#include "sensors/Magnet_HMC5883L.h"
#include "sensors/Accel_ADXL345.h"
#include "sensors/Gyro_L3G4200D.h"
#include "sensors/Baro_BMP085.h"


class Satellite {
private:
	bool isLoHF;
	I2CBus i2cbus;
	Magnet_HMC5883L magneto;
	Accel_ADXL345 accel;
	Gyro_L3G4200D gyro;
	Baro_BMP085 baro;
	std::thread thrGPS, *thrRadio, thrDHT, *thrSens;
	I2CDev bat;
	
	std::atomic<bool> radio_stop;
	
	std::atomic<Vec3D> acc;
	
	int radio_thread();　
	int sensors_thread();
	DataPkg buildPacket();
public:
	Satellite(bool isLo);
	~Satellite();
	void loop();
};
