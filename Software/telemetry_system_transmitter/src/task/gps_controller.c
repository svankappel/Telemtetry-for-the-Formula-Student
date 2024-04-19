/*! --------------------------------------------------------------------
 *	Telemetry System	-	@file gps_controller.c
 *----------------------------------------------------------------------
 * HES-SO Valais Wallis 
 * Systems Engineering
 * Infotronics
 * ---------------------------------------------------------------------
 * @author Sylvestre van Kappel
 * @date 02.08.2023
 * ---------------------------------------------------------------------
 * @brief GPS Controller task receives data from the GPS module on the
 * 		  uart port and fill the GPS buffer with the received data
 * ---------------------------------------------------------------------
 * Telemetry system for the Valais Wallis Racing Team.
 * This file contains code for the onboard device of the telemetry
 * system. The system receives the data from the sensors on the CAN bus 
 * and the data from the GPS on a UART port. An SD Card contains a 
 * configuration file with all the system parameters. The measurements 
 * are sent via Wi-Fi to a computer on the base station. The measurements 
 * are also saved in a CSV file on the SD card. 
 *--------------------------------------------------------------------*/

//includes
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(gps);
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/uart.h>
#include <stdlib.h>
#include <zephyr/toolchain.h>
#include <string.h>

//project file includes
#include "gps_controller.h"
#include "memory_management.h"
#include "config_read.h"


//! GPS thread priority level
#define GPS_CONTROLLER_STACK_SIZE 4096
//! GPS thread priority level
#define GPS_CONTROLLER_PRIORITY 2

//! GPS stack definition
K_THREAD_STACK_DEFINE(GPS_CONTROLLER_STACK, GPS_CONTROLLER_STACK_SIZE);
//! Variable to identify the GPS thread
static struct k_thread gpsControllerThread;

// uart node
#define UART_DEVICE_NODE_GPS DT_CHOSEN(zephyr_shell_uart_gps)

//max size of NMEA message
#define MSG_SIZE 85

/*! @brief gps buffer struct
    @param speed current gps speed
    @param coord current gps coords
    @param fix current gps fix status
    @param NameLiveCoord name of the coord field in the live transmission
    @param NameLogCoord name of the coord field in the logs
    @param LiveCoordEnable coords enabled in the live transmission
    @param NameLiveSpeed name of the speed field in the live transmission
    @param NameLogSpeed name of the speed field in the logs
    @param LiveSpeedEnable speed enabled in the live transmission
    @param NameLiveFix name of the fix status field in the live transmission
    @param NameLogFix name of the fix status field in the logs
    @param LiveFixEnable fix status enabled in the live transmission
*/
tGps gpsBuffer;
K_MUTEX_DEFINE(gpsBufferMutex);

// queue to store up to 10 messages (aligned to 4-byte boundary) 
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

//uart device declaration
static const struct device *const uart_dev_gps = DEVICE_DT_GET(UART_DEVICE_NODE_GPS);

// receive buffer used in UART ISR callback 
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

//serial callback function prototype
/*!
 * @brief Read characters from UART until line end is detected. Afterwards push the
 * 		  data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data);


//-----------------------------------------------------------------------------------------------------------------------
/*! GPS_Controller implements the GPS_Controller task
* @brief GPS_Controller read the UART form GPS and fill the sensorBuffer array 
*        
*/
void GPS_Controller(void)
{
	//gps fix
	bool gpsFix = false;
	
	//rx buffer
	char rx_buf[MSG_SIZE];

	//check if uart device is ready
	if (!device_is_ready(uart_dev_gps)) 
	{
		LOG_INF("UART device not found!");
		return;
	}
	// configure interrupt and callback to receive data 
	uart_irq_callback_user_data_set(uart_dev_gps, serial_cb, NULL);
	uart_irq_rx_enable(uart_dev_gps);

	// indefinitely wait for input from UART
	while (k_msgq_get(&uart_msgq, &rx_buf, K_FOREVER) == 0) 
	{

		//-----------------------------------------------------
		//		NMEA GSA Frame -	GPS receiver operating mod

		if(strstr(rx_buf,"$GNGSA"))
		{
			if(rx_buf[10]=='3')		//detect if gps has fix
				gpsFix=true;

			else
				gpsFix=false;

			k_mutex_lock(&gpsBufferMutex,K_FOREVER);		//lock gps buffer mutex
			gpsBuffer.fix = gpsFix;							//copy gps fix value to buffer
			k_mutex_unlock(&gpsBufferMutex);				//unlock mutex
		}

		//analyse other frames only if GPS is fixed
		if(gpsFix)	
		{
			//-----------------------------------------------------
			//	NMEA GLL Frame - contains latitude and longitude

			if(strstr(rx_buf,"$GNGLL"))
			{
				// NMEA frame DMm + sign
				char latDeg[4];
				char latMin[12];
				char latSign[2];
				char lonDeg[4];
				char lonMin[12];
				char lonSign[2];
				
				char * saveptr=NULL;			//strtok save pointer
				char * str = rx_buf;			//string of frame
				char * token = (char*)strtok_r(str,",",&saveptr);	// get first token

				int i = 0;		//loop index

				//loop for all token of NMEA frame
				while (token!=NULL) 
				{  
					token = (char*)strtok_r(NULL,",",&saveptr);	//get next token
					switch(i)
					{
						case 0: strncpy(latDeg,token,2);				// latitude degree angle
								latDeg[2] = '\0';
								strncpy(latMin,token+2,2);				// latitude minutes
								strcpy(latMin+2,token+5);				// latitude degrees of minutes
						break;
						case 1: strcpy(latSign,*token=='N' ? "" : "-");	// latitude sign (north or south)
						break;
						case 2: strncpy(lonDeg,token,3);				// longitude degree angle
								lonDeg[3]='\0';
								strncpy(lonMin,token+3,2);				// longitude minutes
								strcpy(lonMin+2,token+6);				// longitude degrees of minutes
						break;
						case 3: strcpy(lonSign,*token=='E' ? "" : "-");	// longitude sign (west or east)
						break;
					}
					if(i<3)				//break after 5 loops (other data not intersting)
						i++;	
					else
						break;
				}
				
				//calculate decimal coords from NMEA DM.m
				uint32_t lat = atoi(latDeg);
				uint32_t lon = atoi(lonDeg);
				uint32_t lat_dec = atoi(latMin)/6;
				uint32_t lon_dec = atoi(lonMin)/6;

				//generate decimal coords string
				char coord[25];
				sprintf(coord,"%s%d.%d %s%d.%d",latSign,lat,lat_dec,lonSign,lon,lon_dec);

				k_mutex_lock(&gpsBufferMutex,K_FOREVER);		//lock gps buffer mutex
				strcpy(gpsBuffer.coord,coord);					//copy coords to gps buffer
				k_mutex_unlock(&gpsBufferMutex);				//unlock mutex
			}

			//-----------------------------------------------------
			//	NMEA RMC Frame - contains latitude and longitude

			if(strstr(rx_buf,"$GNRMC"))
			{
				bool frameValid = false;

				// NMEA frame DMm + sign
				char latDeg[4];
				char latMin[12];
				char latSign[2];
				char lonDeg[4];
				char lonMin[12];
				char lonSign[2];
				
				char * saveptr=NULL;			//strtok save pointer
				char * str = rx_buf;			//string of frame
				char * token = (char*)strtok_r(str,",",&saveptr);	// get first token

				int i = 0;		//loop index

				//loop for all token of NMEA frame
				while (token!=NULL) 
				{  
					token = (char*)strtok_r(NULL,",",&saveptr);	//get next token
					switch(i)
					{
						case 1: frameValid = (*token=='A');				//NMEA RMC valid 
						break;
						case 2: strncpy(latDeg,token,2);				// latitude degree angle
								latDeg[2] = '\0';
								strncpy(latMin,token+2,2);				// latitude minutes
								strcpy(latMin+2,token+5);				// latitude degrees of minutes
						break;
						case 3: strcpy(latSign,*token=='N' ? "" : "-");	// latitude sign (north or south)
						break;
						case 4: strncpy(lonDeg,token,3);				// longitude degree angle
								lonDeg[3]='\0';
								strncpy(lonMin,token+3,2);				// longitude minutes
								strcpy(lonMin+2,token+6);				// longitude degrees of minutes
						break;
						case 5: strcpy(lonSign,*token=='E' ? "" : "-");	// longitude sign (west or east)
						break;
					}
					if(i<5)				//break after 5 loops (other data not intersting)
						i++;	
					else
						break;
				}
				if(frameValid)			//if frame is valid
				{
					//calculate decimal coords from NMEA DM.m
					uint32_t lat = atoi(latDeg);
					uint32_t lon = atoi(lonDeg);
					uint32_t lat_dec = atoi(latMin)/6;
					uint32_t lon_dec = atoi(lonMin)/6;

					//generate decimal coords string
					char coord[25];
					sprintf(coord,"%s%d.%d %s%d.%d",latSign,lat,lat_dec,lonSign,lon,lon_dec);

					
					k_mutex_lock(&gpsBufferMutex,K_FOREVER);		//lock gps buffer mutex
					strcpy(gpsBuffer.coord,coord);					//copy coords to gps buffer
					k_mutex_unlock(&gpsBufferMutex);				//unlock mutex
				}
			}

			//-----------------------------------------------------
			//	NMEA GGA Frame - contains latitude and longitude

			if(strstr(rx_buf,"$GNGGA"))
			{
				// NMEA frame DMm + sign
				char latDeg[4];
				char latMin[12];
				char latSign[2];
				char lonDeg[4];
				char lonMin[12];
				char lonSign[2];
				
				char * saveptr=NULL;			//strtok save pointer
				char * str = rx_buf;			//string of frame
				char * token = (char*)strtok_r(str,",",&saveptr);	// get first token

				int i = 0;		//loop index

				//loop for all token of NMEA frame
				while (token!=NULL) 
				{  
					token = (char*)strtok_r(NULL,",",&saveptr);	//get next token
					switch(i)
					{
						case 1: strncpy(latDeg,token,2);				// latitude degree angle
								latDeg[2] = '\0';
								strncpy(latMin,token+2,2);				// latitude minutes
								strcpy(latMin+2,token+5);				// latitude degrees of minutes
						break;
						case 2: strcpy(latSign,*token=='N' ? "" : "-");	// latitude sign (north or south)
						break;
						case 3: strncpy(lonDeg,token,3);				// longitude degree angle
								lonDeg[3]='\0';
								strncpy(lonMin,token+3,2);				// longitude minutes
								strcpy(lonMin+2,token+6);				// longitude degrees of minutes
						break;
						case 4: strcpy(lonSign,*token=='E' ? "" : "-");	// longitude sign (west or east)
						break;
					}
					if(i<4)				//break after 5 loops (other data not intersting)
						i++;	
					else
						break;
				}
				
				//calculate decimal coords from NMEA DM.m
				uint32_t lat = atoi(latDeg);
				uint32_t lon = atoi(lonDeg);
				uint32_t lat_dec = atoi(latMin)/6;
				uint32_t lon_dec = atoi(lonMin)/6;

				//generate decimal coords string
				char coord[25];
				sprintf(coord,"%s%d.%d %s%d.%d",latSign,lat,lat_dec,lonSign,lon,lon_dec);

				k_mutex_lock(&gpsBufferMutex,K_FOREVER);		//lock gps buffer mutex
				strcpy(gpsBuffer.coord,coord);					//copy coords to gps buffer
				k_mutex_unlock(&gpsBufferMutex);				//unlock mutex
			}

			//-----------------------------------------------------
			//	NMEA VTG Frame - contains speed

			if(strstr(rx_buf,"$GNVTG"))
			{
				char *str = rx_buf;		//string buffer
				char *saveptr = NULL;	//save pointer for strtok_r

				char * token;			//token of data frame
				char lastToken[12];		//variable to stock last token

				token = (char*)strtok_r(str,",",&saveptr);			//get first token

				while (token != NULL) 		//loop for all tokens
				{
					strcpy(lastToken, token);				//save token for next cycle
					token = (char*)strtok_r(NULL, ",",&saveptr);	//get next token
					if(token[0]=='K')		//if current token is 'K'				
						break;				//speed is last token -> break
				}

				k_mutex_lock(&gpsBufferMutex,K_FOREVER);		//lock gps buffer mutex
				strcpy(gpsBuffer.speed,lastToken);				//read speed in frame
				k_mutex_unlock(&gpsBufferMutex);				//unlock mutex
			}
		}
	}
	
}


//-----------------------------------------------------------------------------------------------------------------------
/*! Task_GPS_Controller_Init implements the GPS_Controller task initialization
* @brief GPS_Controller thread initialization
*      
*/
void Task_GPS_Controller_Init( void )
{
	k_thread_create	(&gpsControllerThread,
					GPS_CONTROLLER_STACK,										        
					GPS_CONTROLLER_STACK_SIZE,
					(k_thread_entry_t)GPS_Controller,
					NULL,
					NULL,
					NULL,
					GPS_CONTROLLER_PRIORITY,
					0,
					K_NO_WAIT);	

	 k_thread_name_set(&gpsControllerThread, "gpsController");
	 k_thread_start(&gpsControllerThread);
}




//-----------------------------------------------------------------------------------------------------------------------
/*!
 * @brief Read characters from UART until line end is detected. Afterwards push the
 * 		  data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart_dev_gps)) {
		return;
	}

	while (uart_irq_rx_ready(uart_dev_gps)) {

		uart_fifo_read(uart_dev_gps, &c, 1);

		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			// terminate string 
			rx_buf[rx_buf_pos] = '\0';

			// if queue is full, message is silently dropped 
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			// reset the buffer (it was copied to the msgq) 
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		// else: characters beyond buffer size are dropped 
	}
}