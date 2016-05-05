#include <includes.h>


/*
*********************************************************************************************************
*                                                CONSTANTS
*********************************************************************************************************
*/

// control task frequency (Hz)
#define RT_FREQ 50

//setpoint parameters
#define SPEED 0.08  // tested up to .12!
#define RADIUS 350
#define CENTER_X 1650
#define CENTER_Y 1350

// motor definitions
#define MAX_DUTY_MICROSEC 2100
#define MIN_DUTY_MICROSEC 900

// Touch screen definitions
#define X_DIM 1
#define Y_DIM 2
#define TOUCH_MIN_X 150
#define TOUCH_MAX_X 3100
#define TOUCH_MIN_Y 380
#define TOUCH_MAX_Y 2700
/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/

OS_STK  AppStartTaskStk[APP_TASK_START_STK_SIZE];
// TODO define task stacks
OS_STK  AppLCDTaskStk[APP_TASK_LCD_STK_SIZE];
OS_STK  AppLEDTaskStk[APP_TASK_LED_STK_SIZE];
OS_STK  AppTouchTaskStk[APP_TASK_TOUCH_STK_SIZE];
OS_STK  AppMotorTaskStk[APP_TASK_MOTOR_STK_SIZE];

// control setpoint
CPU_INT16U Xpos_set = 1650.0, Ypos_set = 1550.0;

// raw, unfiltered X and Y position of the ball
CPU_INT16U Xpos, Ypos;

// filtered X and Y position of the ball
CPU_INT16U Xposf = 0.0, Yposf = 0.0;

CPU_INT16U duty_us_x = MIN_DUTY_MICROSEC, duty_us_y = MIN_DUTY_MICROSEC;

CPU_INT08U select = X_DIM;

double pidX = 0;
double pidY = 0;

/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppStartTask(void *p_arg);
static  void  AppTaskCreate(void);
// TODO declare function prototypes
static void AppLCDTask (void *p_arg);
static void AppLEDTask (void *p_arg);
static void AppTouchTask (void *p_arg);
static void AppMotorTask (void *p_arg);
uint16_t median(uint16_t* arr, uint16_t n);
uint16_t smooth(uint16_t in, uint16_t up, uint16_t low, uint16_t prev);
double cap(double in, double up, double low);
uint16_t butterFilt(uint16_t* in, uint16_t* out, uint16_t n, uint16_t N);
//static void LedTimer();

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.
* Arguments   : none
*********************************************************************************************************
*/
//OS_EVENT *semLED;

CPU_INT16S  main (void)
{
    CPU_INT08U  err;

    BSP_IntDisAll();                                                    /* Disable all interrupts until we are ready to accept them */
    OSInit();                                                           /* Initialize "uC/OS-II, The Real-Time Kernel"              */

    OSTaskCreateExt(AppStartTask,                                       /* Create the start-up task for system initialization       */
                    (void *)0,
                    (OS_STK *)&AppStartTaskStk[0],
                    APP_TASK_START_PRIO,
                    APP_TASK_START_PRIO,
                    (OS_STK *)&AppStartTaskStk[APP_TASK_START_STK_SIZE-1],
                    APP_TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *)"Start Task", &err);

    OSStart();                                                        /* Start multitasking (i.e. give control to uC/OS-II)       */
	return (-1);                                                        /* Return an error - This line of code is unreachable       */
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*               2) Interrupts are enabled once the task start because the I-bit of the CCR register was
*                  set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/

static  void  AppStartTask (void *p_arg)
{
    (void)p_arg;
	
    BSP_Init();                                                         /* Initialize BSP functions                                 */
    OSStatInit();                                                       /* Determine CPU capacity                                   */
    DispInit();
    // TODO initialize touchscreen and motors
    touch_init();
    motor_init();

    AppTaskCreate();                                                    /* Create additional user tasks                             */

    while (DEF_TRUE) {
	    OSTimeDlyHMSM(0, 0, 5, 0);
    }
}


/*
*********************************************************************************************************
*                              CREATE ADDITIONAL APPLICATION TASKS
*********************************************************************************************************
*/


static  void  AppTaskCreate (void)
{
    // TODO create tasks
    // Perform necessary initializations here
    CPU_INT08U  err;
    
    OSTaskCreateExt(AppLCDTask,
            (void *) 0,
            (OS_STK *) & AppLCDTaskStk[0], // pointer to the begin of the stack
            APP_TASK_LCD_PRIO,
            APP_TASK_LCD_PRIO, // task id; will be used in the future
            (OS_STK *) & AppLCDTaskStk[APP_TASK_LCD_STK_SIZE - 1], // pointer to the end of the stack
            APP_TASK_LCD_STK_SIZE,
            (void *) 0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); // stack checking related flags

    OSTaskCreateExt(AppLEDTask,
            (void *) 0,
            (OS_STK *) & AppLEDTaskStk[0], // pointer to the begin of the stack
            APP_TASK_LED_PRIO,
            APP_TASK_LED_PRIO, // task id; will be used in the future
            (OS_STK *) & AppLEDTaskStk[APP_TASK_LED_STK_SIZE - 1], // pointer to the end of the stack
            APP_TASK_LED_STK_SIZE,
            (void *) 0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); // stack checking related flags

    OSTaskCreateExt(AppTouchTask,
            (void *) 0,
            (OS_STK *) & AppTouchTaskStk[0], // pointer to the begin of the stack
            APP_TASK_TOUCH_PRIO,
            APP_TASK_TOUCH_PRIO, // task id; will be used in the future
            (OS_STK *) & AppTouchTaskStk[APP_TASK_TOUCH_STK_SIZE - 1], // pointer to the end of the stack
            APP_TASK_TOUCH_STK_SIZE,
            (void *) 0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); // stack checking related flags
    
    OSTaskCreateExt(AppMotorTask,
            (void *) 0,
            (OS_STK *) & AppMotorTaskStk[0], // pointer to the begin of the stack
            APP_TASK_MOTOR_PRIO,
            APP_TASK_MOTOR_PRIO, // task id; will be used in the future
            (OS_STK *) & AppMotorTaskStk[APP_TASK_MOTOR_STK_SIZE - 1], // pointer to the end of the stack
            APP_TASK_MOTOR_STK_SIZE,
            (void *) 0,
            OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); // stack checking related flags

    #if OS_TASK_NAME_SIZE > 9
        OSTaskNameSet(APP_TASK_LCD_PRIO, (CPU_INT08U *)"LCDTask", &err);
        OSTaskNameSet(APP_TASK_LED_PRIO, (CPU_INT08U *)"LEDTask", &err);
        OSTaskNameSet(APP_TASK_TOUCH_PRIO, (CPU_INT08U *)"TouchTask", &err);
        OSTaskNameSet(APP_TASK_MOTOR_PRIO, (CPU_INT08U *)"MotorTask", &err);
    #endif

//    void* LedTimerHandler=OSTmrCreate(0, 1000, OS_TMR_OPT_PERIODIC, LedTimer, (void *)0, (CPU_INT08U *)"LED Tmr", &err);
//    OSTmrStart(LedTimerHandler, &err);
}


static void AppLCDTask (void *p_arg){
    (void)p_arg; // avoid warning
    
    unsigned int i = 0;
    char buffer[32];

    DispClrScr();
    DispStr(0,0,"CS 431: Lab #10");

    LED_On(2);
    while (1) {
        OSTimeDlyHMSM(0, 0, 1, 0);

        sprintf(buffer,"%4d s since reset", i++);
        DispStr(1,0,buffer);
        sprintf(buffer,"Xp=%4u,Yp=%4u", Xpos,Ypos);
        DispStr(2,0,buffer);
        sprintf(buffer,"dX=%4u,dY=%4u", duty_us_x,duty_us_y);
        DispStr(3,0,buffer);
//        sprintf(buffer,"pX=%.1f,pY=%.1f", pidX,pidY);
//        DispStr(4,0,buffer);
    }
}

static void AppLEDTask(void *p_arg) {
    (void)p_arg;
    static uint8_t led = 1;

    while (DEF_TRUE)
    {
        // execute loop every second
//        CPU_INT08U err;
        //OSSemPend(semLED, 0, &err);
        // update active LED
        OSTimeDlyHMSM(0, 0, 1, 0);

        LED_Off(led);
        led = (led == 5 ? 1 : led+1);
        LED_On(led);
    }
}

static void AppTouchTask(void *p_arg) {
    (void)p_arg;

    uint16_t N = 5;
    uint16_t xVal[] = {0,0,0,0,0};
    uint16_t yVal[] = {0,0,0,0,0};
    uint16_t xOut[] = {0,0,0,0,0};
    uint16_t yOut[] = {0,0,0,0,0};
    uint16_t xPrevVal = 0;
    uint16_t yPrevVal = 0;

    while (DEF_TRUE)
    {
        OSTimeDlyHMSM(0, 0, 0, 10);

        if (select == X_DIM) {
            // DONE: read samples from X-dimension and set Xpos as the median
            int i = 0;
            for (i = 0; i<N; i ++){
//                xVal[i] = smooth(touch_adc(1), TOUCH_MAX_X, TOUCH_MIN_X,xPrevVal);
                xVal[i] = touch_adc(1);
                xOut[i] = butterFilt(xVal, xOut, i, N);
            }
//            Xpos = median(xVal,N);
            Xpos = xOut[N-1];
            xPrevVal = Xpos;
            touch_select_dim(Y_DIM);
            select = Y_DIM;
        } else {
            // DONE: read samples from Y-dimension and set Ypos as the median
            int i = 0;
            for (i = 0; i<N; i ++){
//                yVal[i] = smooth(touch_adc(2), TOUCH_MAX_Y, TOUCH_MIN_Y,yPrevVal);
                yVal[i] = touch_adc(2);
                yOut[i] = butterFilt(yVal, yOut, i, N);
            }
//            Ypos = median(yVal,N);
            Ypos = yOut[N-1];
            yPrevVal = Ypos;
            touch_select_dim(X_DIM);
            select = X_DIM;
        }
    }
}

static void AppMotorTask(void *p_arg) {
    (void)p_arg;

    uint32_t tick = 0;
    double prevErrX = 0;
    double prevErrY = 0;
    double errorX = 0;
    double errorY = 0;
    double derivativeX = 0;
    double derivativeY = 0;
    double integralX = 0;
    double integralY = 0;

    double dt = 0.01; // second
    double KpX = 0.190, KiX = 0.00 , KdX = 1.45;
    double KpY = 0.190, KiY = 0.00 , KdY = 1.45;

    while (DEF_TRUE){
        OSTimeDlyHMSM(0, 0, 0, 50);

        Xpos_set = CENTER_X + RADIUS * cos(tick * SPEED);
        Ypos_set = CENTER_Y + RADIUS * sin(tick * SPEED);
        tick++;

        errorX = (int)Xpos_set - (int)Xpos;
        integralX = integralX + errorX*dt;
        derivativeX = (errorX - prevErrX);
        pidX = KpX*errorX + KiX*integralX + KdX*derivativeX;
        prevErrX = errorX;

        errorY = (int)Ypos_set - (int)Ypos;
        integralY = integralY + errorY*dt;
        derivativeY = (errorY - prevErrY);
        pidY = KpY*errorY + KiY*integralY + KdY*derivativeY;
        prevErrY = errorY;

        duty_us_x = cap((pidX + 1500), MAX_DUTY_MICROSEC, MIN_DUTY_MICROSEC);
        duty_us_y = cap((pidY + 1500), MAX_DUTY_MICROSEC, MIN_DUTY_MICROSEC);
        motor_set_duty(X_DIM, duty_us_x);
        motor_set_duty(Y_DIM, duty_us_y+150);
    }
}

uint16_t median(uint16_t arrX[], uint16_t N)
{
    uint16_t min=65536;
    uint16_t minIndex=0;
    int i =0;
            for(i=0; i<N; i++)
            {
                if(arrX[i]<min)
                {
                    min=arrX[i];
                    minIndex=i;
                }
            }

            uint16_t swap = arrX[0];
            arrX[0] = arrX[minIndex];
            arrX[minIndex] = swap;

            min = 10000;

            for(i=1; i<N; i++)
            {
                if(arrX[i]<min)
                {
                    min=arrX[i];
                    minIndex=i;
                }
            }

            swap = arrX[1];
            arrX[1] = arrX[minIndex];
            arrX[minIndex] = swap;

            for(i=2; i<N; i++)
            {
                if(arrX[i]<min)
                {
                    min=arrX[i];
                    minIndex=i;
                }
            }

            return arrX[minIndex];
}

uint16_t smooth(uint16_t in, uint16_t up, uint16_t low, uint16_t prev){
//    return in;
    if (in > up){
            return prev;
    } else if (in < low){
            return prev;
    } else{
            return in;
    }
}

uint16_t butterFilt(uint16_t* in, uint16_t* out, uint16_t n, uint16_t N){
    uint16_t i = 0;
    double inD[N];
    double outD[N];
    double B[] = {0.0940, 0.3759, 0.5639, 0.3759, 0.0940};
    double A[] = {1.0000, 0.0000,0.4860, 0.0000,0.0177};

    for (i = 0; i < N; i++){
        inD[i] = (double)in[i];
        outD[i] = (double)out[i];
    }

    double acc = 0.0;
    for (i = 0; i < N; i++){
        if (n-1 < 0){
            acc += B[i]/A[0]*inD[n-i+N];
        }else if (n-1 >= N){
            acc += B[i]/A[0]*inD[n-i-N];
        }else{
            acc += B[i]/A[0]*inD[n-i];
        }
    }
    for (i = 1; i < N; i++){
        if (n-1 < 0){
            acc -= A[i]/A[0]*outD[n-i+N];
        }else if (n-1 >= N){
            acc -= A[i]/A[0]*outD[n-i-N];
        }else{
            acc -= A[i]/A[0]*outD[n-i];
        }
    }
    return (uint16_t)acc;
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
//static void LedTimer()
//{
//    OSSemPost(semLED);
//}