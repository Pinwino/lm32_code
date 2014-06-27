#include <stdio.h>
#include <stdlib.h>

#include "access_internals.h"

int main (void)
{
	
	access_caloe access;
	network_connection nc;
	int is_config_int=0;
	int rcode = ALL_OK;
	int len;
	FILE *fp;                                                                                                        
	int nbytes = 100, line_end=0;
	uint32_t cntr = 0;
	char *line = (char *) malloc (nbytes + 1);                                                                      
	
	const char tok[] = " ";
   	char *ret;

	char aux[]="udp/10.10.1.1";
	//const char f[]="/home/pepe/wr/fd_clean/hdl/syn/spec/uart.ram";
	const char f[]="/home/pepe/lm32_code/uart.ram";
	char * addr;
	char * val;
	uint64_t _val, _addr;
	unsigned long length, line_length=0;

	// Copy IP address to an aux string
	//strcpy(aux,networkc.getIP().c_str());

	// Build an network_connection struct of access_internals
	//build_network_con_full_caloe(aux,networkc.getPort(),&nc);
	build_network_con_caloe (aux, &nc);

	// Parsing boolean value to integer
	/*if(is_config) {
		is_config_int=1;
	}
	else {
		is_config_int = 0;
	}*/
	
	fp = fopen(f, "r");
	fseek(fp, 0, SEEK_END);
	length = ftell(fp) - 3*17;
	//printf("L = %lu\n", length);
	fseek(fp, 0, SEEK_SET);
	
  	while(length > ftell(fp)){
		//printf("Processing line %x\n", cntr+1);
		//printf("ftell(fp) = %lu, length %lu\n", ftell(fp), length);
		if (fgets(line, nbytes, fp)) {
   			//printf("char: %s\n", line);  //try and get the first char from input
			ret=strpbrk(line, "\n");
			*ret=0;
			//printf("char: %s\n", line);
			addr = strpbrk(line, tok);
			*addr = 0;
			addr++;
			//printf("%s\n", addr);
			val = strpbrk(addr, tok);
			*val=0;
			val++;
			//printf("%s\n", val);
			_addr = (uint64_t)strtol(addr, NULL, 16)*4;
			_val = (uint64_t)strtol(val, NULL, 16);
		}
		cntr++;
		//printf("Reading %08x\n", _addr);
		build_access_caloe(0x40000, _addr, _val, 0, MASK_OR, is_config_int, WRITE, SIZE_4B, &nc, &access);
		rcode = execute_caloe(&access);
		//printf("Readed %08x\n", (uint64_t) access.value);
		//printf("Readed %08x\n", (uint64_t) _val);
		//printf("Diff %ld\n", (uint64_t) _val - (uint64_t) access.value);
		if ((uint64_t) access.value != (uint64_t) _val)	{
			printf("ERROR\n");
			printf("addr %s val %s\n", addr, val);
			printf("addr %08X val %08X\n", _addr, _val);
			printf("Diff %ld\n", (uint64_t) _val - (uint64_t) access.value);
			break;
		}
		//if (cntr > 6){}
			//break;
	}
	
	printf("OK!\n");
	printf("addr %s val %s\n", addr, val);
	printf("addr %08X val %08X\n", _addr, _val);

	fclose(fp);
	// Build an access_caloe struct of access_internals*/
	build_access_caloe(0x40000, _addr, _val, 0, MASK_OR, is_config_int, SCAN, SIZE_4B, &nc, &access);
		
	// Execute the access_caloe struct
	rcode = execute_caloe(&access);

	// If access type is READ, store read value
	/*if(mode == READ) {
		value = access.value;
	}*/

	// Free access_caloe and network_connection memory
	free_access_caloe(&access);
	
	/*// Parsing alignment to integer
	int align_v;
	
	switch(align) {
		case SIZE_1B: align_v = 1;
					  break;
		case SIZE_2B: align_v = 2;
					  break;
		case SIZE_4B: align_v = 4;
					  break;
		case SIZE_8B: align_v = 8;
					  break;
	};
	
	// Update address with autoincr value 
	address += (autoincr*align_v);*/

	return rcode;
}
