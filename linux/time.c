#include <linux/math64.h>
//extern static unsigned long usleep_lpj;

unsigned long msecs_to_jiffies(const unsigned int m){
	return m*20*1000;
}

unsigned int jiffies_to_msecs(const unsigned long j){
	return  div_u64_rem(j, 20*1000, NULL);
}
