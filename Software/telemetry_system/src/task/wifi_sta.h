#ifndef WIFI_STA_H
#define WIFI_STA_H

/*! Wifi_Sta implements the task WiFi Stationing.
* 
* @brief Wifi_Stationing makes the complete connection process.
*/
void Wifi_Sta( void );

/*! Task_Wifi_Sta_Init initializes the task Wifi Stationing.
*
* @brief Wifi Stationing initialization
*/
void Task_Wifi_Sta_Init( void );

//functions prototypes
void connection_handler(void);

#endif /* WIFI_STA_H */