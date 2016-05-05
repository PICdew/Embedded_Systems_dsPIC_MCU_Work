#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <libpic30.h>
#include "lcd.h"
#include "led.h"
#include "types.h"
//#include "joystick.h"

int PrevStat;// = 0;
int state = 0;
uint16_t jgetX();
uint16_t jgetY();

void joystick_init(){
    // ADC init
    //disable ADC
    CLEARBIT(AD2CON1bits.ADON);
    //initialize PIN
    SETBIT(TRISBbits.TRISB4); //set TRISE RB4 to input
    SETBIT(TRISBbits.TRISB5); //set TRISE RB5 to input
    SETBIT(AD2PCFGLbits.PCFG4); // RB4 analog ADC2CH4
    SETBIT(AD2PCFGLbits.PCFG5); // RB5 analog ADC2CH5
    //Configure AD2CON1
    SETBIT(AD2CON1bits.AD12B); //set 12b Operation Mode
    AD2CON1bits.FORM = 0; //set integer output
    AD2CON1bits.SSRC = 0x7; //set automatic conversion
    //Configure AD1CON2
    AD2CON2 = 0; //not using scanning sampling
    //Configure AD1CON3
    CLEARBIT(AD2CON3bits.ADRC); //internal clock source
    AD2CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD2CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    //Leave AD1CON4 at its default value
    //enable ADC
    SETBIT(AD2CON1bits.ADON);

    // ==== button 1/external int 1 init
    AD1PCFGHbits.PCFG20 = 1;
    TRISEbits.TRISE8 = 1;
    INTCON2bits.INT1EP = 1; // polarity selection, '1' falling edge triggering of the interrupt.
    IPC5bits.INT1IP = 0x01;
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 1;
    PrevStat = PORTEbits.RE8;
}

double joystick_adc(uint8_t dim){
    if (dim == 1){
        return (double)jgetX();
    } else if (dim==2){
        return (double)jgetY();
    }
}

uint16_t jgetX(void){
    AD2CHS0bits.CH0SA = 0x004; //set ADC to Sample AN4 pin
    SETBIT(AD2CON1bits.SAMP); //start to sample
    while(!AD2CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC2BUF0; //return sample
}
uint16_t jgetY(void){
    AD2CHS0bits.CH0SA = 0x005; //set ADC to Sample AN5 pin
    SETBIT(AD2CON1bits.SAMP); //start to sample
    while(!AD2CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC2BUF0; //return sample
}
