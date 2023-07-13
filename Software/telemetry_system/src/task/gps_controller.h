#ifndef __GPS_CONTROLLER_H
#define __GPS_CONTROLLER_H

/*! GPS_Controller implements the GPS_Controller task
* @brief GPS_Controller read the UART form GPS and fill the sensorBuffer array 
*        
*/
void GPS_Controller(void);

/*! Task_GPS_Controller_Init implements the GPS_Controller task initialization
* @brief GPS_Controller thread initialization
*      
*/
void Task_GPS_Controller_Init( void );


#endif /*__GPS_CONTROLLER_H*/