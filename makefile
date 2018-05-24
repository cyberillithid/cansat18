CC=g++
CFLAGS=-Wall -I. -std=c++11

all: receiver80 cansat80

receiver80: radio_grp rcvr.o
	$(CC) lora.o data.o rcvr.o -o receiver80

cansat80: radio_grp sens_grp sndr.o
	$(CC) -lgps -lpthread lora.o data.o devices.o \
magnet.o accel.o gyro.o baro.o satellite.o sndr.o -o cansat80

sens_grp:
	$(CC) $(CFLAGS) -c sensors/devices.cpp
	$(CC) $(CFLAGS) -c sensors/Magnet_HMC5883L.cpp -o magnet.o
	$(CC) $(CFLAGS) -c sensors/Accel_ADXL345.cpp -o accel.o
	$(CC) $(CFLAGS) -c sensors/Gyro_L3G4200D.cpp -o gyro.o
	$(CC) $(CFLAGS) -c sensors/Baro_BMP085.cpp -o baro.o

rcvr.o:
	$(CC) $(CFLAGS) -c main.cpp -o rcvr.o

sndr.o:
	$(CC) $(CFLAGS) -c satellite.cpp
	$(CC) $(CFLAGS) -DSATELLITE -c main.cpp -o sndr.o

radio_grp:
	$(CC) $(CFLAGS) -c radio/lora.cpp
	$(CC) $(CFLAGS) -c radio/data.cpp

clean:
	rm -rf *.o *80

