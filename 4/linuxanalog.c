#include <sys/io.h>
#include <inttypes.h>
#include "linuxanalog.h"
void das1602_initialize(){
	int SetParam = LDAEMCL | DACEN | HS0 | START;//DAC0R1
	outw(SetParam,BADR1+8);
	outw(0,BADR4+2);
	
	

}
void dac(uint16_t value){
	// ADBE not empty status
	//wait until FIFO buffer is not full LADFUL
	while(((inw(BADR1+0)) & (LADFUL)));
	//write value out to BADR4
	outw(value, BADR4+0);
}
