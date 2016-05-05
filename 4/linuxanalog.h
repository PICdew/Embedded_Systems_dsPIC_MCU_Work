#ifndef _LINUXANALOG_H
#define _LINUXANALOG_H

#define BADR0 0xD000
#define BADR1 0xD0A0
#define BADR2 0xD080
#define BADR3 0xD060
#define BADR4 0xD040

#define LDAEMCL 1
#define DACEN   2
#define START   4
#define DAPS0   8
#define DAPS1   16
#define HS0     32
#define HS1     64
#define DAC0R0  256
#define DAC0R1  512
#define DAC1R0  1024
#define DAC1R1  2048

#define ADNE    4096
#define LADFUL  8192

#define DA0   1
#define DA1   2
#define DA2   4
#define DA3   8
#define DA4   16
#define DA5   32
#define DA6   64
#define DA7   128
#define DA8   256
#define DA9   512
#define DA10  1024
#define DA11  2048

void das1602_initialize();
void dac(uint16_t value);

#endif
