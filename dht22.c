/*
 *      dht22.c:
 *	Simple test program to test the wiringPi functions
 *	Based on the existing dht11.c
 *	Amended by technion@lolware.net
 */

#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXTIMINGS 85

/**
 * Reading DHT22
 *
 * Returns 0: success
 *        -1: Data invalid
 *        -2: Wiring PI Setup failed
 *        -3: Not Root
 *        -4: Reading failed
 *
 */
int read_dht22_dat(int pin,float* humidity, float* temperature)
{
  uint8_t laststate = HIGH;
  uint8_t state = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;
  int dht22_dat[5] = {0,0,0,0,0};

  if (wiringPiSetup () == -1)
    return -2;

  if (setuid(getuid()) < 0)
    return -3;

  dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

  // pull pin down for 18 milliseconds
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  delay(10);
  digitalWrite(pin, LOW);
  delay(18);
  // then pull it up for 40 microseconds
  digitalWrite(pin, HIGH);
  delayMicroseconds(40); 
  // prepare to read the pin
  pinMode(pin, INPUT);

  // detect change and read data
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ((state=digitalRead(pin)) == laststate) {
      if(state>255||state<0)
        return -4;
      counter++;
      delayMicroseconds(1);
      if (counter == 255) {
        break;
      }
    }
    laststate = digitalRead(pin);
    if(laststate>255||laststate<0)
      return -4;

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
    float t, h;
    h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
    h /= 10;
    t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
    t /= 10.0;
    if ((dht22_dat[2] & 0x80) != 0)  t *= -1;

    return 0;
  }
  else
  {
    return -1;
  }
}
