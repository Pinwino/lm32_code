#include "stdlib.h"
#include "hw/irq_timer.h"
#include "timer.h"
#include <linux/jiffies.h>
#include <hw/tics.h>

extern unsigned char * BASE_TIMER;

int setup_timer(struct timer_list *timer, void (*function)(unsigned long), unsigned long data){
	
	timer->function = function;	
	timer->data=data;

	return 0;
}

int mod_timer(struct timer_list *timer, unsigned long expires){
	uint32_t *dir;

	timer->expires=expires;
	
	irq_timer_writel(&timer->itmr, 0x0, TIMER_SEL);
	if(expires != timer->itmr.timer_dead_line || !irq_timer_check_armed(&timer->itmr)){
		timer->itmr.timer_dead_line = 0xabcdfe6b280;
		irq_timer_set_time(&timer->itmr, timer->itmr.timer_dead_line);
	}
	if(!irq_timer_check_armed(&timer->itmr)){
		irq_timer_sel_cascade(&timer->itmr, cascade_disable);
		irq_timer_time_mode(&timer->itmr, diff_time_periodic);
		irq_timer_arm(&timer->itmr, timer_arm);
	}
	
	return 0;
}
