#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>


uint64_t div_u64_rem(uint64_t n, uint32_t base, uint32_t *remainder){
	uint64_t rem = n;
	uint64_t b = base;
	uint64_t res, d = 1;
	uint32_t high = rem >> 32;
	
	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (uint64_t) high << 32;
		rem -= (uint64_t) (high*base) << 32;
	}
	
	while ((int64_t)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}
	
	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		
		b >>= 1;
		d >>= 1;
	} while (d);
	
	*remainder = rem;
	
	//printf("n=%" PRIu64 " res=%" PRIu64 " base=%" PRIu32 " b=%" PRIu64 " high=%" PRIu32 " rem=%" PRIu64 " d=%" PRIu64 "\n", n, res, base, b, high, rem, d);
	
	return res;
}

void int2hex(uint32_t n){
	while (n) 
	{
		if (n & 1)
			printf("1");
		else
			printf("0");

			n >>= 1;
		}
	printf("\n");
}

int main(void){
	
	uint64_t dividend[] = {15, 650, 759, 102569882369, 965448635497712};
	uint64_t quat;
	uint32_t divisor = 4, rem, i;
	uint8_t val = 0x84;
	
	for(i=0; i<(sizeof(dividend)/sizeof(dividend[0])); i++){
		//printf("Divide %" PRIu64 " by %" PRIu32 "\n", dividend[i], divisor);
		//quat= div_u64_rem(dividend[i], divisor, &rem);
		//printf("Reaminder %" PRIu32 " quatotient %" PRIu64 "\n", rem, quat);
		printf("Divide %llu by %lu \n", dividend[i], divisor);
		quat= div_u64_rem(dividend[i], divisor, &rem);
		printf("Reaminder %llu quatotient %lu \n", rem, quat);
	}
	
	/*for (divisor=0; divisor<=8; divisor ++){
		printf("%" PRIu32 " en binario es: ", divisor);
		int2hex(divisor);
		printf("rol32(%" PRIu32 ") = %" PRIu32 ",4) y en binario es: ", divisor, rol32(divisor, 4));
		int2hex(divisor);
	}
	//printf("Val is 0x%x and swapped is " val, ())*/
	
	
	return 0;
}
