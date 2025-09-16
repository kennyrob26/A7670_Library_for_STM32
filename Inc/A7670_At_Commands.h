/*
 * A7670_At_Commands.h
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_AT_COMMANDS_H_
#define INC_A7670_AT_COMMANDS_H_

#include "AT_decoder.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum
{
	CMD_ERROR = 0,
	CMD_OK    = 1
} CMD_Status;

typedef enum AT_ResponseType
{
    AT_RT_Echo_Command  = 0,
    AT_RT_Response      = 1,
    AT_RT_MQTT_Response = 2,
    AT_RT_NULL          = 3
}AT_ResponseType;



CMD_Status A7670_Pocess_Buffer();




#endif /* INC_A7670_AT_COMMANDS_H_ */
