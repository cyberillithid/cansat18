CC=g++
CFLAGS=-Wall -I. -std=c++11

all: radio gy

radio: mexception.o radio.o
	$(CC) mexception.o radio.o -o radio

gy: mexception.o gy.o
	$(CC) mexception.o gy.o -o gy

radio.o:
	$(CC) $(CFLAGS) -c radio.cpp

gy.o:
	$(CC) $(CFLAGS) -c gy.cpp

mexception.o:
	$(CC) $(CFLAGS) -c mexception.cpp

clean:
	rm -rf *.o gy radio

