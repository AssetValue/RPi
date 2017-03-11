#ifdef RPI
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <wiringPiI2C.h>
#include "Adafruit_LIS3DH.h"

using namespace std;
#endif

#define POINTS 600000 //max is 2^(8*(sizeof(unsigned int) = 4)) = 4294967296
#define PERIOD 100
#define MODE 0 //Mode 0: Flush data to file at every datapoint; Mode 1: Flush data to file after POINTS datapoints

void setup() {
  //Setup WiringPi I2C
    fd = wiringPiI2CSetup(LIS3DH_DEFAULT_ADDRESS);
  //Check for LIS3DH device
    if (wiringPiI2CReadReg8(fd,LIS3DH_REG_WHOAMI) != 0x33) {
      printf("Could not start\n");
      return 1;
    }
  // enable all axes, normal mode
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL1, 0x07);
  // 400Hz rate
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL1,(wiringPiI2CReadReg8(fd,LIS3DH_REG_CTRL1) & ~(0xF0)) | (LIS3DH_DATARATE_400_HZ << 4));
  // High res & BDU enabled
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL4, 0x88);
  // DRDY on INT1
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL3, 0x10);
  // Turn on orientation config
    //wiringPiI2CWriteReg8(fd,LIS3DH_REG_PL_CFG, 0x40);
  // enable adcs
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_TEMPCFG, 0x80);
  //Set and display the g range for the accelerometer
    wiringPiI2CWriteReg8(fd,LIS3DH_REG_CTRL4,(wiringPiI2CReadReg8(fd,LIS3DH_REG_CTRL4) & ~(0x30)) | (LIS3DH_RANGE_2_G << 4));
    printf("Range = %dG\n",2 << (lis3dh_range_t)((wiringPiI2CReadReg8(fd,LIS3DH_REG_CTRL4) >> 4) & 0x03));
}

void loop() {
  //Create and display filename
    sprintf(tm, "%lu", (unsigned long)time(NULL));
    strcpy(filename, "data/assetData");
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
    fprintf(f, "time,time,z\n");

  //Collect and write data points
    startclock = clock();
    for (i=0; i<POINTS; i++) {
      startclock += PERIOD;
      while(clock()<startclock);
      z = wiringPiI2CReadReg8(fd, LIS3DH_REG_OUT_Z_L) | ((unsigned short int)wiringPiI2CReadReg8(fd, LIS3DH_REG_OUT_Z_H)) << 8;
      fprintf(f, "%d,%d\n", i*PERIOD,  z);
    }
    fclose(f);
}

#ifdef RPI
int main() {
    short int z;
    int fd;
    unsigned int i;
    char tm[11], filename[30];
    clock_t startclock;
    
    setup();
    while(1) loop();
    return 0;
}
#endif
