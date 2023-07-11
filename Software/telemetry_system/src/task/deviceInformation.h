#ifndef __DEVICEINFORMATION_H
#define __DEVICEINFORMATION_H

#include <stdio.h>

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
			uint8_t ip_assigned						:						  1;
			uint8_t _unused						    : 					      4;
		};
		uint8_t all;
	};
}tContext;

//! Contains the context information of the Wifi stationing
extern tContext context;


#endif /*__DEVICEINFORMATION_H*/