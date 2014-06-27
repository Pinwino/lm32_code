#ifndef __IRQ_CTRL_H__
#define __IRQ_CTRL_H_

#define  RST		0			//wo
#define  STATUS		RST+4		//ro, 1 bit per queue  
#define  POP		STATUS+4	//wo, 1 bit per queue pop
  
//pages for queues
//ueue I is found at:  QUEUES + I *  N_QUEUE
#define  QUEUES		16
#define  N_QUEUE	16 
#define  OFFS_DATA	0				//ro wb data, msi message
#define  OFFS_ADDR	OFFS_DATA+4		//ro wb addr, msi adr low bits ID caller device
#define  OFFS_SEL	OFFS_ADDR+4		//ro wb sel

#define set_1 0x1

extern unsigned char *BASE_IRQ_CTRL;


static inline void irq_ctrl_pop (void) {
	
	uint32_t *p = (uint32_t *) (BASE_IRQ_CTRL + POP);
	*p =set_1;
	
};

#endif /* __IRQ_CTRL_H__ */
