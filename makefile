CC=g++
CFLAGS=-Wall -I. -std=c++11

all: radio gy

radio: mexception.o radio.o lora.o data.o
	$(CC) mexception.o radio.o lora.o data.o -o radio

gy: mexception.o gy.o lora.o data.o
	$(CC) -lgps -lpthread mexception.o gy.o lora.o data.o -o gy

radio.o:
	$(CC) $(CFLAGS) -c radio.cpp

gy.o:
	$(CC) $(CFLAGS) -c gy.cpp

mexception.o:
	$(CC) $(CFLAGS) -c mexception.cpp

lora.o:
	$(CC) $(CFLAGS) -c lora.cpp

data.o:
	$(CC) $(CFLAGS) -c data.cpp

clean:
	rm -rf *.o gy radio

