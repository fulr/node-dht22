/*  Node module for DHT temperature/humidity sensors
 *
 * Adaption for Node.js by Friedl Ulrich Jan. 2014
 *
 *  Modified by Qingping Hou from DHT reader example, original header:
 *
 *  How to access GPIO registers from C-code on the Raspberry-Pi
 *  Example program
 *  15-January-2012
 *  Dom and Gert
 */


/* for usleep */
#define _BSD_SOURCE

#include <node.h>
#include <v8.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <bcm2835.h>
#include <unistd.h>

#define MAXTIMINGS 100

//#define DEBUG

#define DHT11 11
#define DHT22 22
#define AM2302 22

#define BREAKTIME 20

using namespace v8;

int readDHT(int type, int pin, float *temp_p, float *hum_p)
{
	int counter = 0;
	int laststate = HIGH;
	int i = 0;
	int j = 0;
	int checksum = 0;
#ifdef DEBUG
	int bitidx = 0;
	int bits[250];
#endif
	int data[100];

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	// Set GPIO pin to output
	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_write(pin, HIGH);
        bcm2835_delay(50);
	bcm2835_gpio_write(pin, LOW);
        bcm2835_delay(18);
	bcm2835_gpio_write(pin, HIGH);
        bcm2835_delayMicroseconds(1);

	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

	// wait for pin to drop?
	while (bcm2835_gpio_lev(pin) == HIGH && i<100000) {
          bcm2835_delayMicroseconds(1);
          i++;
	}

        if(i==100000)
          return -1;

	// read data!
	for (i = 0; i < MAXTIMINGS; i++) {
		counter = 0;
		while ( bcm2835_gpio_lev(pin) == laststate) {
			counter++;
                        bcm2835_delayMicroseconds(1);
			if (counter == 1000)
				break;
		}
		laststate = bcm2835_gpio_lev(pin);
		if (counter == 1000) break;
#ifdef DEBUG
		bits[bitidx++] = counter;
#endif
		if ((i>3) && (i%2 == 0)) {
			// shove each bit into the storage bytes
			data[j/8] <<= 1;
			if (counter > BREAKTIME)
				data[j/8] |= 1;
			j++;
		}
	}

#ifdef DEBUG
	for (int i=3; i<bitidx; i+=2) {
		printf("bit %d: %d\n", i-3, bits[i]);
		printf("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > BREAKTIME);
	}
	printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);
#endif

	if (j >= 39) {
		checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
		if (data[4] == checksum) {
			/* yay! checksum is valid */
			if (type == DHT11) {
				/*printf("Temp = %d *C, Hum = %d \%\n", data[2], data[0]);*/
				*temp_p = (float)data[2];
				*hum_p = (float)data[0];
			} else if (type == DHT22) {
				*hum_p = data[0] * 256 + data[1];
				*hum_p /= 10;

				*temp_p = (data[2] & 0x7F)* 256 + data[3];
				*temp_p /= 10.0;
				if (data[2] & 0x80)
					*temp_p *= -1;
				/*printf("Temp =  %.1f *C, Hum = %.1f \%\n", f, h);*/
			}
			return 0;
		}
		return -2;
	}

	return -1;
}


Handle<Value> ReadDHT22(const Arguments& args) {
  HandleScope scope;
  int pin=4;

  if(args.Length()>0 && args[0]->IsNumber())
    pin=args[0]->Int32Value();

  Local<Object> obj=Object::New();

  float humidity;
  float temperature;

  int state=readDHT(22,pin,&temperature,&humidity);

  obj->Set(String::New("state"),Number::New(state));
  obj->Set(String::New("humidity"),Number::New(humidity));
  obj->Set(String::New("temperature"),Number::New(temperature));

  return scope.Close(obj);
}

void init(Handle<Object> exports) {
  if(bcm2835_init()==0)
    return;

  exports->Set(String::NewSymbol("read"),
      FunctionTemplate::New(ReadDHT22)->GetFunction());
}

NODE_MODULE(dht22, init)

