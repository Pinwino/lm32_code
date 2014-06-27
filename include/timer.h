#ifndef __TIMER_H__
#define __TIMER_H__

#include "hw/irq_timer.h"

 struct timer_list {
	unsigned long expires;
	struct irq_timer itmr;
	void (*function)(unsigned long);
	unsigned long data;
};
 
int setup_timer(struct timer_list *timer, void (*function)(unsigned long), unsigned long data);
int mod_timer(struct timer_list *timer, unsigned long expires);
 
/*struct tvec_base {
	spinlock_t lock;
	struct timer_list *running_timer;
	unsigned long timer_jiffies;
	unsigned long next_timer;
	unsigned long active_timers;
	struct tvec_root tv1;
	struct tvec tv2;
	struct tvec tv3;
	struct tvec tv4;
	struct tvec tv5;
 } ____cacheline_aligned;*/
  
#endif __TIMER_H__
