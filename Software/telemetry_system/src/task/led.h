#ifndef __LED_H
#define __LED_H

/*! Led implements the Led task
* @brief Led blink led1 when wifi is connected,
*        blink led2 when data logging is active
*		 and set led2 on when config file is failed
*/
void Led(void);

/*! Task_Led_Init implements the Led task initialization
* @brief Led thread initialization
*/
void Task_Led_Init( void );

#endif /*__LED_H*/