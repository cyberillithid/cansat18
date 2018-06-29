CC=g++
CFLAGS=-Wall -I. -std=c++11

ODIR=obj

_OBJ_RAD = lora.o data.o 
OBJ_RAD = $(patsubst %,$(ODIR)/radio/%,$(_OBJ_RAD))

_OBJ_SENS = devices.o Magnet_HMC5883L.o Accel_ADXL345.o Gyro_L3G4200D.o Baro_BMP085.o DHT22.o
OBJ_SENS = $(patsubst %,$(ODIR)/sensors/%,$(_OBJ_SENS))

LIBS = -lgps -lpthread -lwiringPi

all: receiver80 cansat80

install: cansat80
	config/install.sh

install_low: cansat80
	config/install.sh low
	
receiver80: $(OBJ_RAD) $(ODIR)/rcvr.o
	$(CC) -lpthread $^ -o $@

cansat80: $(OBJ_RAD) $(OBJ_SENS) $(ODIR)/sndr.o $(ODIR)/satellite.o
	$(CC) $(LIBS) $(CFLAGS) -o $@ $^

$(ODIR)/radio/%.o: radio/%.cpp | $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/sensors/%.o: sensors/%.cpp | $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/rcvr.o: | $(ODIR)
	$(CC) $(CFLAGS) -c main.cpp -o $(ODIR)/rcvr.o

$(ODIR)/sndr.o: | $(ODIR)
	$(CC) $(CFLAGS) -DSATELLITE -c main.cpp -o $(ODIR)/sndr.o

$(ODIR)/satellite.o: | $(ODIR)
	$(CC) $(CFLAGS) -c satellite.cpp -o $(ODIR)/satellite.o

$(ODIR):
	mkdir $(ODIR)
	mkdir $(ODIR)/radio
	mkdir $(ODIR)/sensors

clean:
	rm -rf $(ODIR) *80

