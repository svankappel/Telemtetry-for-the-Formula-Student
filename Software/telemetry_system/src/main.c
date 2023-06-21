/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "Task/wifi_sta.h"

int main(void)
{
	Task_Wifi_Sta_Init();
	//Task_UDP_Client_Init();


	k_sleep( K_FOREVER );
	
}
