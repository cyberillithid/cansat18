#include "devices.h"

class Baro_BMP085: protected I2CDev {
private:
	//limits: 300-1100 hPa (+9..-0.5km)
	//int16_t coefs[11];
	int16_t AC1[3];
	uint16_t AC4[3];
	int16_t B1, B2, MB, MC, MD;
	uint8_t oss = 0;
	long temp, pres;
public:
	Baro_BMP085(I2CBus& bus);
	bool fetchData();
	long getTemp();
	long getPress();
};

