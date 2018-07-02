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
#include <atomic>
#include <thread>
std::atomic<bool> dieRcvr;
std::thread *loThr;
#else
#define IS_RCVR false
#include "satellite.h"
#endif	

void INThandler(int sig){
  printf("Bye.\n");
  time_t t = time(NULL);
#ifndef SATELLITE
  dieRcvr = true;
  loThr->join();
#endif
  fprintf(stderr, "Finished work at %s", ctime(&t));
  exit(0);
}

#ifndef SATELLITE
void getPkg(LoRa* lora, const char* tag) {
	char fs[256] = "/home/pi/log.";
	strcat(fs, tag);
	FILE* logfile = fopen(fs, "wb");
	while (!dieRcvr) {
		char* cur = lora->receiveAll(5000);
		if (cur != nullptr) {
			try {
				printf("%s: ", tag);
				DataPkg a(cur, lora->getLength());
				a.print();
				fwrite(&a, sizeof(DataPkg), 1, logfile);
				fflush(stdout);
			} catch (std::exception& e) {
				std::cerr << tag << ": " << e.what() << "\n";
			}
			delete[] cur;
		}
	}
	fclose(logfile);
	printf("%s finished\n",tag);
	delete lora;
}

void rcvloop(bool hf, bool lf) {
	LoRa* loraHi, *loraLo;
	if (hf) {
		loraHi = new LoRa("/dev/spidev0.0", RCVR_ADDR, CH_866_80);
		printf("LoRa/Hi Version: %d\n", loraHi->getVersion());
		loraHi->setLogLevel(3);
	}
	if (lf) {
		loraLo = new LoRa("/dev/spidev1.0", RCVR_ADDR, CH_433_80);
		printf("LoRa/Lo Version: %d\n", loraLo->getVersion());
		loraLo->setLogLevel(3);
	}
	dieRcvr = false;
	if (lf) loThr = new std::thread(getPkg, loraLo, "433");
	if (hf) getPkg(loraHi, "866");
}
#endif

int main(int argc, char *argv[])
{
	//parse options
	// -- we don't have any
	//printf("%d", argc);
	bool hi = true, lo = IS_RCVR;
	if (argc > 1){
		switch (argv[1][0]){
			case 'l': //low
				hi = false;
			case 'b': //both
				lo = true;
				break;	
			case 'h': //high
				lo = false;
			default: //per
				break;
		}
	}
	// we catch the CTRL-C key
	signal(SIGINT, INThandler);
	signal(SIGTERM, INThandler);
#ifdef SATELLITE
	Satellite sat(lo);
	sat.loop();
#else
	rcvloop(hi, lo);
#endif
	
	return 0;
}

