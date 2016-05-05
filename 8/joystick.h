#ifndef __JOYSTICK_H
#define __JOYSTICK_H

extern int PrevStat;
extern int state;
void joystick_init();
double joystick_adc(uint8_t dim);

#endif
