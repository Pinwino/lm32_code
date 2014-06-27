#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"

int main (void){

	uint32_t cnt = 0x4, oper, tmr = 0x1;
	uint8_t mask = 0x1;
	
	oper = cnt & mask ;
	printf("oper = %08x\n", oper);
	
	oper = cnt | (mask << 0x1);
	printf("oper = %08x\n", oper);

	return 0;
}
