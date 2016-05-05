#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <libpic30.h>
#include "lcd.h"

#include "types.h"

uint16_t getX();
uint16_t getY();

void touch_init(){//uint8_t chan){
    //set up the I/O pins E1, E2, E3 to be output pins
    CLEARBIT(TRISEbits.TRISE1); //I/O pin set to output
    CLEARBIT(TRISEbits.TRISE2); //I/O pin set to output
    CLEARBIT(TRISEbits.TRISE3); //I/O pin set to output

    // ADC init
    //disable ADC
    CLEARBIT(AD1CON1bits.ADON);
    //initialize PIN
    SETBIT(TRISBbits.TRISB15); //set TRISE RB15 to input
    SETBIT(TRISBbits.TRISB9); //set TRISE RB9 to input
    SETBIT(AD1PCFGLbits.PCFG15); // RB15 analog ADC1CH15
    SETBIT(AD1PCFGLbits.PCFG9); // RB9 analog ADC1CH9
    //Configure ADxCON1
    SETBIT(AD1CON1bits.AD12B); //set 12b Operation Mode
    AD1CON1bits.FORM = 0; //set integer output
    AD1CON1bits.SSRC = 0x7; //set automatic conversion
    //Configure ADxCON2
    AD1CON2 = 0; //not using scanning sampling
    //Configure ADxCON3
    CLEARBIT(AD1CON3bits.ADRC); //internal clock source
    AD1CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD1CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    //Leave ADxCON4 at its default value
    //enable ADC
    SETBIT(AD1CON1bits.ADON);
}

void touch_select_dim(uint8_t dim){
    /*
    if (dim == 1){ // x
        //set up the I/O pins E1, E2, E3 so that the touchscreen X-coordinate pin
        //connects to the ADC
        CLEARBIT(PORTEbits.RE1);
        SETBIT(PORTEbits.RE2);
        SETBIT(PORTEbits.RE3);
    }else if (dim == 2){ // y
        SETBIT(PORTEbits.RE1);
        CLEARBIT(PORTEbits.RE2);
        CLEARBIT(PORTEbits.RE3);
    } else{
        SETBIT(PORTEbits.RE1);
        SETBIT(PORTEbits.RE2);
        CLEARBIT(PORTEbits.RE3);
    }
    */
    if (dim == 1){ // x
        //set up the I/O pins E1, E2, E3 so that the touchscreen X-coordinate pin
        //connects to the ADC
        CLEARBIT(LATEbits.LATE1);
        SETBIT(LATEbits.LATE2);
        SETBIT(LATEbits.LATE3);
    }else if (dim == 2){ // y
        SETBIT(LATEbits.LATE1);
        CLEARBIT(LATEbits.LATE2);
        CLEARBIT(LATEbits.LATE3);
    } else{
        SETBIT(LATEbits.LATE1);
        SETBIT(LATEbits.LATE2);
        CLEARBIT(LATEbits.LATE3);
    }
}

uint16_t touch_adc(uint8_t dim){
    if (dim == 1){
        return getX();
    } else if (dim==2){
        return getY();
    }
}

uint16_t getX(void){
    AD1CHS0bits.CH0SA = 0x00F; //set ADC to Sample AN15 pin
    //AD1CHS0bits.CH0NA = 0;
    SETBIT(AD1CON1bits.SAMP); //start to sample
    while(!AD1CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD1CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC1BUF0; //return sample
}
uint16_t getY(void){
    AD1CHS0bits.CH0SA = 0x009; //set ADC to Sample AN9 pin
    //AD1CHS0bits.CH0NA = 0;
    SETBIT(AD1CON1bits.SAMP); //start to sample
    while(!AD1CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD1CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC1BUF0; //return sample
}