#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <stdlib.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"
#include "flextouch.h"
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

int median(uint16_t* arr, int n);
int compare (const void * a, const void * b);
void delay(uint16_t delay);

int start = 0;
uint16_t xVal[] = {0,0,0,0,0};
double duty = 0.9;

void main(){
	// LCD init
	__C30_UART=1;
	lcd_initialize();
	lcd_clear();

	// ==== LED init
	led_initialize();
	CLEARLED(LED1_PORT);
	CLEARLED(LED2_PORT);

        // ==== LPOSCEN init
	__builtin_write_OSCCONL(OSCCONL | 2);
	// ==== timer 1 init
	T1CONbits.TON = 0; //Disable Timer
	T1CONbits.TCS = 0; //Select external clock
	T1CONbits.TSYNC = 0; //Disable Synchronization
	T1CONbits.TCKPS = 0b10; //Select 1:64 Prescaler
	TMR1 = 0x00; // Clear timer register
	PR1 = 10000; // Load the period value, 50 ms
	IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
	IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
	IEC0bits.T1IE = 1; // Enable Timer1 interrupt
	T1CONbits.TON = 1; // Start Timer

	// ==== touch init
        touch_init();
        touch_select_dim(1); // x
        delay(100000);

	// ==== motor init
	motor_init(0);
	motor_init(1);

	// initial y position
        motor_set_duty(0, 0.9);
	motor_set_duty(1, 2.1);

        SETLED(LED1_PORT);

        int xMedian = 0;
        int count = 0;
        double prevErr = 0;
        double error = 0;
        double derivative = 0;
        double integral = 0;
        double output = 0;
        double dt = 0.05; // second
        double setPoint = (3135.0+265.0)/2.0;
        double Kp = 1.0, Ki = 0.0, Kd = 0.2;

        while(1){
            while(!start);
            start = 0;

            // compute the median
            xMedian = median(xVal,5);

            // perform PID computation
            error = setPoint - xMedian;
            integral = integral + error*dt;
            derivative = (error - prevErr)/dt;
            output = Kp*error + Ki*integral + Kd*derivative;
            duty = (output + 1100.0)/(1400.0+1100.0)*1.2 + 0.9;
            prevErr = error;

            //update display value
            if (count == 6){
                //lcd_clear();
                lcd_locate(0,0);
                lcd_printf("PID: %.2f,%.2f,%.2f",Kp,Ki,Kd);
                lcd_locate(0,1);
                lcd_printf("xm: %4d",xMedian);
                lcd_locate(0,2);
                lcd_printf("der: %.2f",derivative);
                lcd_locate(0,3);
                lcd_printf("int: %.2f",integral);
                lcd_locate(0,4);
                lcd_printf("err: %.2f",error);
                lcd_locate(0,5);
                lcd_printf("F_x: %.2f",output);
                lcd_locate(0,6);
                lcd_printf("duty: %.3f",duty);
                count = 0;
            }
            count++;
        }

}

void __attribute__ ((__interrupt__)) _T1Interrupt(void){
    int i = 0;

    IFS0bits.T1IF = 0;

    TOGGLELED(LED2_PORT);

    // Do control in here
    // read the ball position
    for (i = 0; i < 5; i++){
        xVal[i] = touch_adc(1);
    }
    // set PID x value
    if(duty < 0.9)
        duty = 0.9;
    if(duty > 2.1)
        duty = 2.1;
    motor_set_duty(0, duty);
    motor_set_duty(1, 2.1);
    /*if(duty>=0.9 && duty<=2.1){
        motor_set_duty(0, duty);
    }*/
    // allow PID computation
    start = 1;
}

int median(uint16_t* arr, int n){
    //  assume n is odd
    qsort(arr, n, sizeof(uint16_t), compare);
    return arr[(n+1)/2];
}

int compare (const void * a, const void * b){
	return ( *(uint16_t*)a - *(uint16_t*)b );
}

void delay(uint16_t delay){
    uint16_t i;
    for (i = 0; i < delay; i++);
}