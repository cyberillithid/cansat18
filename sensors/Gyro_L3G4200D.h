#include "sens.h"
#include "devices.h"

class Gyro_L3G4200D : public Sensor3D, protected I2CDev {
private:
	// limits? consts?
	double rate;
	int16_t raw[3];
public:
	 Gyro_L3G4200D(I2CBus& bus);
	 uint8_t getTemp();
	 bool hasData();
	 void getRaw(void* d);
	 bool fetchData(Vec3D* v);
	 void setup();
};
