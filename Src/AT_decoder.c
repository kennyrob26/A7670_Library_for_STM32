/*
 * AT_decoder.c
 *
 *  Created on: Sep 4, 2025
 *      Author: kenny
 */

#include "AT_decoder.h"

AT_INFO at;

AT_Status existNewMessage();
AT_Status processMessage();

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == at.huart)
	{
		at.response_buffer[Size] = '\0';
		at.existMessage = 1;
		HAL_UARTEx_ReceiveToIdle_DMA(at.huart, (uint8_t*)at.response_buffer, TAMANHO_MENSAGEM);
	}

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == at.huart) // ajuste para sua UART
    {
        // Limpa flags de erro
        __HAL_UART_CLEAR_FEFLAG(at.huart);
        __HAL_UART_CLEAR_NEFLAG(at.huart);
        __HAL_UART_CLEAR_OREFLAG(at.huart);

        // Reinicia o DMA
        HAL_UARTEx_ReceiveToIdle_DMA(at.huart, (uint8_t*)at.response_buffer, TAMANHO_MENSAGEM);
    }
}


AT_Status AT_defineUART(UART_HandleTypeDef *huartx)
{
	at.huart = huartx;
	if(at.huart != NULL)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(huartx, (uint8_t*)at.response_buffer, TAMANHO_MENSAGEM);
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
	if(existNewMessage() == AT_OK)
	{
		if(processMessage() == AT_OK)
			return AT_OK;
		else
			return AT_ERROR;
	}
	else
		return AT_ERROR;

}
AT_Status existNewMessage()
{
	uint32_t tempo = HAL_GetTick();
	uint32_t tempo_inicio = tempo;
	while(at.existMessage != 1 && (tempo - tempo_inicio) < TIMEOUT)
	{
		tempo = HAL_GetTick();
	}

	if(at.existMessage == 1)
		return AT_OK;
	else
		return AT_ERROR;

}

AT_Status processMessage()
{
	at.existMessage = 0;

	if(strlen((char*)at.response_buffer) == 0)
		return AT_ERROR;

	if(strncmp((char*)at.response_buffer, "AT", 2) == 0)
	{
		at.echo = strtok(at.response_buffer, "\r\r\n");
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
	}
	else
	{
		at.echo = NULL;
		strcpy((char*)at.response, at.response_buffer);

		return AT_OK;
	}

	return AT_ERROR;
}





