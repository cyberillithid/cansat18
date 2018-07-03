#include "sens.h"
#include "devices.h"

//notes:
// Freefall, tap thresholds & events [mb use, mb test?]
class Accel_ADXL345 : public Sensor3D, protected I2CDev{
private:
	//limits: 2..16 g
	//noise: pm40; static pm 250 
	//TODO: note on diff 'tw XY and Z
	double resol; //256 LSB per g
	// I2C -> max for fast 800 Hz, slow 200 Hz
	double rate; // .1 Hz -> 3200 Hz; <6.3 & >1600 - diff; 
	int16_t raw[3];
public:
	Accel_ADXL345(I2CBus& bus);

	void setOffsets(Vec3D& d);
	
	bool hasData();
	void fetchRaw(void *d);
	void getData(Vec3D* v);
	bool fetchData(Vec3D* v);
	void setup();
};
