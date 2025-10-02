/*
 * A7670_At_Commands.c
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#include "A7670_At_Commands.h"

AT_ResponseType response_type = AT_RT_NULL;

CMD_Status setResponseType();
CMD_Status processAtCommand();

CMD_Status A7670_Pocess_Buffer()
{

	setResponseType();

	switch (response_type)
	{
		case AT_RT_Echo_Command:
			processAtCommand();
			return CMD_OK;
		break;
		case AT_RT_MQTT_Response:
			A7670_MQTT_ResponseHandler();
		break;
		case AT_RT_Response:
			//at.response = at.response_buffer;
			return CMD_OK;
		break;
		default:
			break;
	}

    return AT_ERROR;
}

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

/**
 * @brief Checks the response type of the AT command
 * 
 * Compare known answers with received answers
 */
CMD_Status setResponseType()
{
    if(strncmp((char*)at.response_buffer, "AT", 2) == 0)
        response_type = AT_RT_Echo_Command;
    else if (strncmp((char*)at.response_buffer, "\r\n+CMQTTRXSTART", 15) == 0)
        response_type = AT_RT_MQTT_Response;
    else if(strncmp((char*)at.response_buffer, "\r\n", 2) == 0)
    	response_type = AT_RT_Response;

    return AT_OK;

}

/** 
 * @brief Handle AT Messagens
 * 
 * This function handles AT messages, where the buffer is divided into 3 parts:
 * echo     -> Returns the command sent
 * response -> This is the response to the command sent
 * OK       -> If OK, it indicates that the command was successful
 * 
 * And these will be stored in AT_INFO
 * 
 * @return CMD_Status
*/
CMD_Status processAtCommand()
{

    at.echo = strtok((char*)at.response_buffer, "\r\r\n");
    if(at.echo == NULL)
    {
        at.status = 0;
        return CMD_ERROR;
    }

    at.response = strtok(NULL, "\r\n");
    if(at.response != NULL)
    {
        if(!strcmp(at.response, "OK"))
        {
            at.OK = at.response;
            at.status = 1;
            return CMD_OK;
        }
        else if(!strcmp(at.response, "ERROR"))
        {
            at.OK = at.response;
            at.status = 0;
            return CMD_ERROR;
        }
        else
        {
            at.OK = strtok(NULL, "\r\n");
            at.status = 1;
            return CMD_OK;
        }
    }

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

