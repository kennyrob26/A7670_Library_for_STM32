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
			A7670_MQTT_Process_response();
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

CMD_Status A7670_CMD_Creset()
{
	strcpy(at.at_command, "AT+CRESET");

	if(AT_sendCommand("PB DONE", 15000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;

}

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





