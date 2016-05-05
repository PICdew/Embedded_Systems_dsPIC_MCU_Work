#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  


void main(){
	//Init LCD, LED, and initialize some variables
	__C30_UART=1;
    int count = 0;
    int DBcount = 0;
    int Tcount = 0;
    int DBNOcount = 0;
    int LED4Status = 0;
    int PrevStat = 0;
	lcd_initialize();
	led_initialize();

    //clear and locate our lcd in the top left corner
	lcd_clear();
	lcd_locate(0,0);

    //Task 1
	lcd_printf("Eric Meyers\n");
	lcd_printf("Long Le\n");
	lcd_printf("Ahmed Abulila\n");

	// those PINs as output
	LEDTRIS &= ~(BV(LED1) | BV(LED2) | BV(LED3) | BV(LED4) | BV(LED5));
	CLEARLED(LED1_PORT);
	CLEARLED(LED2_PORT);
	CLEARLED(LED3_PORT);

	// put the shared PORTE PIN8 in digital mode
	SETBIT(AD1PCFGHbits.PCFG20); 

	// those PINs as input
	TRISEbits.TRISE8 = 1;
	TRISDbits.TRISD10 = 1;

	PrevStat = PORTEbits.RE8;
	Nop();

 	while(1){
            //Task 2
            if(count % 100000 == 0){
                TOGGLELED(LED4_PORT);
            }
            count++;

            //Task 6
            if(Tcount ==  2500){ //threshold will be 2500 1s before we count it as "pressed"
                if(DBcount > 250){
					// state changed declared
                    DBNOcount++;
                    PrevStat = PORTEbits.RE8;
                }
                //Print the pressed-count and reset the counters
                lcd_locate(0,3);
                lcd_printf("Count=%d",DBNOcount/2);
                lcd_locate(0,4);
                lcd_printf("Count=%x",DBNOcount/2);
                Tcount = 0;
                DBcount = 0;
            } else if ((PORTEbits.RE8 == PrevStat) && (Tcount != 0)) {
                Tcount++; //else than incrememnt 
            } else if (PORTEbits.RE8 != PrevStat) {
                DBcount++;
                Tcount++;
            }

            //Task 3 & 6
            if(PrevStat ==0){
                SETLED(LED1_PORT);
            } else {
                CLEARLED(LED1_PORT);
            }

            //Task 4
            if(PORTDbits.RD10 == 0) {
                SETLED(LED2_PORT);
            } else {
                CLEARLED(LED2_PORT);
            }

            //Task 5
            if(PORTDbits.RD10 == PORTEbits.RE8) {
                CLEARLED(LED3_PORT);
            } else {
                SETLED(LED3_PORT);
            }
            Nop();
	}
}

