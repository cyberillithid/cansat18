#include <wiringPi.h>
#include "DHT22.h"
#include <exception>
#include <stdexcept> 

#define MAXTIMINGS 85

DHT22::DHT22(int wPin){
	pin = wPin;
	retries = 0;
}

static uint8_t sizecvt(const int read)
{
  /* digitalRead() and friends from wiringpi are defined as returning a value
  < 256. However, they are returned as int() types. This is a safety function */

  if (read > 255 || read < 0)
  
    throw std::runtime_error("Invalid data from wiringPi library\n");
  return (uint8_t)read;
}

bool DHT22::fetch() {
	uint8_t dht22_dat[5] = {0,0,0,0,0};
	uint8_t laststate = HIGH;
	uint8_t j = 0, i;
	uint8_t counter = 0;  
  pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
	delay(10);
	digitalWrite(pin, LOW);
	delay(18);
	digitalWrite(pin, HIGH);
	delayMicroseconds(40); 
	pinMode(pin, INPUT);
	
	// detect change and read data
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (sizecvt(digitalRead(pin)) == laststate) {
      counter++;
      delayMicroseconds(2);
      if (counter == 255) {
        break;
      }
    }
    laststate = sizecvt(digitalRead(pin));

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      dht22_dat[j/8] <<= 1;
      if (counter > 16)
        dht22_dat[j/8] |= 1;
      j++;
    }
  }

  // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
  // print it out if data is good
  if ((j >= 40) && 
      (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
		  val = *(uint32_t*)(dht22_dat);
		  
		/*  
        float t, h;
        h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
        h /= 10;
        t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
        t /= 10.0;
        if ((dht22_dat[2] & 0x80) != 0)  t *= -1;


    printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );*/
    return true;
  }
  retries++;
	return false;
}

uint32_t DHT22::get() {
	return val;
}
