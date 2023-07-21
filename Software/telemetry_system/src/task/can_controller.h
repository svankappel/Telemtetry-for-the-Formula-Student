#ifndef __CAN_CONTROLLER_H
#define __CAN_CONTROLLER_H

/*! CAN_Controller implements the CAN_Controller task
* @brief CAN_Controller read the CAN Bus and fill the sensorBuffer array 
*        
*/
void CAN_Controller(void);

/*! Task_CAN_Controller_Init implements the CAN_Controller task initialization
* @brief CAN_Controller thread initialization
*      
*/
void Task_CAN_Controller_Init( void );


#endif /*__CAN_CONTROLLER_H*/