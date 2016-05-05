#ifndef __FLEXTOUCH_H
#define __FLEXTOUCH_H

void touch_init();
void touch_select_dim(uint8_t dim);
uint16_t touch_adc(uint8_t dim);

#endif
