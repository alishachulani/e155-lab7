/*
File: Lab_6_JHB.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

#include "lib/STM32L432KC_GPIO.h"
#include "lib/STM32L432KC_RCC.h"
#include "lib/STM32L432KC_FLASH.h"
#include "lib/STM32L432KC_USART.h"
#include "lib/STM32L432KC_TIM.h"

#include "lib/STM32L432KC_SPI.h"
#include "lib/DS1722.h"


//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";
char* tempConfigStr = "<p>Temperature Sensor Configuration Control:</p><form action=\"8bit\"><input type=\"submit\" value=\"8-bit resolution\"></form>\
	<form action=\"9bit\"><input type=\"submit\" value=\"9-bit resolution\"></form>\
	<form action=\"10bit\"><input type=\"submit\" value=\"10-bit resolution\"></form>\
	<form action=\"11bit\"><input type=\"submit\" value=\"11-bit resolution\"></form>\
	<form action=\"12bit\"><input type=\"submit\" value=\"12-bit resolution\"></form>";
char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

// Update the LED output based on user input in webrequest
int updateLEDStatus(char request[])
{
	int led_status = digitalRead(LED_PIN);
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}


// Update the configuration of temperature sensor based on user input in webrequest
void updateTempConfiguration(char request[])
{
  if (inString(request, "8bit") == 1) {
    setTempConfiguration(8);
  } else if (inString(request, "9bit") == 1) {
    setTempConfiguration(9);
  } else if (inString(request, "10bit") == 1) {
    setTempConfiguration(10);
  } else if (inString(request, "11bit") == 1) {
    setTempConfiguration(11);
  } else if (inString(request, "12bit") == 1) {
    setTempConfiguration(12);
  }
}

int main(void) {

  GPIO_TypeDef * GPIO_PORT_PTR = gpioPinToBase(PB4);
  int pin_offset = gpioPinOffset(PB4);
  GPIO_PORT_PTR->PUPDR &= ~(0b11 << 8);
  configureFlash();
  configureClock();

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(LED_PIN, GPIO_OUTPUT);
  digitalWrite(LED_PIN, 0);

  initSPI(0b100, 0, 1);


  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */
  
    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }
  
    // Update string with current LED state
    int led_status = updateLEDStatus(request);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    // Update temperature sensor configuration
    updateTempConfiguration(request);

    // Read configuration register 
    char CFG = readConfiguration();

    // Create a string that displays the resolution based on the config register
    char CFG_current_status[20];
    if (CFG == 0xE0)
      sprintf(CFG_current_status, "8bit resolution");
    else if (CFG == 0xE2)
      sprintf(CFG_current_status, "9bit resolution");
    else if (CFG == 0xE4)
      sprintf(CFG_current_status, "10bit resolution");
    else if (CFG == 0xE6)
      sprintf(CFG_current_status, "11bit resolution");
    else if (CFG == 0xE8)
      sprintf(CFG_current_status, "12bit resolution");

    // Read Temperature LSB
    char LSB = readTempLSB();

    // Read Temperature MSB
    char MSB = readTempMSB();

    // Get the binary string representations
    char LSBStr[20];
    sprintf(LSBStr, "%c%c%c%c%c%c%c%c",
        (LSB & 0x80) ? '1' : '0',
        (LSB & 0x40) ? '1' : '0',
        (LSB & 0x20) ? '1' : '0',
        (LSB & 0x10) ? '1' : '0',
        (LSB & 0x08) ? '1' : '0',
        (LSB & 0x04) ? '1' : '0',
        (LSB & 0x02) ? '1' : '0',
        (LSB & 0x01) ? '1' : '0'
    );
    char MSBStr[20];
    sprintf(MSBStr, "%c%c%c%c%c%c%c%c",
        (MSB & 0x80) ? '1' : '0',
        (MSB & 0x40) ? '1' : '0',
        (MSB & 0x20) ? '1' : '0',
        (MSB & 0x10) ? '1' : '0',
        (MSB & 0x08) ? '1' : '0',
        (MSB & 0x04) ? '1' : '0',
        (MSB & 0x02) ? '1' : '0',
        (MSB & 0x01) ? '1' : '0'
    );
    char CFGStr[20];
    sprintf(CFGStr, "%c%c%c%c%c%c%c%c",
        (CFG & 0x80) ? '1' : '0',
        (CFG & 0x40) ? '1' : '0',
        (CFG & 0x20) ? '1' : '0',
        (CFG & 0x10) ? '1' : '0',
        (CFG & 0x08) ? '1' : '0',
        (CFG & 0x04) ? '1' : '0',
        (CFG & 0x02) ? '1' : '0',
        (CFG & 0x01) ? '1' : '0'
    );

    

    // Create a string with the decimal (<1) representation of LSB
    char LSBStrDecimal[20];
    double decimalValue = 0.0;
    decimalValue += (LSB & 0x80) ? 0.5 : 0.0;
    decimalValue += (LSB & 0x40) ? 0.25 : 0.0;
    decimalValue += (LSB & 0x20) ? 0.125 : 0.0;
    decimalValue += (LSB & 0x10) ? 0.0625 : 0.0;
    sprintf(LSBStrDecimal, "%f", decimalValue);
    
    char tempStr[20];
    char MSBStrDecimal[20];
    int MSB_helper;
    if (MSB & (1 << 7)) {
      sprintf(MSBStrDecimal, "-%d", (MSB - 0b10000000));
      MSB_helper = -128 + (float) (MSB & 0x7F);      // -128+(msb); we only want to add the 7 digits of MSB, not all 8; fix this line here
      sprintf(tempStr, "%f", MSB_helper + decimalValue);
    }
    else {
      sprintf(MSBStrDecimal, "%d", MSB);
      sprintf(tempStr, "%f", MSB + decimalValue);
    }


    // Transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED
    sendString(USART, tempConfigStr); // button for controlling the resolution

    sendString(USART, "<h2>LED Status</h2>");

    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");
    char degree = 176;
    char helperStr[20];
    sprintf(helperStr, " %c C", degree);
    sendString(USART, "<h2>DS1722 Temperature Sensor</h2>");
    sendString(USART, "<h2>");
    sendString(USART, tempStr);
    sendString(USART, helperStr);
    sendString(USART, "</h2>");

    sendString(USART, "<p>");
    sendString(USART, "Config read: ");
    sendString(USART, CFGStr);
    sendString(USART, " ");
    sendString(USART, CFG_current_status);
    sendString(USART, "</p>");

    sendString(USART, "<p>");
    sendString(USART, "LSB read: ");
    sendString(USART, LSBStr);
    sendString(USART, "</p>");

    sendString(USART, "<p>");
    sendString(USART, "MSB read: ");
    sendString(USART, MSBStr);
    sendString(USART, "</p>");
  
    sendString(USART, webpageEnd);
  }
}