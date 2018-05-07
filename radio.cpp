/*
 * radio.cpp
 *
 */

#include <iostream>
#include <csignal>
#include "lora.h"
#include <string.h>
#include "data.h"

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
			DataPkg a(cur, lora->getLength());
			a.print();
			//printf("Received %s", cur);
			delete[] cur;
			delay(100);
		}
	}
}

int main(int argc, char *argv[])
{
	//parse options

	// we catch the CTRL-C key
	signal(SIGINT, INThandler);
	//setup();
	rcvloop();
	
	return 0;
}

