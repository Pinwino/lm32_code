#ifndef __LM32_DELAY_H
#define __LM32_DELAY_H

#include  <board-wrc.h>
#include "sys/types.h"
#include <linux/jiffies.h>

extern int usleep(useconds_t usec);

#define udelay(a) usleep(a)
#define ndelay(a) usleep(a/1000)
#define msleep(a) usleep(a*1000)


#endif /* __LM32_DELAY_H */
