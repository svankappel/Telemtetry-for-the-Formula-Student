#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sta, CONFIG_LOG_DEFAULT_LEVEL);

#include <nrfx_clock.h>
#include <zephyr/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/printk.h>
#include <zephyr/init.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <qspi_if.h>

#include "net_private.h"
#include "deviceinformation.h"
#include "wifi_sta.h"


#define SSID1 "VRT-Telemetry"
#define SSID2 "motog8"
#define PASSWORD1 "TJJC2233"
#define PASSWORD2 "TJJC2233"
#define REDUNDANCY true


//! Wifi thread priority level
#define WIFI_STACK_SIZE 4096
//! Wifi thread priority level
#define WIFI_PRIORITY 4

//! WiFi stack definition
K_THREAD_STACK_DEFINE(WIFI_STACK, WIFI_STACK_SIZE);
//! Variable to identify the Wifi thread
static struct k_thread wifiThread;

#define WIFI_SHELL_MODULE "wifi"

#define WIFI_SHELL_MGMT_EVENTS (NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)

#define MAX_SSID_LEN        32
#define DHCP_TIMEOUT        70
#define CONNECTION_TIMEOUT  30
#define STATUS_POLLING_MS   300


//static functions prototypes
static int wifi_connect(const char* ssid, char* pass);
static int __wifi_args_to_params(struct wifi_connect_req_params *params, const char* ssid, char* pass);
static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface);
static void print_dhcp_ip(struct net_mgmt_event_callback *cb);
static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface);
static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb);
static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb);
static int cmd_wifi_status(void);


//callback struct
static struct net_mgmt_event_callback wifi_shell_mgmt_cb;
static struct net_mgmt_event_callback net_shell_mgmt_cb;


//context struct from deviceInformation.h
tContext context;

//-----------------------------------------------------------------------------------------------------------------------
/*! Task_Wifi_Stationing_Init initializes the task Wifi Stationing.
*
* @brief 
*/
void Task_Wifi_Sta_Init( void ){
    
	k_thread_create	(&wifiThread,
					WIFI_STACK,										        
					WIFI_STACK_SIZE,
					(k_thread_entry_t)Wifi_Sta,
					NULL,
					NULL,
					NULL,
					WIFI_PRIORITY,
					0,
					K_NO_WAIT);	

	 k_thread_name_set(&wifiThread, "wifiStationing");
	 k_thread_start(&wifiThread);
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Wifi_Sta implements the task WiFi Stationing.
* 
* @brief Wifi_Stationing makes the complete connection process.
*/
void Wifi_Sta( void )
{
	//initialize context struct
	memset(&context, 0, sizeof(context));

	//add callback for wifi event (connect and disconnect)
	net_mgmt_init_event_callback(&wifi_shell_mgmt_cb, wifi_mgmt_event_handler, WIFI_SHELL_MGMT_EVENTS);
	net_mgmt_add_event_callback(&wifi_shell_mgmt_cb);

	//add event for net event (DHCP)
	net_mgmt_init_event_callback(&net_shell_mgmt_cb, net_mgmt_event_handler, NET_EVENT_IPV4_DHCP_BOUND);
	net_mgmt_add_event_callback(&net_shell_mgmt_cb);

	//wait some time before starting the thread
	k_sleep(K_SECONDS(1));

	while (1) 		//--------------   thread infinite loop
    {
		wifi_connect(SSID1,PASSWORD1);		//try to connect to the first ssid
		connection_handler();				//bloc the thread if connection succeed
		wifi_connect(SSID2,PASSWORD2);		//try to connect to the first ssid
		connection_handler();				//bloc the thread if connection succeed
		
	}				//--------------	en of thread infinite loop
}


//------------------------------------------------------------------------------------------------
//		wifi connect function -> sends a connect request to wifi driver

static int wifi_connect(const char* ssid, char* pass)
{
	//wifi params
	struct net_if *iface = net_if_get_default();
	static struct wifi_connect_req_params cnx_params;

	//set context to disconnected state
	context.connected = false;
	context.ip_assigned = false;
	context.connect_result = false;

	//set ssid and password
	__wifi_args_to_params(&cnx_params,ssid,pass);

	//request connection
	if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &cnx_params, sizeof(struct wifi_connect_req_params))) 
	{
		LOG_ERR("Connection request failed");
		return -ENOEXEC;
	}

	LOG_INF("Connection requested");

	return 0;
}

//------------------------------------------------------------------------------------------------
//		Connection handler (called after wifi_connect)

void connection_handler(void)
{
	for (int i = 0; i < CONNECTION_TIMEOUT; i++) 		//wait some time 
	{
		k_sleep(K_MSEC(STATUS_POLLING_MS));
		cmd_wifi_status();

		if (context.connect_result) 				//break loop if connection succeed
			break;
	}
	while(context.connected) 				//wait while conncetion is up
	{
		k_msleep(100);
	} 
	if (!context.connect_result) 				//log message if connection is down
	{
		LOG_INF("Connection Timed Out");
	}
}




//------------------------------------------------------------------------------------------------
//		wifi events callback (connect and disconnect)

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface)
{
	switch (mgmt_event) 
	{
	case NET_EVENT_WIFI_CONNECT_RESULT:			//connect event
		handle_wifi_connect_result(cb);			//call function
		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:		//disconnect event
		handle_wifi_disconnect_result(cb);		//call function
		break;
	default:
		break;
	}
}
//------------------------------------------------------------------------------------------------
//		wifi connect event function

static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb)
{
	//get status
	const struct wifi_status *status =(const struct wifi_status *) cb->info;

	//return if context already connected
	if (context.connected) 
		return;

	if (status->status) 		//log error if connection failed
		LOG_ERR("Connection failed (%d)", status->status);	
	else 
	{
		//set context if connection succeed
		LOG_INF("Connected");
		context.connected = true;
	}

	context.connect_result = true;
}
//------------------------------------------------------------------------------------------------
//		wifi disconnect event function

static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb)
{
	//get status
	const struct wifi_status *status = (const struct wifi_status *) cb->info;

	//return if context already disconnected
	if (!context.connected) 
		return;

	//log disconnection result
	if (context.disconnect_requested) 
	{
		LOG_INF("Disconnection request %s (%d)", status->status ? "failed" : "done", status->status);
		context.disconnect_requested = false;
	} 
	else //disconnected
	{
		LOG_INF("Received Disconnected");
		memset(&context, 0, sizeof(context));		//reset context
	}

}


//------------------------------------------------------------------------------------------------
//		net event callback (dhcp)

static void net_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface)
{
	switch (mgmt_event) 
	{
	case NET_EVENT_IPV4_DHCP_BOUND:		//ip address assigned
		print_dhcp_ip(cb);				//print informations
		context.ip_assigned=true;		//set context
		break;
	default:
		break;
	}
}
//------------------------------------------------------------------------------------------------
//		print dhcp information function

static void print_dhcp_ip(struct net_mgmt_event_callback *cb)
{
	/* Get DHCP info from struct net_if_dhcpv4 and print */
	const struct net_if_dhcpv4 *dhcpv4 = cb->info;
	const struct in_addr *addr = &dhcpv4->requested_ip;
	char dhcp_info[128];

	net_addr_ntop(AF_INET, addr, dhcp_info, sizeof(dhcp_info));

	LOG_INF("DHCP IP address: %s", dhcp_info);
}



//------------------------------------------------------------------------------------------------
//		wifi args to params function -> set wifi param struct with ssid and password

static int __wifi_args_to_params(struct wifi_connect_req_params *params, const char* ssid, char* pass)
{
	// no timeout
	params->timeout = SYS_FOREVER_MS;		

	/* SSID */
	params->ssid=ssid;
	params->ssid_length = strlen(params->ssid);

	//set security according to configuration file
#if defined(CONFIG_STA_KEY_MGMT_WPA2)
	params->security = 1;
#elif defined(CONFIG_STA_KEY_MGMT_WPA2_256)
	params->security = 2;
#elif defined(CONFIG_STA_KEY_MGMT_WPA3)
	params->security = 3;
#else
	params->security = 0;
#endif

	//set password
#if !defined(CONFIG_STA_KEY_MGMT_NONE)
	params->psk = pass;
	params->psk_length = strlen(params->psk);
#endif

	//set channel 
	params->channel = WIFI_CHANNEL_ANY;

	/* MFP (optional) */
	params->mfp = WIFI_MFP_OPTIONAL;

	return 0;
}

//------------------------------------------------------------------------------------------------
//		wifi log informations function

static int cmd_wifi_status(void)
{
	struct net_if *iface = net_if_get_default();
	struct wifi_iface_status status = { 0 };

	if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status,
				sizeof(struct wifi_iface_status))) {
		LOG_INF("Status request failed");

		return -ENOEXEC;
	}

	LOG_INF("==================");
	LOG_INF("State: %s", wifi_state_txt(status.state));

	if (status.state >= WIFI_STATE_ASSOCIATED) {
		uint8_t mac_string_buf[sizeof("xx:xx:xx:xx:xx:xx")];

		LOG_INF("Interface Mode: %s",
		       wifi_mode_txt(status.iface_mode));
		LOG_INF("Link Mode: %s",
		       wifi_link_mode_txt(status.link_mode));
		LOG_INF("SSID: %-32s", status.ssid);
		LOG_INF("BSSID: %s",
		       net_sprint_ll_addr_buf(
				status.bssid, WIFI_MAC_ADDR_LEN,
				mac_string_buf, sizeof(mac_string_buf)));
		LOG_INF("Band: %s", wifi_band_txt(status.band));
		LOG_INF("Channel: %d", status.channel);
		LOG_INF("Security: %s", wifi_security_txt(status.security));
		LOG_INF("MFP: %s", wifi_mfp_txt(status.mfp));
		LOG_INF("RSSI: %d", status.rssi);
	}
	return 0;
}