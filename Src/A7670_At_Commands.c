/*
 * A7670_At_Commands.c
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#include "A7670_At_Commands.h"


/**
 * @brief Reset A7670 Module
 * 
 * This commad resets the module A7670 sending AT+CRESET
 * 
 * @return CMD_Status
 * @retval CMD_OK if reboot is successful
 * @retval CMD ERROR if the command is sent but we do not receive a module initialization response
 */

CMD_Status A7670_CMD_Creset()
{
	strcpy(at.at_command, "AT+CRESET");

	if(AT_sendCommand("PB DONE", 15000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;

}


CMD_Status A7670_CMD_SSLConfigVersion(uint8_t ctx_index, uint8_t ssl_version)
{
	sprintf(at.at_command, "AT+CSSLCFG=\"sslversion\",%d,%d", ctx_index, ssl_version);
	if(AT_sendCommand("OK", 500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_SSLConfigAuthMode(uint8_t ctx_index, uint8_t auth_mode)
{
	sprintf(at.at_command, "AT+CSSLCFG=\"authmode\",%d,%d", ctx_index, auth_mode);
	if(AT_sendCommand("OK", 500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_SSLConfigCacert(uint8_t ctx_index, const char* ca_name)
{
	sprintf(at.at_command, "AT+CSSLCFG=\"cacert\",%d,%s", ctx_index, ca_name);
	if(AT_sendCommand("OK", 500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_SSLConfigSNI(uint8_t ctx_index, uint8_t enable_SNI)
{
	sprintf(at.at_command, "AT+CSSLCFG=\"enableSNI\",%d,%d", ctx_index, enable_SNI);
	if(AT_sendCommand("OK", 500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

