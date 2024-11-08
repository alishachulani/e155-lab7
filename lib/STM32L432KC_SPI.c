// STM32L432KC_SPI.c
// TODO: <YOUR NAME>
// TODO: <YOUR EMAIL>
// TODO: <DATE>
// TODO: <SHORT DESCRIPTION OF WHAT THIS FILE DOES>

#include "STM32L432KC_SPI.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_RCC.h"

// Function to initialize SPI registers
void initSPI(int br, int cpol, int cpha) {
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);
    
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // Turn on SPI1 clock domain (SPI1EN bit in APB2ENR)

    // Set SPI GPIO Pin Modes
    pinMode(SPI_CE, GPIO_OUTPUT); 
    pinMode(SPI_SCK, GPIO_ALT);
    pinMode(SPI_MISO, GPIO_ALT);
    pinMode(SPI_MOSI, GPIO_ALT);
    

    // Set output speed type to high for clock line
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED3);

    // Set AF Select registers to AF05 for SCK, MOSI, MISO
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL3, 0b101);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 0b101);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL4, 0b101);
    
    // Set baud rate divisor based on input
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);

    // Clear, then set CPOL and CPHA bits
    SPI1->CR1 &= ~(SPI_CR1_CPOL);
    SPI1->CR1 &= ~(SPI_CR1_CPHA);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);

    // Clear LSBFirst 
    SPI1->CR1 &= ~(SPI_CR1_LSBFIRST);

    // Clear SSM 
    SPI1->CR1 &= ~(SPI_CR1_SSM);

    // Set DS bits
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111);

    // Set MSTR
    SPI1->CR1 |= (SPI_CR1_MSTR);

    // Set SSOE
    SPI1->CR2 |= (SPI_CR2_SSOE);
    
    // Set FRXTH
    SPI1->CR2 |= (SPI_CR2_FRXTH);

    // Enable SPI
    SPI1->CR1 |= (SPI_CR1_SPE);
}
// Function to send and recieve a char over SPI
char spiSendReceive(char send) {
    while(!(SPI1->SR & SPI_SR_TXE)); // Wait until the transmit buffer is empty by checking if the status register matches the transmit buffer empty flag
    *(volatile char *) (&SPI1->DR) = send; // Write send char into transmit data register
    while(!(SPI1->SR & SPI_SR_RXNE)); // Wait until data recieved flag is set by checking if the status register matches the receive buffer not empty flag
    char recieved = (volatile char) SPI1->DR; // Store the recieved character
    return recieved; // Return received character
}