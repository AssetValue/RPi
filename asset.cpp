#ifdef RPI
#include <stdio.h>
#include <iostream>
//#include <csignal>
#include <time.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "Adafruit_LIS3DH.h"

using namespace std;
#endif

#define POINTS 4500000 //max is 2^(8*(sizeof(unsigned int) = 4)) = 4294967296
#define PERIOD 200
#define MODE 0 //Mode 0: Flush data to file at every datapoint; Mode 1: Flush data to file after POINTS datapoints
#define LEDPIN  17 //Status LED Broadcom pin number (pulled low)

int fd;
unsigned int i, starttime;
char tm[12], filename[30];
//sig_atomic_t sigflag;

int setup() {
//    sigflag = 1;
    filename[0]='\0';
  //Setup WiringPi GPIO
    wiringPiSetupGpio();
    pinMode(LEDPIN, OUTPUT);
  //Setup WiringPi I2C
    fd = wiringPiI2CSetup(LIS3DH_DEFAULT_ADDRESS);
  //Check for LIS3DH device
    if (wiringPiI2CReadReg8(fd,LIS3DH_REG_WHOAMI) != 0x33) {
        printf("Could not start\n");
        return 1;
    }
  // DRDY on INT1
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL3, 0x10);
  // BDU disabled, +-4G Range
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL4, 0x10);
  // enable all axes, low power mode, 5kHz
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL1, 0x9C);
  //Display the g range for the accelerometer
    printf("Range = %dG\n",2 << (lis3dh_range_t)((wiringPiI2CReadReg8(fd,LIS3DH_REG_CTRL4) >> 4) & 0x03));
    return 0;
}

int loop() {
  //Turn on status LED
    digitalWrite(LEDPIN, HIGH);
  //Create and display filename
    sprintf(tm, "%lu", (unsigned long)time(NULL));
    strcpy(filename, "/data/assetData");
    strcat(filename, tm);
    strcat(filename, ".csv");
    printf("%s\n", filename);

  //Create a file to write to
    FILE *f = fopen(filename, "a");
    if (f == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

  //Set up file header
    fprintf(f, "time,z\n");

  //Collect and write data points
    starttime = micros();
    for (i=0; i<POINTS; i++) {
        starttime += PERIOD;
        while(micros()<starttime);
        fprintf(f, "%d,%d\n", micros(), (signed char)wiringPiI2CReadReg8(fd, LIS3DH_REG_OUT_Z_H));
    }
    fclose(f);
    return 0;
}

#ifdef RPI
//void sighandler(int s)
//{
//    sigflag = 1;
//}
//
int cleanup() {
    digitalWrite(LEDPIN, LOW);
    printf("Done!\n");
    return 0;
}

int main() {
    
//    signal(SIGINT, sighandler);
    if (setup()) return 1;
//    while(sigflag) {
    while(1) {
      if (loop()) return 1;
    }
    if (cleanup()) return 1;
    return 0;
}
#endif
