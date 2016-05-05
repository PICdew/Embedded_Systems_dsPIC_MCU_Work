#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"
#include "flexserial.h"
#include "crc16.h"
#include "lab03.h"

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
	// LCD init
	__C30_UART=1;
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
        // ==== LED init
	led_initialize();

        // ==== LPOSCEN init
	__builtin_write_OSCCONL(OSCCONL | 2);
        
	// ==== TIMER 1 Initialization
	T1CONbits.TON = 0; //Disable Timer
	T1CONbits.TCS = 1; //Select external clock
	T1CONbits.TSYNC = 0; //Disable Synchronization
	T1CONbits.TCKPS = 0b00; //Select 1:1 Prescaler
	TMR1 = 0x00; // Clear timer register
	PR1 = 32767; // Load the period value, 1 s
	IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
	IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
        IEC0bits.T1IE = 1; // Enable Timer1 interrupt

	// ==== UART 2 Initialization
	uart2_init(9600);

	// ==== LOCAL VARIABLES
	uint8_t c;	//data variable
	uint16_t rCRC = 0; // remote CRC
	uint16_t lCRC = 0; // local CRC
	int16_t i = 0, j = 0; //loop variables
	uint16_t N; //message length
        uint16_t nFails = 0; //number of fails
        char buff[256];

	while(1){
		/*Spin and wait for a message*/
                if(i<4)
                    c = uart2_getc();
                /*if (i==2){
                    lcd_locate(0,6);
                    lcd_printf("after i==2");
                }*/
		/*MESSAGE START*/
		if (c == MSG_START && i == 0){
                        
		}
		else {
                        //lcd_locate(0,0);
                        //lcd_printf("Seen 1st Byte\n");
                        //lcd_locate(0,1);
                        //lcd_printf("Loop Value: %4d\n",i);

			switch(i){
				case 1:		/*CRC ONE*/
                                        //lcd_locate(0,2);
                                        //lcd_printf("CRC Initial: %4x\n", rCRC);
					rCRC = (c << 8);
                                        //lcd_locate(0,3);
                                        //lcd_printf("Received CRC1: %4x\n", rCRC);
					break;
				case 2:		/*CRC TWO*/
					rCRC |= c;
                                        //lcd_locate(0,4);
                                        //lcd_printf("Received CRC2: %4x\n", rCRC);
					break;
				case 3:		/*MESSAGE LENGTH*/
					N = c;
                                        //lcd_locate(0,3);
                                        //lcd_printf("Message Length: %d\n", N);
					break;
				case 4:		/*MESSAGE GATHER AND CRC UPDATE*/
                                        //lcd_locate(0,7);
                                        //lcd_printf("Message:%s\n",c);
                                        SETLED(LED4_PORT);
                                        TMR1 = 0x00;
                                        T1CONbits.TON = 1;
					for (j = 0; j < N; j++){
						c = uart2_getc();
						if (isCorrupt)
							break;
                                                buff[j] = c;
						lCRC = crc_update(lCRC,c);
					}
                                        CLEARLED(LED4_PORT);
                                        //lcd_locate(0,4);
                                        //lcd_printf("remote CRC: %4x\n", lCRC);
                                        //lcd_locate(0,3);
                                        //lcd_printf("%d\n",i);
					if (isCorrupt){
						nFails++;

                                                lcd_locate(0,3);
                                                lcd_printf("Recv fail: %4d\n", nFails);
                                                uart2_putc(MSG_NACK);
					}else{
                                            // reset on done
                                            if (rCRC == lCRC){
                                                    //T1CONbits.TON = 0;
                                                    lcd_clear();
                                                    lcd_locate(0,3);
                                                    lcd_printf("Recv fail: %4d\n", nFails);
                                                    lcd_locate(0,4);
                                                    lcd_printf("remote CRC: %4x\n", rCRC);
                                                    lcd_locate(0,5);
                                                    buff[j] = 0;
                                                    lcd_printf("Msg: %s\n", buff);

                                                    uart2_putc(MSG_ACK);
                                                    nFails = 0;
                                            }else{
                                                nFails++;

                                                lcd_locate(0,3);
                                                lcd_printf("Recv fail: %4d\n", nFails);
                                                uart2_putc(MSG_NACK);
                                            }
                                            //lcd_locate(0,5);
                                            //lcd_printf("local CRC: %4x\n", lCRC);
                                        }
                                        // reset on corrupt data
                                        T1CONbits.TON = 0;
                                        isCorrupt = 0;
                                        rCRC = 0;
                                        lCRC = 0;
                                        i = -1;

					break;
                                /*
				default:
					// reset on done
                                        lcd_locate(0,4);
                                        lcd_printf("remote CRC: %4x\n", rCRC);
                                        lcd_locate(0,5);
                                        lcd_printf("local CRC: %4x\n", lCRC);
					if (rCRC == lCRC){
						//T1CONbits.TON = 0;
						//nFails = 0;
						uart2_putc(MSG_ACK);
					}
					rCRC = 0;
					lCRC = 0;
					i = -1;
					break;
                                */
			}
		}
                /*if (i==1){
                    lcd_locate(0,5);
                    lcd_printf("reached i==2");
                }*/
		i++;

		/*if (c != MSG_START && i == 0){
			continue;
		}
		else
		{
			if (i == 0){
				T1CONbits.TON = 1;
			}else if (i == 1){
				rCRC |= c << 8;
			}else if (i == 2){
				rCRC |= c;
			}else if (i == 3){
				N = c;
			}else if (i >= 4 && i <= N+1){
				lCRC = crc_update(lCRC,c);
				if (isCorrupt){

					T1CONbits.TON = 0;
					isCorrupt = 0;
					nFails++;

					// reset on corrupt data
					rCRC = 0;
					lCRC = 0;
					i = 0;
				}
			} else {
				// reset on done
				if (rCRC == lCRC){
					T1CONbits.TON = 0;
					nFails = 0;
					uart2_putc(MSG_ACK);
				}
				rCRC = 0;
				lCRC = 0;
				i = 0;
			}*/
	}
}

void __attribute__ ((__interrupt__)) _T1Interrupt(void)
{
    CLEARBIT(IFS0bits.T1IF); // clear the interrupt flag
    isCorrupt = 1;
    //SETLED(LED1_PORT);
}