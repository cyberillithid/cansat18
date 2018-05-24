#include "sens.h"
#include "devices.h"

class Magnet_HMC5883L: public Sensor3D, protected I2CDev {
private:
	//limits: 1..8 gauss
	double resol; //230 .. 1730 LSb / gauss; default 0.92 mG / LSb
	double rate; //default 15 Hz
	//res: 1-2 deg accuracy
public:
	Magnet_HMC5883L(I2CBus& b);
	bool hasData();
	bool fetchData(Vec3D* v);
	void setup();
};

// int magnet_test();
