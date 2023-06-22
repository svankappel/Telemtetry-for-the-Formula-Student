#include <stdio.h>


//! UDP Client port
#define UDP_CLIENT_PORT 1502
//! UDP client connection check interval in miliseconds
#define UDP_CLIENT_SLEEP_TIME_MS 100



/*! @struct sContext
 * @brief Data structure with the Wifi stationing context information
 * @typedef tContext
 * @brief Data type with the Wifi stationing context information
*/
typedef struct sContext{
	//! Pointer to the sell
	const struct shell *pShell;
	union {
		struct {
			//! Connected 
			uint8_t connected					    : 					      1;
			uint8_t connect_result				    : 					      1;
			uint8_t disconnect_requested		    : 					      1;
			uint8_t _unused						    : 					      5;
		};
		uint8_t all;
	};
}tContext;

//! Contains the contex information of the Wifi stationing
extern tContext context;