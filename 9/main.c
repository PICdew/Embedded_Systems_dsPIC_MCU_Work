#include "lcd.h"
#include "types.h"
#include "led.h"
#include "flexmotor.h"
#include "flextouch.h"
#include "performance.h"
#include "math.h"
#include "stdlib.h"

#include <p33Fxxxx.h>

#define FCY 12800000UL

// control task frequency (Hz)
#define RT_FREQ 50

uint16_t TOUCH_MIN_X = 150;
uint16_t TOUCH_MAX_X = 3100;
uint16_t TOUCH_MIN_Y = 380;
uint16_t TOUCH_MAX_Y = 2700;


//setpoint parameters
#define SPEED 0.08  // tested up to .12!
#define RADIUS 400
#define CENTER_X (3250)/2 //(3100.0+300.0)/2.0
#define CENTER_Y (3080)/2 //(2755.0+438.0)/2.0

// Servo defines
#define MAX_DUTY_MICROSEC 2100
#define MIN_DUTY_MICROSEC 900
#define SERVO_PWM_PERIOD_MICROSEC 20000
#define INIT_DUTYX_MICROSEC 1410     // leveled plate on X axis: 1410
#define INIT_DUTYY_MICROSEC 1400     // leveled plate on Y axis: 1400
#define MOTOR_X_CHAN 1
#define MOTOR_Y_CHAN 2

// Touch screen definitions
#define X_DIM 1
#define Y_DIM 2


// do not change position of this include
#include <libpic30.h>

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);


// control setpoint
double Xpos_set = CENTER_X, Ypos_set = CENTER_Y;


// raw, unfiltered X and Y position of the ball
volatile double Xpos, Ypos;
volatile uint8_t start = 0;
volatile uint8_t select = X_DIM;
volatile uint8_t deadline_miss = 0;



/*===== Functions =====*/
double median(double* arr, int n);
int compare (const void * a, const void * b);
double cap(double in, double up, double low);
double smooth(double in, double up, double low, double prev);


double xVal[] = {0,0,0,0,0};
double yVal[] = {0,0,0,0,0};
double xPrevVal = 0;
double yPrevVal = 0;
int N = 5;
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
double dt = 0.01; // second
double KpX = 0.190, KiX = 0.00 , KdX = 1.45;
double KpY = 0.190, KiY = 0.00 , KdY = 1.45;
//double KpX = 0.176, KiX = 0.00 , KdX = 0.49;
//double KpY = 0.15, KiY = 0.00 , KdY = 0.56;

double pidX_controller(double Xp) {
    // TODO: Implement PID X
    double pid;
    errorX = Xpos_set - Xp;
    integralX = integralX + errorX*dt;
    derivativeX = (errorX - prevErrX);
    outputX = KpX*errorX + KiX*integralX + KdX*derivativeX;
//  outputX = cap(outputX,8000.0,-8000.0);

//    pid = (outputX + 8000.0)/(8000.0+8000.0)*1.2 + 0.9;
    pid =outputX;

    prevErrX = errorX;
    return pid;
}


double pidY_controller(double Yp) {
    // TODO: Implement PID Y
    double pid;
    errorY = Ypos_set - Yp;
    integralY = integralY + errorY*dt;
    derivativeY = (errorY - prevErrY);
    outputY = KpY*errorY + KiY*integralY + KdY*derivativeY;
//    outputY = cap(outputY,8000.0,-8000.0);

//    pid = (outputY + 8000.0)/(8000.0+8000.0)*1.2 + 0.9;
    prevErrY = errorY;
    pid = outputY;

    return pid;
}


// Configure the real-time task timer and its interrupt.
void timers_initialize() {

  //Set Timer1 to generate an interrupt every 10ms (100Hz) ==> PR1=500
  CLEARBIT(T1CONbits.TON); //Disable Timer
  CLEARBIT(T1CONbits.TCS); //Select internal instruction cycle clock
  CLEARBIT(T1CONbits.TGATE); //Disable Gated Timer mode
  T1CONbits.TCKPS = 0b11; //Select 1:256 Prescaler
  PR1 = 500; //Load the period value ==> running at 100Hz now!
  TMR1 = 0x00; //Clear timer register
  IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
  CLEARBIT(IFS0bits.T1IF); // Clear Timer1 Interrupt Flag
  SETBIT(IEC0bits.T1IE); // Enable Timer1 interrupt
  SETBIT(T1CONbits.TON); // Start Timer
}

int main(){
  uint8_t start_r, old_IPL;
  uint8_t hz50_scaler, hz5_scaler, hz1_scaler, sec;
  uint32_t tick = 0;

  hz50_scaler = hz5_scaler = hz1_scaler = sec = 0;

  touch_init();

  __delay_ms(200);
  lcd_initialize();             // Initialize the LCD 

  motor_init();

  lcd_clear();
  lcd_locate(0,0);
  lcd_printf("-- Ball position: --");

  timers_initialize();          // Initialize timers

  while (1) {
    start_r = 0;
    while(!start_r) {      
      // disable all maskable interrupts
      SET_AND_SAVE_CPU_IPL(old_IPL, 7);
      start_r = start;

      // enable all maskable interrupts
      RESTORE_CPU_IPL(old_IPL);
    }

    // Periodic real-time task code starts here = CENTER_X + RADIUS * cos(tick * SPEED);
//      Ypos_set = CENT!!!
    double pidX, pidY;
    uint16_t duty_us_x, duty_us_y;

    // 50Hz control task
    if(hz50_scaler == 0) {
      calcQEI(Xpos_set, Xpos, Ypos_set, Ypos);
//      Xpos_set = CENTER_X;
//      Xpos_set = CENTER_Y;
      Xpos_set = CENTER_X + RADIUS * cos(tick * SPEED);
      Ypos_set = CENTER_Y + RADIUS * sin(tick * SPEED);
      tick++;


      pidX = pidX_controller(Xpos);
      pidY = pidY_controller(Ypos);

      // TODO: Convert PID to motor duty cycle (900-2100 us)

      // setMotorDuty is a wrapper function that calls your motor_set_duty
      // implementation in flexmotor.c. The 2nd parameter expects a value
      // between 900-2100 us
//      duty_us_x = cap((pidX*1000.0), 2100, 900);
//      duty_us_y = cap((pidY*1000.0), 2100, 900);

      duty_us_x = cap((pidX + 1500), 2100, 900);
      duty_us_y = cap((pidY + 1500), 2100, 900);
      motor_set_duty(1, duty_us_x);
      motor_set_duty(2, duty_us_y+100);
//      setMotorDuty(MOTOR_X_CHAN, duty_us_x);
//      setMotorDuty(MOTOR_Y_CHAN, duty_us_y);
    }

    // 5Hz display task
    if(hz5_scaler == 0) {
//      lcd_locate(0,1);
//      lcd_printf("Xp=%.1f,Yp=%.1f", Xpos, Ypos);
//      lcd_locate(0,2);
//      lcd_printf("X*=%.1f, Y*=%.1f", Xpos_set, Ypos_set);
//      lcd_locate(0,3);
//      lcd_printf("pX=%.1f,pY=%.1f", pidX, pidY);
//      lcd_locate(0,4);
//      lcd_printf("dx=%u, dY=%u", duty_us_x, duty_us_y);
//
      if(deadline_miss >= 1) {
        lcd_locate(0,6);
        lcd_printf("%4d d_misses!!!", deadline_miss);
      }
    }

    // 1Hz seconds display task
    if(hz1_scaler == 0) {
      lcd_locate(0,7);
      lcd_printf("QEI: %5u", getQEI());
      sec++;
    }
    
    hz50_scaler = (hz50_scaler + 1) % 2;
    hz5_scaler = (hz5_scaler + 1) % 20;
    hz1_scaler = (hz1_scaler + 1) % 100;

    start = 0;
  }

  return 0;
}

// This ISR will execute whenever Timer1 has a compare match.
// it kicks off the periodic execution of user code and performs I/O
// Min period: 10msec due to X,Y switch time for touchscreen
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
  IFS0bits.T1IF = 0; // clear interrupt flag

  if(start == 1)
    deadline_miss++;

  if (select == X_DIM) {
    // DONE: read 5 samples from X-dimension and set Xpos as the median
    int i = 0;
    for (i = 0; i<N; i ++){
        xVal[i] = smooth(touch_adc(1), TOUCH_MAX_X, TOUCH_MIN_X,xPrevVal);
//        xVal[i] = touch_adc(1);//cap(touch_adc(1), TOUCH_MAX_X, TOUCH_MIN_X);
    }
    Xpos = median(xVal,N);
    xPrevVal = Xpos;
    touch_select_dim(Y_DIM);
    select = Y_DIM;
  }
  else {
    // DONE: read 5 samples from Y-dimension and set Ypos as the median
    int i = 0;
    for (i = 0; i<N; i ++){
        yVal[i] = smooth(touch_adc(2), TOUCH_MAX_Y, TOUCH_MIN_Y,yPrevVal);
//         yVal[i] = touch_adc(2);//cap(touch_adc(2), TOUCH_MAX_Y, TOUCH_MIN_Y);
    }

    Ypos = median(yVal,N);
    yPrevVal = Ypos;
    touch_select_dim(X_DIM);
    select = X_DIM;
  }
  start = 1;
}


double median(double* arr, int n){
    //  assume n is odd
    qsort(arr, n, sizeof(double), compare);
    return arr[(n+1)/2];
}

int compare (const void * a, const void * b){
	return (int)( *(double*)a - *(double*)b );
}

double cap(double in, double up, double low){
//    return in;
    if (in > up){
            return up;
    } else if (in < low){
            return low;
    } else{
            return in;
    }
}
double smooth(double in, double up, double low, double prev){
//    return in;
    if (in > up){
            return prev;
    } else if (in < low){
            return prev;
    } else{
            return in;
    }
}
