/*
 * main.cpp
 *
 */
 
#include <iostream>
#include <csignal>
#include "radio/lora.h"
#include <string.h>
#include "radio/data.h"


#ifndef SATELLITE
#define IS_RCVR true
#define M_LOOP rcvloop
#else
#define IS_RCVR false
#define M_LOOP sat.loop
#include "satellite.h"
#endif

void INThandler(int sig){
  printf("Bye.\n");
  exit(0);
}

void rcvloop() {
	LoRa *lora = new LoRa("/dev/spidev0.0", RCVR_ADDR);
	printf("LoRa Version: %d\n", lora->getVersion());
	lora->setLogLevel(3);
	for (;;) {
		char* cur = lora->receiveAll(5000);
		if (cur != nullptr) {
			try {
				DataPkg a(cur, lora->getLength());
				a.print();
			} catch (std::exception& e) {
				std::cerr << e.what() << "\n";
			}
			//printf("Received %s", cur);
			delete[] cur;
			//delay(100);
		}
	}
}

int main(int argc, char *argv[])
{
	//parse options
	// -- we don't have any
	
	// we catch the CTRL-C key
	signal(SIGINT, INThandler);
#ifdef SATELLITE
	Satellite sat;
#endif
	M_LOOP();
	
	return 0;
}

