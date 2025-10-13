/*
 * A7670_At_Commands.h
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_AT_COMMANDS_H_
#define INC_A7670_AT_COMMANDS_H_

#include <AT_Handler.h>
#include "A7670_Commands_MQTT.h"
#include "A7670_Commands_GNSS.h"
#include "A7670_Commands_SIM.h"
#include "A7670_MQTT_Publish.h"
#include <stdio.h>
#include <stdlib.h>


CMD_Status A7670_CMD_Creset();
CMD_Status A7670_Pocess_Buffer();

CMD_Status A7670_CMD_SSLConfigVersion(uint8_t ctx_index, uint8_t ssl_version);
CMD_Status A7670_CMD_SSLConfigAuthMode(uint8_t ctx_index, uint8_t auth_mode);
CMD_Status A7670_CMD_SSLConfigCacert(uint8_t ctx_index, const char* ca_name);
CMD_Status A7670_CMD_SSLConfigSNI(uint8_t ctx_index, uint8_t enable_SNI);

#endif /* INC_A7670_AT_COMMANDS_H_ */
