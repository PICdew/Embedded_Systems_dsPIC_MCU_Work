#ifndef __FLEXSERIAL_H
#define __FLEXSERIAL_H
#include "types.h"
extern volatile int isCorrupt;
void uart2_init(uint16_t baud);
int uart2_putc(uint8_t c);
uint8_t uart2_getc();

#endif 
