#ifndef __TICS_H__
#define __TICS_H__

#include <stdint.h>

#define jiffies get_tics()

extern unsigned char *BASE_TICS;

static inline uint32_t get_tics(){
	uint32_t *p =  (uint32_t *) BASE_TICS;
	return abs(*p);
};

#endif /* __TICS_H__ */
