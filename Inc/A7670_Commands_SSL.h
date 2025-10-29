/*
 * A7670_Commands_SSL.h
 *
 *  Created on: Oct 19, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SSL_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SSL_H_

#include "A7670_At_Commands.h"


typedef enum
{
	SSL_VERSION_SSL_3_0 = 0,
	SSL_VERSION_TLS_1_0 = 1,
	SSL_VERSION_TLS_1_1 = 2,
	SSL_VERSION_TLS_1_2 = 3,
	SSL_VERSION_ALL     = 4		//4 is the default and recommended value
}SSL_Version;

typedef enum
{
	SSL_AUTH_DISABLE           = 0,
	SSL_AUTH_SERVER_ONLY	   = 1,
	SSL_AUTH_SERVER_AND_CLIENT = 2,
	SSL_AUTH_CLIENT_ONLY       = 3
}SSL_Auth;

CMD_Status A7670_SSLConfig(uint8_t context, SSL_Version version, SSL_Auth authmode, char *ca_name);
CMD_Status A7670_CMD_SSLConfigVersion(uint8_t ctx_index, SSL_Version ssl_version);
CMD_Status A7670_CMD_SSLConfigAuthMode(uint8_t ctx_index, SSL_Auth auth_mode);
CMD_Status A7670_CMD_SSLConfigCacert(uint8_t ctx_index, const char* ca_name);
CMD_Status A7670_CMD_SSLConfigSNI(uint8_t ctx_index, uint8_t enable_SNI);

#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SSL_H_ */
