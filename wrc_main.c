#include <stdio.h>
#include <inttypes.h>

#include <stdarg.h>

#include "syscon.h"
#include "uart.h"
#include "endpoint.h"
#include "minic.h"
#include "pps_gen.h"
#include "ptpd.h"
#include "ptpd_netif.h"
#include "i2c.h"
//#include "eeprom.h"
#include "onewire.h"
#include "softpll_ng.h"

#include "wrc_ptp.h"

///////////////////////////////////
//Calibration data (from EEPROM if available)
#ifdef WRPC_MASTER
int32_t sfp_alpha = -73622176;  //default value if could not read EEPROM
#else
int32_t sfp_alpha = 73622176;  //default value if could not read EEPROM
#endif
int32_t sfp_deltaTx = 0;
int32_t sfp_deltaRx = 0;
uint32_t cal_phase_transition = 595; //7000;

void wrc_initialize()
{
  int ret, i;
  uint8_t mac_addr[6], ds18_id[8] = {0,0,0,0,0,0,0,0};
  char sfp_pn[17];
	
	uart_init();

	mprintf("WR Core: starting up...\n");

	timer_init(1);

#if 1
  //Generate MAC address
  ow_init();
  if( ds18x_read_serial(ds18_id) == 0 ) 
    TRACE_DEV("Found DS18xx sensor: %x:%x:%x:%x:%x:%x:%x:%x\n",
        ds18_id[7], ds18_id[6], ds18_id[5], ds18_id[4], 
        ds18_id[3], ds18_id[2], ds18_id[1], ds18_id[0]);
  else
    TRACE_DEV("DS18B20 not found\n");
#endif

  mac_addr[0] = 0x08;   //
  mac_addr[1] = 0x00;   // CERN OUI
  mac_addr[2] = 0x30;   //  
  mac_addr[3] = ds18_id[3]; // www.maxim-ic.com
  mac_addr[4] = ds18_id[2]; // APPLICATION NOTE 186  
  mac_addr[5] = ds18_id[1]; // Creating Global Identifiers Using 1-Wire® Devices

  TRACE_DEV("Local MAC address: %x:%x:%x:%x:%x:%x\n", mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);

  ep_init(mac_addr);
  ep_enable(1, 1);

  minic_init();
  pps_gen_init();
  wrc_ptp_init();
  
  /* Derive the IP from the MAC address (10.x.y.z) */
  /* This will be done with BOOTP in the near future */
  ipv4_init("wru1", 0x0A000000 | mac_addr[3] << 16 | mac_addr[4] << 8 | mac_addr[5]);
}

#define LINK_WENT_UP 1
#define LINK_WENT_DOWN 2
#define LINK_UP 3
#define LINK_DOWN 4

int wrc_check_link()
{
	static int prev_link_state = -1;
	int link_state = ep_link_up(NULL);
	int rv = 0;

  if(!prev_link_state && link_state)
  {
    TRACE_DEV("Link up.\n");
    gpio_out(GPIO_LED_LINK, 1);
    rv = LINK_WENT_UP;
  } 
  else if(prev_link_state && !link_state)
  {
    TRACE_DEV("Link down.\n");
    gpio_out(GPIO_LED_LINK, 0);
    rv = LINK_WENT_DOWN;
  }  else rv = (link_state ? LINK_UP : LINK_DOWN);
  prev_link_state = link_state;

  return rv;
}

int wrc_extra_debug = 0;
int wrc_gui_mode = 0;

void wrc_debug_printf(int subsys, const char *fmt, ...)
{
	va_list ap;
	
	if(wrc_gui_mode) return;
	
	va_start(ap, fmt);
	
	if(wrc_extra_debug || (!wrc_extra_debug && (subsys & TRACE_SERVO)))
	 	vprintf(fmt, ap);
	
	va_end(ap);
}

static int wrc_enable_tracking = 1;
static int ptp_enabled = 1;
int wrc_man_phase = 0;

static void ui_update()
{

		if(wrc_gui_mode)
		{
			wrc_mon_gui();
			if(uart_read_byte() == 27)
			{
				shell_init();
				wrc_gui_mode = 0;
			}	
		}
		else
			shell_interactive();

}

int main(void)
{
  wrc_extra_debug = 1;
  wrc_gui_mode = 0;

  wrc_initialize();

	shell_init();

	wrc_ptp_set_mode(WRC_MODE_SLAVE);
	wrc_ptp_start();

	for(;;)
	{
	
    int l_status = wrc_check_link();

    switch (l_status)
    {
      case LINK_UP:
        update_rx_queues();
        break;

      case LINK_WENT_DOWN:
        spll_init(SPLL_MODE_FREE_RUNNING_MASTER, 0, 1);

        break;
    }        

		ui_update();
		wrc_ptp_update();
    spll_update_aux_clocks();
    ipv4_poll();
  }
}

