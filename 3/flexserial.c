#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
//#include "led.h"

volatile int isCorrupt = 0;

void uart2_init(uint16_t baud){
	// ==== Stop UART port
	CLEARBIT(U2MODEbits.UARTEN); //Disable UART for configuration
	// Disable Interrupts
	IEC1bits.U2RXIE = 0;
	IEC1bits.U2TXIE = 0;
	// Clear Interrupt flag bits
	IFS1bits.U2RXIF = 0;
	IFS1bits.U2TXIF = 0;
	// Set IO pins
	TRISFbits.TRISF4 = 1; //set as input UART2 RX pin
	TRISFbits.TRISF5 = 0; //set as output UART2 TX pin
	// baud rate
	// use the following equation to compute the proper
	// setting for a specific baud rate
	U2MODEbits.BRGH = 0; //Set low speed baud rate
	U2BRG = (uint32_t)800000 / baud -1; //Set the baudrate to be at 9600
	// Operation settings and start port
	U2MODE = 0; // 8-bit, no parity and, 1 stop bit
	U2MODEbits.RTSMD = 0; //select simplex mode
	U2MODEbits.UEN = 0; //select simplex mode
	U2MODE |= 0x00;
	U2MODEbits.UARTEN = 1; //enable UART

        //uint8_t c = 0;
        //while(U2STAbits.URXDA)
        //    c = U2RXREG & 0x00FF;

	U2STA = 0;
	U2STAbits.UTXEN = 1; //enable UART TX
        //U2STAbits.URXEN = 1; //enable UART RX
}

int uart2_putc(uint8_t c){
	while (U2STAbits.UTXBF);// Wait on UTxBF bit in the UxSTA registe
	U2TXREG = c;// Load the UxTXREG register with an 8 bit value
	while(!U2STAbits.TRMT);// Wait on TRMT bit in the UxSTA register.
        return 0;
}

uint8_t uart2_getc(){
	//Check if there is data overflow error (by checking UxSTAbits.OERR). You must clear this bit if it has been set in order to receive new data.
	if (U2STAbits.OERR) {
		U2STAbits.OERR = 0;
	}
	//Check if there is data in the buffer (by checking UxSTAbits.URXDA. If yes, then read the data from UxRXREG.
	// non-blocking read
        /*
	if (U2STAbits.URXDA) {
		return (U2RXREG & 0x00FF);
	}else{
		return 0xFF;
	}
	*/
	// block read
	while (!U2STAbits.URXDA && !isCorrupt);
        return (U2RXREG & 0x00FF);
}
