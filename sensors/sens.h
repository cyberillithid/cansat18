#pragma once
struct Vec3D{
	double x;
	double y;
	double z;
};

class Sensor3D {
public:
	virtual bool hasData() = 0;
	virtual bool fetchData(Vec3D*) = 0;
	virtual void setup() = 0;
};
