#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"
#include "flexmotor.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  

int PrevStat = 0;
int state = 0;

uint16_t getX();
uint16_t getY();

void main(){
	// LCD init
	__C30_UART=1;
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);

	// ==== LED init
	led_initialize();
	CLEARLED(LED1_PORT);
	CLEARLED(LED2_PORT);

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

        motor_init(0);
        motor_init(1);

        uint16_t xVal = 0;
        uint16_t xValMin = 0;
        uint16_t xValMax = 0;
        uint16_t xServo = 0;

        uint16_t yVal = 0;
        uint16_t yValMin = 0;
        uint16_t yValMax = 0;
        uint16_t yServo = 0;

        while (state == 0){
            xVal = getX();
            lcd_locate(0,1);
            lcd_printf("x max:%4d",xVal);
        }
        xValMax = xVal;
        while (PORTEbits.RE8 == 0);

        while (state == 1){
            xVal = getX();
            lcd_locate(0,2);
            lcd_printf("x min:%4d",xVal);
        }
        xValMin = xVal;
        while (PORTEbits.RE8 == 0);

        while (state == 2){
            yVal = getY();
            lcd_locate(0,3);
            lcd_printf("y max:%4d",yVal);
        }
        yValMax = yVal;
        while (PORTEbits.RE8 == 0);

        while (state == 3){
            yVal = getY();
            lcd_locate(0,4);
            lcd_printf("y min:%4d",yVal);
        }
        yValMin = yVal;
        while (PORTEbits.RE8 == 0);

        double minTime = 0.9;
        double maxTime = 2.1;
        double range = maxTime - minTime;
        uint16_t yRange = yValMax - yValMin;
        uint16_t xRange = xValMax - xValMin;
        double duty = 0;
        while (state == 4){
            xVal = getX();
            duty = (((double)xVal - (double)xValMin)/(double)xRange)*range + minTime;

            //lcd_locate(0,5);
            //lcd_printf("x wid: %0.2f", duty);//_us);
            if(duty>=0.9 && duty<=2.1){
                motor_set_duty(0, duty);
            }
        }
        while (PORTEbits.RE8 == 0);

        while (state == 5){
            yVal = getY();
            duty = (((double)yVal - (double)yValMin)/(double)yRange)*range + minTime;

            //lcd_locate(0,6);
            //lcd_printf("y wid: %0.2f", duty);//_us);
            if(duty>=0.9 && duty<=2.1){
                motor_set_duty(1, duty);
            }
        }
        while (PORTEbits.RE8 == 0);

        /*
        motor_set_duty(0, 900);
        motor_set_duty(1, 900);
         */
        while(1);

        //lcd_locate(0,7);
        //lcd_printf("%d, %d, %d, %d",xValMax,xValMin,yValMax,yValMin);

        //lcd_locate(0,7);
        //lcd_printf("%d, %d, %d, %d",xValMax,xValMin,yValMax,yValMin);

        //motor_set_duty(0, uint16_t duty_x_us);
        //motor_set_duty(1, uint16_t duty_y_us);
}

//int DBNOcount = 0;

uint16_t getX(void){
    AD2CHS0bits.CH0SA = 0x004; //set ADC to Sample AN4 pin
    SETBIT(AD2CON1bits.SAMP); //start to sample
    while(!AD2CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC2BUF0; //return sample
}
uint16_t getY(void){
    AD2CHS0bits.CH0SA = 0x005; //set ADC to Sample AN5 pin
    SETBIT(AD2CON1bits.SAMP); //start to sample
    while(!AD2CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC2BUF0; //return sample
}

void __attribute__ ((__interrupt__)) _INT1Interrupt(void){
	IFS1bits.INT1IF = 0;
        uint32_t DBcount = 0;
        uint32_t Tcount = 0;

        for (Tcount = 0; Tcount < 2500; Tcount ++ ){
            if (PORTEbits.RE8 != PrevStat)
                DBcount++;
        }


        if(DBcount > 250){
            // state changed declared
            PrevStat = PORTEbits.RE8;
            state++;
        }
        Tcount = 0;
        DBcount = 0;
}

