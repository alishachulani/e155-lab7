// DS1722.c
// Alisha Chulani
// achulani@g.hmc.edu
// October 2024
// code for interfacing with DS1722 temperature sensor 

#include "STM32L432KC_SPI.h"
#include "STM32L432KC_GPIO.h"

// function to set configurations
void setTempConfiguration(int resolution) {
  // Config value bits [3:1] = [R2:R0]. Bit 0 is SD bit which must be 0
  char configRegValue = 0x08;
  if (resolution == 8) {
    configRegValue = 0x00; 
  } else if (resolution == 9) {
    configRegValue = 0x02;
  } else if (resolution == 10) {
    configRegValue = 0x04;
  } else if (resolution == 11) {
    configRegValue = 0x06;
  }  else if (resolution == 12) {
    configRegValue = 0x08;
  }
  digitalWrite(SPI_CE, 1);
  spiSendReceive(0x80); 
  spiSendReceive(configRegValue);
  digitalWrite(SPI_CE, 0);
}


// function to read configurations 
char readConfiguration(void) {
  digitalWrite(SPI_CE, 1);
  spiSendReceive(0x00);
  char CFG = spiSendReceive(0x00);
  digitalWrite(SPI_CE, 0);
  return CFG;
}

// function to read the least significant temperature bits
char readTempLSB(void) {
  digitalWrite(SPI_CE, 1);
  spiSendReceive(0x01);
  char LSB = spiSendReceive(0x00);
  digitalWrite(SPI_CE, 0);
  return LSB;
}

// function to read the most significant temperature bits
char readTempMSB(void) {
  digitalWrite(SPI_CE, 1);
  spiSendReceive(0x02);
  char MSB = spiSendReceive(0x00);
  digitalWrite(SPI_CE, 0);
  return MSB;
}
