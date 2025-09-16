/*
 * AT_decoder.c
 *
 *  Created on: Sep 4, 2025
 *      Author: kenny
 */

#include "AT_decoder.h"

AT_INFO at;


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


