#include <zephyr/kernel.h>
#include <nrfx_clock.h>


#include "task/wifi_sta.h"
#include "task/udp_client.h"



int main(void)
{
	nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);			//set 128 MHZ clock

	Task_Wifi_Sta_Init();
	Task_Led_Init();
	//Task_UDP_Client_Init();

	k_sleep( K_FOREVER );
	return 0;
}
