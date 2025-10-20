/*
 * A7670_Commands_SSL.h
 *
 *  Created on: Oct 19, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SSL_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SSL_H_

#include "A7670_At_Commands.h"

CMD_Status A7670_SSLConfig(uint8_t context, uint8_t version, uint8_t authmode, char *ca_name);
CMD_Status A7670_CMD_SSLConfigVersion(uint8_t ctx_index, uint8_t ssl_version);
CMD_Status A7670_CMD_SSLConfigAuthMode(uint8_t ctx_index, uint8_t auth_mode);
CMD_Status A7670_CMD_SSLConfigCacert(uint8_t ctx_index, const char* ca_name);
CMD_Status A7670_CMD_SSLConfigSNI(uint8_t ctx_index, uint8_t enable_SNI);

#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SSL_H_ */
