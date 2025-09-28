/*
 * AT_decoder.c
 *
 *  Created on: Sep 4, 2025
 *      Author: kenny
 */

#include <AT_Handler.h>

AT_INFO at;


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == at.huart)
	{
		at.response_buffer[Size] = '\0';

		if(strstr(at.response_buffer, "+CMQTTRXEND: 0") != NULL)
		{
			//strcpy(mqtt_resp.last_message, at.response_buffer);
			//mqtt_resp.exist_new_response = 1;
			A7670_MQTT_QueuePushMessage(at.response_buffer);
		}

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

AT_Status AT_config_Wait_Response(const char *expected_response, uint32_t timeout)
{
	at.wait_response.start_tick = HAL_GetTick();
	at.wait_response.timeout = timeout;
	strcpy(at.wait_response.expected_response, expected_response);
	at.wait_response.waiting_status = 1;

	return AT_OK;
}

AT_Status AT_checkWaitResponse()
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

AT_Status AT_checkWaitResponse_Blocking()
{
	uint8_t check_wait_response = AT_WAITING;

	while(check_wait_response == AT_WAITING)
	{
		check_wait_response = AT_checkWaitResponse();
	}

	if(check_wait_response == AT_OK)
		return AT_OK;
	else if(check_wait_response == AT_TIMEOUT)
		return AT_TIMEOUT;
	else
		return AT_ERROR;
}


AT_Status AT_sendCommand(char *expected_response, uint16_t timeout)
{
	uint8_t exist_expected_response = strlen(expected_response);
	uint8_t length = strlen(at.at_command);

	at.at_command[length]     = '\r';
	at.at_command[length + 1] = '\0';

	if(HAL_UART_Transmit(at.huart, (uint8_t*)at.at_command, length+1, 100) != HAL_OK)
	{
		return AT_ERROR;
	}
	else
	{
		if(exist_expected_response && timeout)
		{
			AT_config_Wait_Response(expected_response, timeout);
			if(AT_checkWaitResponse_Blocking() == AT_OK)
				return AT_OK;
			else
				return AT_ERROR;
		}
		return AT_OK;
	}
	return AT_ERROR;
}


AT_Status AT_existNewMessage(uint16_t timeout)
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







