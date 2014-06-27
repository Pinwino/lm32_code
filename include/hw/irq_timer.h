#ifndef __IRQ_TIMER_H__
#define __IRQ_TIMER_H__

#include "stdint.h"

#define RST				0 					//0x00, wo, Reset Active Low
#define ARM_STAT		RST+4   			//0x04, ro, Shows armed timers,  (1 armed, 0 disarmed), 1 bit per timer
#define ARM_SET			ARM_STAT+4   		//0x08, wo, arm timers,          
#define ARM_CLR			ARM_SET+4   		//0x0C, wo, disarm timers,         
#define SRC_STAT		ARM_CLR+4   		//0x10, ro, shows timer sources, (1 diff time, 0 abs time), 1 bit per timer 
#define SRC_SET			SRC_STAT+4   		//0x14, wo, select diff time as source
#define SRC_CLR			SRC_SET+4   		//0x18, wo, select abs time as source    
#define D_MODE_STAT		SRC_CLR+4   		//0x1C, ro, shows diff time modes, (1 periodic, 0 1time), 1 bit per timer 
#define D_MODE_SET		D_MODE_STAT+4   	//0x20, wo, select periodic mode
#define D_MODE_CLR		D_MODE_SET+4   		//0x24, wo, select 1time mode
#define CSC_STAT		D_MODE_CLR+4   		//0x28, ro, shows cascaded start, (1 cascaded, 0 normal), 1 bit per timer 
#define CSC_SET			CSC_STAT+4   		//0x2C, wo, set cascaded start
#define CSC_CLR			CSC_SET+4   		//0x30, wo, select normal start    
#define DBG				CSC_CLR+4   		//0x34, wo, reset counters, 1 bit per timer

#define BASE_TIMERS		64              //0x40
#define TIMER_SEL		BASE_TIMERS+0   //0x40, rw, timer select. !!!CAUTION!!! content of all TM_... regs depends on this
#define TM_TIME_HI		TIMER_SEL+4   	//0x44, rw, deadline HI word
#define TM_TIME_LO		TM_TIME_HI+4   	//0x48, rw, deadline LO word
#define TM_MSG			TM_TIME_LO+4   	//0x4C, rw, MSI msg to be sent on MSI when deadline7D5D7F53 is hit
#define TM_DST_ADR		TM_MSG+4   		//0x50, rw, MSI adr to send the msg to when deadline is hit
#define CSC_SEL			TM_DST_ADR+4   	//0x54, rw, select comparator output for cascaded start

#define num_hw_timers		1

#define set_0				0x0
#define set_1				0x1

#define timer_arm 			0x1
#define timer_disarm 		0x0

#define abs_time			0x0
#define diff_time_periodic	0x2
#define diff_time_1time		0x3

#define cascade_disable		0x0
#define cascade_enable		0x1

struct irq_timer {
	unsigned char * timer_addr_base;
	
	//timer specific fields
	uint8_t timer_id_num;
	unsigned long timer_dead_line;
	char  msi_msg[4];
	uint32_t msi_addr;
	
	//timer generic fields
	uint8_t arm;
	uint8_t time_source;
	uint8_t timer_mode;
	uint8_t cascade;
};

static inline uint32_t irq_timer_readl(struct irq_timer *itmr, unsigned long reg){
	
	uint32_t *p = (uint32_t *) (itmr->timer_addr_base + reg);
	return *p;
};

static inline void irq_timer_writel(struct irq_timer *itmr, uint32_t v, unsigned long reg)
{
	 uint32_t *p = (uint32_t *) (itmr->timer_addr_base + reg);
	 *p = v;
};

static inline void irq_timer_reset (struct irq_timer *itmr){
	irq_timer_writel(itmr, 0x1, RST);
};

uint8_t irq_timer_check_armed (struct irq_timer *itmr);
int irq_timer_arm (struct irq_timer *, uint8_t option);
int irq_timer_time_mode (struct irq_timer  *itmr, uint8_t option);
int irq_timer_sel_cascade(struct irq_timer  *itmr, uint8_t option);
void irq_timer_set_time(struct irq_timer  *itmr, unsigned long expires);
//static inline void irq_timer_arm (struct irq_timer *itmr, int){}

#endif /* __IRQ_TIMER_H__ */
