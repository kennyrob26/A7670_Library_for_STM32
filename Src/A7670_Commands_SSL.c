/*
 * A7670_Commands_SSL.c
 *
 *  Created on: Oct 19, 2025
 *      Author: kenny
 */

#include "A7670_Commands_SSL.h"
#include "A7670_Commands_MQTT.h"


CMD_Status A7670_SSLConfig(uint8_t context, SSL_Version version, SSL_Auth authmode, char *ca_name)
{
	A7670_CMD_SSLConfigVersion(context, version);
	A7670_CMD_SSLConfigAuthMode(context, authmode);
	A7670_CMD_SSLConfigCacert(context, ca_name);
	A7670_CMD_SSLConfigSNI(context, 1);

	A7670_MQTT_SetSSL(MQTT_SSL_ENABLE);					//OBS: ainda preciso configurar uma maneira de habilitar o ssl
	return CMD_OK;
}

CMD_Status A7670_CMD_SSLConfigVersion(uint8_t ctx_index, SSL_Version ssl_version)
{
	char command[30];
	sprintf(command, "AT+CSSLCFG=\"sslversion\",%d,%d", ctx_index, ssl_version);
	if(AT_sendCommand(command, "OK", 2000) == AT_OK)
	{
		return CMD_OK;
	}
	else
		return CMD_ERROR;
}

CMD_Status A7670_CMD_SSLConfigAuthMode(uint8_t ctx_index, SSL_Auth auth_mode)
{
	char command[30];
	sprintf(command, "AT+CSSLCFG=\"authmode\",%d,%d", ctx_index, auth_mode);
	if(AT_sendCommand(command, "OK", 2000) == AT_OK)
	{
		return CMD_OK;
	}
	else
		return CMD_ERROR;
}

CMD_Status A7670_CMD_SSLConfigCacert(uint8_t ctx_index, const char* ca_name)
{
	char command[50] = "AT+CSSLCFG=\"cacert\",0,\"isrgrootx1.pem\"";
	char expected_response[50] = "AT+CSSLCFG=\"cacert\",0,\"isrgrootx1.pem\"";
	memset(at.response, 0, 150);
	//sprintf(command, "AT+CSSLCFG=\"cacert\",%d,\"%s\"", ctx_index, ca_name);
	if(AT_sendCommand(command, "OK", 5000) == AT_OK)
	{
		return CMD_OK;
	}
	else
		return CMD_ERROR;
}

CMD_Status A7670_CMD_SSLConfigSNI(uint8_t ctx_index, uint8_t enable_SNI)
{
	char command[35];
	sprintf(command, "AT+CSSLCFG=\"enableSNI\",%d,%d", ctx_index, enable_SNI);
	if(AT_sendCommand(command, "OK", 500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}
