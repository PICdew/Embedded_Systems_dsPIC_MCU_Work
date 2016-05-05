#include "flexmotor.h"
#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"

void motor_init(/*uint8_t chan*/){
    //setup Timer 2
    CLEARBIT(T2CONbits.TON); // Disable Timer
    CLEARBIT(T2CONbits.TCS); // Select internal instruction cycle clock
    CLEARBIT(T2CONbits.TGATE); // Disable Gated Timer mode
    TMR2 = 0x00; // Clear timer register
    T2CONbits.TCKPS = 0b10; // Select 1:64 Prescaler

    CLEARBIT(IFS0bits.T2IF); // Clear Timer2 interrupt status flag
    CLEARBIT(IEC0bits.T2IE); // Disable Timer2 interrupt enable control bit
    PR2 = 4000; // Set timer period 20ms: 4000= 20*10^-3 * 12.8*10^6 * 1/64

//    if (chan == 0){ // x
        //setup OC8
        CLEARBIT(TRISDbits.TRISD7); /* Set OC8 as output */
        //OC8R = 3820; /* Set the initial duty cycle to 5ms*/
        //OC8RS = 3820;
        OC8CON = 0x0006; /* Set OC8: PWM, no fault check, Timer2 */

//    }else{ // y
        //setup OC7
        CLEARBIT(TRISDbits.TRISD6); /* Set OC7 as output */
        //OC7R = 3820;//3700;
        //OC7RS = 3820;//3700;
        OC7CON = 0x0006; /* Set OC8: PWM, no fault check, Timer2 */
//    }
    SETBIT(T2CONbits.TON); /* Turn Timer 2 on */
}

void motor_set_duty(uint8_t chan, uint16_t duty_us){
    double duty = ((double) duty_us)/1000.0;
    if (chan == 1){ // x
        //lcd_locate(0,5);
        //lcd_printf("x wid: %0.2f", duty);//_us);
        //lcd_printf("x wid: %3d", (uint16_t)duty_ms*200.0);//_us);
        OC8RS = (20.0-duty)*200.0; /* Load OCRS: next pwm duty cycle */
    }else{
        //lcd_locate(0,6);
        //lcd_printf("y wid: %0.2f", duty);//_us);
        //lcd_printf("y wid: %3d", (uint16_t)duty_ms*200.0);//_us);
        //OC7RS = (20.0-duty)*200.0; /* Load OCRS: next pwm duty cycle */
        //OC7R = (20.0-duty)*200.0;//3700;
        OC7RS = (20.0-duty)*200.0;//300;//3700;
    }
}