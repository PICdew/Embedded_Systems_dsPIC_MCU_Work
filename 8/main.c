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
#include "joystick.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

double median(double* arr, int n);
int compare (const void * a, const void * b);
void delay(uint16_t delay);
double cap(double in, double up, double low);
uint32_t DBcount = 0;
int start = 0;
double xVal[] = {0,0,0,0,0,0,0,0,0,0};
double yVal[] = {0,0,0,0,0,0,0,0,0,0};
uint16_t N = 5;
uint16_t idx = 0;
double dutyX = 0.9;
double dutyY = 0.9;
double xVal1 = 0;
double yVal1 = 0;
double xValMin = 0;
double xValMax = 0;
double yValMin = 0;
double yValMax = 0;
const double setPointXMax = 2896.0;
const double setPointXMin = 357.0;
double setPointX = ((2896.0+357.0)/2.0);
const double setPointYMax = 2380.0;
const double setPointYMin = 490.0;
double setPointY = (2380.0+490.0)/2.0;
int lpFlg = 0;
int intFlg = 0;
void main(){
	// LCD init
	__C30_UART=1;
	lcd_initialize();
	lcd_clear();

	// ==== LED init
	led_initialize();
	CLEARLED(LED1_PORT);
	CLEARLED(LED2_PORT);
        CLEARLED(LED3_PORT);

        // ==== LPOSCEN init
	__builtin_write_OSCCONL(OSCCONL | 2);

        // joystick init
        joystick_init();

        // Read max/min x/y as input
        while (state == 0){
            xVal1 = joystick_adc(1);
            lcd_locate(0,0);
            lcd_printf("xMax: %4.1f",xVal1);
            if(PrevStat == 0)
                state++;
        }
        xValMax = xVal1;
        while (PORTEbits.RE8 == 0);
        PrevStat = 1;
        while (state == 1){
            xVal1 = joystick_adc(1);
            lcd_locate(0,1);
            lcd_printf("xMin: %4.1f",xVal1);
            if(PrevStat == 0)
                state++;
        }
        xValMin = xVal1;
        while (PORTEbits.RE8 == 0);
        PrevStat = 1;

        while (state == 2){
            yVal1 = joystick_adc(2);
            lcd_locate(0,2);
            lcd_printf("yMax: %4.1f",yVal1);
            if(PrevStat == 0)
                state++;
        }
        yValMax = yVal1;
        while (PORTEbits.RE8 == 0);
        PrevStat = 1;

        while (state == 3){
            yVal1 = joystick_adc(2);
            lcd_locate(0,3);
            lcd_printf("yMin: %4.1f",yVal1);
            if(PrevStat == 0)
                state++;
        }
        yValMin = yVal1;
        while (PORTEbits.RE8 == 0);

        PrevStat = 1;
        SETLED(LED1_PORT);

        // ==== timer 1 init
	T1CONbits.TON = 0; //Disable Timer
	T1CONbits.TCS = 0; //Select external clock
	T1CONbits.TSYNC = 0; //Disable Synchronization
	T1CONbits.TCKPS = 0b10; //Select 1:64 Prescaler
	TMR1 = 0x00; // Clear timer register
	PR1 = 5000; // Load the period value, 50 ms (5000 -> 25 ms)
	IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
	IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
	IEC0bits.T1IE = 1; // Enable Timer1 interrupt
	T1CONbits.TON = 1; // Start Timer

        // ==== touch init
        touch_init();

	// ==== motor init
	motor_init(0);
	motor_init(1);

	// initial y position
        motor_set_duty(0, 0.9);
	motor_set_duty(1, 0.9);
        int count = 0;
        double xMedian = 0;
        double yMedian = 0;
        double prevErrX = 0;
        double prevErrY = 0;
        double errorX = 0;
        double errorY = 0;
        double derivativeX = 0;
        double derivativeY = 0;
        double integralX = 0;
        double integralY = 0;
        double outputX = 0;
        double outputY = 0;
        double dt = 0.05; // second
        double KpX = 1.2, KiX = 0.0, KdX = 1.2;
        double KpY = 1.2, KiY = 0.0, KdY = 1.2;
//        double KpX = 1.2, KiX = 0.0, KdX = 0.4;
//        double KpY = 1.0, KiY = 0.0, KdY = 0.8;
        int i = 0;
        touch_select_dim(1); // x
        delay(100000);
        lpFlg = 0;
        intFlg =0;
        for (i = 0; i<5; i ++)
            xVal[i] = touch_adc(1);
        xMedian = median(xVal,N);
        errorX = setPointX - xMedian;
        integralX = 0;//integralX + errorX*dt;
        derivativeX = (errorX - prevErrX)/dt;
        outputX = KpX*errorX + KiX*integralX + KdX*derivativeX;
        outputX = cap(outputX,6000.0,-6000.0);
        dutyX = (outputX + 6000.0)/(6000.0+6000.0)*1.2 + 0.9;
        prevErrX = 0;// errorX;
        touch_select_dim(2);
        while(1){
            intFlg = 1;
            if(PrevStat == 0){
                TOGGLELED(LED3_PORT);
                //TOGGLELED(LED2_PORT);
                //state++;
                //Set the points
                xVal1 = joystick_adc(1);
                yVal1 = joystick_adc(2);
                setPointX = ((xVal1-xValMin)/(xValMax-xValMin))*(setPointXMax-setPointXMin)+setPointXMin; // TODO: conversion
                setPointX = cap(setPointX,setPointXMax,setPointXMin);
                setPointY = ((yVal1-yValMin)/(yValMax-yValMin))*(setPointYMax-setPointYMin)+setPointYMin;
                setPointY = cap(setPointY,setPointYMax,setPointYMin);
                while(PORTEbits.RE8 == 0);
                PrevStat = 1;
                prevErrX =0;
                integralX = 0;
                prevErrY =0;
                integralY = 0;
            }
//            if( ps == 0)
//                    state++
            while(!start);// ps == 1);
            start = 0;
            if (lpFlg == 0){
                yMedian = median(yVal,N);
                errorY = setPointY - yMedian;
                integralY = integralY + errorY*dt;
                derivativeY = (errorY - prevErrY)/dt;
                outputY = KpY*errorY + KiY*integralY + KdY*derivativeY;
                outputY = cap(outputY,14000.0,-11000.0);
                //dutyY = cap(outputY, 2.1, 0.9);
                dutyY = (outputY + 11500.0)/(14000.0+11000.0)*1.2 + 0.9;
                prevErrY = errorY;
                lpFlg = 1;
            } else {
                xMedian = median(xVal,N);
                errorX = setPointX - xMedian;
                integralX = integralX + errorX*dt;
                derivativeX = (errorX - prevErrX)/dt;
                outputX = KpX*errorX + KiX*integralX + KdX*derivativeX;
                //dutyX = cap(outputX, 2.1, 0.9);
                outputX = cap(outputX,9500.0,-7000.0);
                dutyX = (outputX + 7500.0)/(9500.0+7000.0)*1.2 + 0.9;
                prevErrX = errorX;
                lpFlg = 0;
            }
            // compute the median
            /*
            xMedian = median(xVal,N);
            yMedian = median(yVal,N);
*/
            // perform PID computation


            //update display value
            if (count == 30){
                // get new set points
                xVal1 = joystick_adc(1);
                yVal1 = joystick_adc(2);
                while (PORTEbits.RE8 == 0);
//
//                lcd_clear();
///*                lcd_locate(0,0);
//                lcd_printf("PID: %.2f,%.2f,%.2f",KpX,KiX,KdX);
//                lcd_locate(0,1);
//                lcd_printf("int: %.2f,%.2f",integralX,integralY);
                lcd_locate(0,2);
                lcd_printf("der: %.2f,%.2f",derivativeX,derivativeY);
                lcd_locate(0,3);
                lcd_printf("err: %.2f,%.2f",errorX,errorY);
//                lcd_locate(0,4);
//                lcd_printf("cur: %4.1f,%4.1f", xMedian,yMedian);/*
//                lcd_printf("F_x: %.2f,%.2f",outputX,outputY);
//                lcd_locate(0,5);
//                lcd_printf("duty: %.3f,%.3f",dutyX, dutyY);
//                lcd_locate(0,6);
//                lcd_printf("cur: %4.1f,%4.1f",xVal1,yVal1);*/
                lcd_locate(0,7);
                lcd_printf("cur: %4.1f,%4.1f", xMedian,yMedian);
//                //lcd_printf("set: %4.1f,%4.1f",setPointX,setPointY);

                count = 0;
            }
            count++;
        }

}

void __attribute__ ((__interrupt__)) _T1Interrupt(void){
    int i = 0;

    IFS0bits.T1IF = 0;

    TOGGLELED(LED2_PORT);
    if (lpFlg == 0 && intFlg == 1){
        for (i=0; i<5; i++)
            yVal[i] = touch_adc(2);
        dutyX = cap(dutyX,2.1,0.9);
        motor_set_duty(0, dutyX);
        touch_select_dim(1);
        //intFlg = 1;
    } else if(lpFlg == 1 && intFlg == 1) {
        for (i=0; i<5; i++)
            xVal[i] = touch_adc(1);
        dutyY = cap(dutyY,2.1,0.9);
        motor_set_duty(1, dutyY);
        touch_select_dim(2);
        //intFlg = 0;
    }
    // read the ball position
    //touch_select_dim(1); // x
    //delay(100000);
	/*
    for (i = 0; i < N; i++){
        xVal[i] = touch_adc(1);
    }
	*/
    //xVal[idx] = touch_adc(1);

    //touch_select_dim(2); // y
    //delay(100000);
	/*
    for (i = 0; i < N; i++){
        yVal[i] = touch_adc(2);
    }SUCCESSFUL (total time: 2s)

    yVal[idx] = touch_adc(2);
    if (idx>=N-1){
            idx = 0;
    }else{
            idx++;
    }
    // Do control here
    //dutyX = cap(dutyX,2.1,0.9);
    //motor_set_duty(0, dutyX);
    //dutyY = cap(dutyY,2.1,0.9);
    //motor_set_duty(1, dutyY);
    //motor_set_duty(1, 2.1);
    /*if(duty>=0.9 && duty<=2.1){
        motor_set_duty(0, duty);
    }*/
    // allow PID computation*/
    if(intFlg == 1)
        start = 1;
}

double cap(double in, double up, double low){
    if (in > up){
            return up;
    } else if (in < low){
            return low;
    } else{
            return in;
    }
}

double median(double* arr, int n){
    //  assume n is odd
    qsort(arr, n, sizeof(double), compare);
    return arr[(n+1)/2];
}

int compare (const void * a, const void * b){
	return (int)( *(double*)a - *(double*)b );
}

void delay(uint16_t delay){
    uint16_t i;
    for (i = 0; i < delay; i++);
}

void __attribute__ ((__interrupt__)) _INT1Interrupt(void){
   // TOGGLELED(LED3_PORT);
    //uint32_t DBcount = 0;
    uint32_t Tcount;

    for (Tcount = 0; Tcount < 100; Tcount ++ ){
       // PrevStat = PORTEbits.RE8;
        if (PORTEbits.RE8 != PrevStat)
            DBcount++;
    }

    if(DBcount > 50){
        // state changed declared
//        if(PrevStat == 1)
        //state++;
        PrevStat = PORTEbits.RE8;
        //state++;
        if (state > 4){
            setPointX = ((xVal1-xValMin)/(xValMax-xValMin))*(setPointXMax-setPointXMin)+setPointXMin; // TODO: conversion
            setPointY = ((yVal1-yValMin)/(yValMax-yValMin))*(setPointYMax-setPointYMin)+setPointYMin;
        }
        DBcount = 0;
    }

    //Tcount = 0;
    DBcount = 0;
    IFS1bits.INT1IF = 0;
}
