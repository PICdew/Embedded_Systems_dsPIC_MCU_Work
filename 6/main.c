#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <stdlib.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"
#include "flextouch.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  

int state = 0;

int median(uint16_t* arr, int n);
int compare (const void * a, const void * b);

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

        touch_init();
        touch_select_dim(0); // standby
        delay(100000);

        uint16_t xVal[] = {0,0,0,0,0};
        uint16_t yVal[] = {0,0,0,0,0};
        int idx = 0;

        while(1){
            touch_select_dim(1); // x
            delay(100000);
            xVal[idx] = touch_adc(1);
            
            lcd_locate(0,1);
            lcd_printf("xm: %4d",median(xVal,5));
            //lcd_printf("x: %4d",xVal[idx]);

            lcd_locate(0,2);
            lcd_printf("x: [%4d,%4d,%4d,%4d,%4d]",xVal[0],xVal[1],xVal[2],xVal[3],xVal[4]);
            
            touch_select_dim(2); // y
            delay(100000);
            yVal[idx] = touch_adc(2);
            
            lcd_locate(0,4);
            lcd_printf("ym: %4d",median(yVal,5));
            //lcd_printf("y: %4d",yVal[idx]);

            lcd_locate(0,5);
            lcd_printf("y: [%4d,%4d,%4d,%4d,%4d]",yVal[0],yVal[1],yVal[2],yVal[3],yVal[4]);

            idx++;
            if (idx == 5){
                idx = 0;
            }
        }

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