/*
 * AT_decoder.c
 *
 *  Created on: Sep 4, 2025
 *      Author: kenny
 */

#include "AT_decoder.h"

AT_INFO at;
MQTT_RESPONSE mqtt_resp;
AT_Status processAtCommand();
AT_Status mqtt_response();

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == at.huart)
	{
		at.response_buffer[Size] = '\0';
		at.existMessage = 1;
		HAL_UARTEx_ReceiveToIdle_DMA(at.huart, (uint8_t*)at.response_buffer, BUFFER_LENGTH);
	}

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == at.huart)
    {
        __HAL_UART_CLEAR_FEFLAG(at.huart);
        __HAL_UART_CLEAR_NEFLAG(at.huart);
        __HAL_UART_CLEAR_OREFLAG(at.huart);

        HAL_UARTEx_ReceiveToIdle_DMA(at.huart, (uint8_t*)at.response_buffer, BUFFER_LENGTH);
    }
}


AT_Status AT_defineUART(UART_HandleTypeDef *huartx)
{
	at.huart = huartx;
	if(at.huart != NULL)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(huartx, (uint8_t*)at.response_buffer, BUFFER_LENGTH);
		return AT_OK;
	}
	else
		return AT_ERROR;
}


AT_Status AT_processCommand()
{
	at.existMessage = 0;
	if (AT_sendCommand() == AT_OK)
	{
		if (AT_responseCommand() == AT_OK)
		{
			if(at.echo != NULL)
			{
				if(strstr(at.echo, at.at_command))
					return AT_OK;
				else
					return AT_ERROR;
			}
			else
				return AT_OK;
		}
	}
	return AT_ERROR;
}

AT_Status AT_config_Wait_Response(const char *expected_response, uint32_t timeout)
{
	at.wait_response.start_tick = HAL_GetTick();
	at.wait_response.timeout = timeout;
	strcpy(at.wait_response.expected_response, expected_response);
	at.wait_response.waiting_status = 1;

	return AT_OK;
}

AT_Status AT_check_Wait_Response()
{
	if(at.wait_response.waiting_status == 0)
		return AT_ERROR;

	if(at.existMessage == 1)
	{
		if(strstr((char*)at.response_buffer, at.wait_response.expected_response) != NULL)			//Significa que a mensagem contém o Token procurado
		{
			at.existMessage = 0;
			at.wait_response.waiting_status = 0;
			return AT_OK;
		}
		else
		{
			at.existMessage = 0;
		}
	}

	if((HAL_GetTick() - at.wait_response.start_tick) > at.wait_response.timeout)
	{
		at.wait_response.waiting_status = 0;
		return AT_TIMEOUT;
	}

	return AT_WAITING;
}

AT_Status AT_check_Wait_Response_Blocking()
{
	uint8_t check_wait_response = AT_WAITING;

	while(check_wait_response == AT_WAITING)
	{
		check_wait_response = AT_check_Wait_Response();
	}

	if(check_wait_response == AT_OK)
		return AT_OK;
	else if(check_wait_response == AT_TIMEOUT)
		return AT_TIMEOUT;
	else
		return AT_ERROR;
}

AT_Status AT_sendCommand()
{
	uint8_t length = strlen(at.at_command);

	at.at_command[length]     = '\r';
	at.at_command[length + 1] = '\0';

	if(HAL_UART_Transmit(at.huart, (uint8_t*)at.at_command, length+1, 100) == HAL_OK)
	{
		  return AT_OK;
	}
	return AT_ERROR;
}



AT_Status AT_responseCommand()
{
	if(AT_Exist_New_Message(50) == AT_OK)
	{
		if(AT_Pocess_Buffer() == AT_OK)
			return AT_OK;
		else
			return AT_ERROR;
	}
	else
		return AT_ERROR;

}
AT_Status AT_Exist_New_Message(uint16_t timeout)
{
	if(timeout > 0)
	{
		uint32_t tempo = HAL_GetTick();
		uint32_t tempo_inicio = tempo;
		while(at.existMessage != 1 && (tempo - tempo_inicio) < timeout)
		{
			tempo = HAL_GetTick();
		}
	}

	if(at.existMessage == 1)
	{
		at.existMessage = 0;
		return AT_OK;
	}
	else
		return AT_ERROR;

}

AT_Status AT_Pocess_Buffer()
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
		return AT_OK;
	break;
	case AT_RT_MQTT_Response:
		AT_config_Wait_Response("+CMQTTRXEND: 0", 50);				//waiting for "END" of message
		if(AT_check_Wait_Response_Blocking() == AT_OK)
		{
			strcpy(mqtt_resp.last_message, (const char*)at.response_buffer);
			mqtt_response();
			return AT_OK;
		}
		else
			return AT_ERROR;
	break;
	case AT_RT_Response:
		//at.response = at.response_buffer;
		return AT_OK;
	break;
	default:
		break;
	}

    return AT_ERROR;
}

AT_Status processAtCommand()
{

    at.echo = strtok((char*)at.response_buffer, "\r\r\n");
    if(at.echo == NULL)
    {
        at.status = 0;
        return AT_ERROR;
    }

    at.response = strtok(NULL, "\r\n");
    if(at.response != NULL)
    {
        if(!strcmp(at.response, "OK"))
        {
            at.OK = at.response;
            at.status = 1;
            return AT_OK;
        }
        else if(!strcmp(at.response, "ERROR"))
        {
            at.OK = at.response;
            at.status = 0;
            return AT_ERROR;
        }
        else
        {
            at.OK = strtok(NULL, "\r\n");
            at.status = 1;
            return AT_OK;
        }
    }

    return AT_ERROR;

}

AT_Status mqtt_response()
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

    return AT_OK;

}

/*
AT_Status mqtt_response()
{
    char *cli_id;
    char *topic_len;
    char *payload_len;
    char *topic;
    char *payload;
    char *end;
    char *byte0;

    cli_id = strstr((char*)at.response_buffer, " ");
    if(cli_id != NULL)
    {
    	*cli_id ='\0';
        cli_id++;
        byte0 = strstr(cli_id, ",");
        *byte0 = '\0';
        mqtt_resp.client_id = atoi(cli_id);
    }

    topic_len = (byte0 + 1);
    if(topic_len != NULL)
    {
        byte0 = strstr(topic_len, ",");
        *byte0 = '\0';
        mqtt_resp.topic_lentgth = atoi(topic_len);
    }

    payload_len = (byte0 + 1);
    if(payload_len != NULL)
    {
        byte0 = strstr(payload_len + 1, "\r");
        *byte0 = '\0';
        byte0++;
        mqtt_resp.payload_length = atoi(payload_len);
    }

    topic = strstr(byte0, " ");
    topic = strstr((topic), "\n");
    if(topic != NULL)
    {
        topic++;
        byte0 = strstr((topic), "\r");
        *byte0 = '\0';
        byte0++;
        strcpy(mqtt_resp.topic,topic);
    }

    payload = strstr(byte0, " ");
    payload = strstr(payload, "\n");
    if(payload != NULL)
    {
        payload++;
        byte0 = strstr(payload, "\r");
        *byte0 = '\0';
        strcpy(mqtt_resp.payload, payload);
    }

    end = strstr((byte0 + 1), " ");
    if(end != NULL)
    {
        end++;
        byte0 = strstr(end,"\r");
        *byte0 = '\0';
        mqtt_resp.end = atoi(end);
    }

    at.response_buffer[0] = '\0';
    return AT_OK;
}

AT_Status mqtt_response()
{

    char *start;
    char *topic;
    char *payload;
    char *end;

    start = strtok((char*)at.response_buffer, "\r\n");
    (void)strtok(NULL, "\r\n");
    topic = strtok(NULL, "\r\n");
    (void)strtok(NULL, "\r\n");
    payload =  strtok(NULL, "\r\n");
    end = strtok(NULL, "\r\n");

    char *ptr;
    start = strtok(start, " ");
    if((ptr = strtok(NULL, ",")) != NULL)
        mqtt_resp.client_id  = atoi(ptr);

    if((ptr = strtok(NULL, ",")) !=NULL)
        mqtt_resp.topic_lentgth = atoi(ptr);

    if((ptr = strtok(NULL, ",")) !=NULL)
        mqtt_resp.payload_length = atoi(ptr);

    if(topic != NULL)
    	mqtt_resp.topic = topic;

    if(payload != NULL)
    	mqtt_resp.payload = payload;

    end = strtok(end, " ");
    if((ptr = strtok(NULL, " ")) != NULL)
        mqtt_resp.end = atoi(ptr);


    return AT_OK;

}

*/
