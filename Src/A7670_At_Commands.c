/*
 * A7670_At_Commands.c
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#include "A7670_At_Commands.h"



CMD_Status processAtCommand();
CMD_Status mqtt_response();

MQTT_RESPONSE mqtt_resp;


CMD_Status A7670_Pocess_Buffer()
{

    AT_ResponseType response_type = AT_RT_NULL;

    if(strncmp((char*)at.response_buffer, "AT", 2) == 0)
        response_type = AT_RT_Echo_Command;
    else if (strncmp((char*)at.response_buffer, "\r\n+CMQTTRXSTART", 15) == 0)
        response_type = AT_RT_MQTT_Response;
    else if(strncmp((char*)at.response_buffer, "\r\n", 2) == 0)
    	response_type = AT_RT_Response;


	switch (response_type)
	{
	case AT_RT_Echo_Command:
		processAtCommand();
		return CMD_OK;
	break;
	case AT_RT_MQTT_Response:
		AT_config_Wait_Response("+CMQTTRXEND: 0", 50);				//waiting for "END" of message
		if(AT_check_Wait_Response_Blocking() == AT_OK)
		{
			strcpy(mqtt_resp.last_message, (const char*)at.response_buffer);
			mqtt_response();
			return CMD_OK;
		}
		else
			return CMD_ERROR;
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

CMD_Status mqtt_response()
{
    uint8_t index = 0;
    while(mqtt_resp.last_message[index] != '\0')
    {
        if(mqtt_resp.last_message[index] == '\r')
        {
            mqtt_resp.last_message[index] = '\n';
        }
        index++;
    }

    char *start;
    char *topic;
    char *payload;
    char *end;

    start = strtok((char*)mqtt_resp.last_message, "\n");
    (void)strtok(NULL, "\n");
    topic = strtok(NULL, "\n");
    (void)strtok(NULL, "\n");
    payload =  strtok(NULL, "\n");
    end = strtok(NULL, "\n");

    char *ptr;
    start = strtok(start, " ");
    if((ptr = strtok(NULL, ",")) != NULL)
        mqtt_resp.client_id  = atoi(ptr);

    if((ptr = strtok(NULL, ",")) !=NULL)
        mqtt_resp.topic_lentgth = atoi(ptr);

    if((ptr = strtok(NULL, ",")) !=NULL)
        mqtt_resp.payload_length = atoi(ptr);

    if(topic != NULL)
    	strcpy(mqtt_resp.topic, topic);

    if(payload != NULL)
    	strcpy(mqtt_resp.payload, payload);

    end = strtok(end, " ");
    if((ptr = strtok(NULL, " ")) != NULL)
        mqtt_resp.end = atoi(ptr);

    return CMD_OK;

}



