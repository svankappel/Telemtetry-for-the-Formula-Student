#ifndef WIFI_STA_H
#define WIFI_STA_H

/*! Wifi_Stationing implements the task WiFi Stationing.
* 
* @brief Wifi_Sta makes the complete connection process, while 
*       printing by LOG commands the connection status.
*       This function is used on an independent thread.
*/
void Wifi_Sta( void );

/*! Task_Wifi_Sta_Init initializes the task Wifi Stationing.
*
* @brief Wifi Stationing initialization
*/
void Task_Wifi_Sta_Init( void );

#endif /* WIFI_STA_H */