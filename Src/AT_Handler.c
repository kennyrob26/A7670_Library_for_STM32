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
		at.response[Size] = '\0';

		if(strstr((char*)at.response, "+CMQTTRXEND: 0") != NULL )
			A7670_MQTT_QueuePushMessage(at.response);
		else if(strstr((char*)at.response, "+CMTI: \"SM\"") != NULL)
			A7670_SMS_QueuePushMessage(at.response);
		else
			at.existMessage = 1;

		HAL_UARTEx_ReceiveToIdle_DMA(at.huart, (uint8_t*)at.response, BUFFER_LENGTH);
	}

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == at.huart)
    {
        __HAL_UART_CLEAR_FEFLAG(at.huart);
        __HAL_UART_CLEAR_NEFLAG(at.huart);
        __HAL_UART_CLEAR_OREFLAG(at.huart);

        HAL_UARTEx_ReceiveToIdle_DMA(at.huart, (uint8_t*)at.response, BUFFER_LENGTH);
    }
}

/**
 * @brief Defines wich UART is used for comunication with A7670
 * 
 * This function sets the UART where the AT command is sent and sets the function for DMA interrupt.
 * 
 * @param huartx is a pointer to the instance uart
 * @return AT_Status
 * @retval AT_OK if uart configuration is successful
 * @retval AT_ERROR if it was not possible to configure the uart
 */

AT_Status AT_defineUART(UART_HandleTypeDef *huartx)
{
	at.huart = huartx;
	if(at.huart != NULL)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(huartx, (uint8_t*)at.response, BUFFER_LENGTH);
		return AT_OK;
	}
	else
		return AT_ERROR;
}

/**
 * @brief This function allows us to configure the wait for a command response, where we define the expected response, and a maximum response time.
 * @param expected_response the String we expect as a response
 * @param timeout the maximum expected time for find the expected_response
 */
AT_Status AT_configWaitResponse(const char *expected_response, uint32_t timeout)
{
	at.wait_response.start_tick = HAL_GetTick();
	at.wait_response.timeout = timeout;
	strcpy(at.wait_response.expected_response, expected_response);
	at.wait_response.waiting_status = 1;

	return AT_OK;
}

/**
 * @brief non-blocking function that waits for a command response, but first you must configure AT_configWaitResponse.
 * @return This function return AT_Status
 * @retval AT_ERROR if not in wait mode, this occurs when AT_configWaitResponse has not been configured
 * @retval AT_OK if response to command was found
 * @retval AT_TIMEOUT if the set timeout has not elapsed
 * @retval AT_WAITING if we are still waiting for an answer, and time is not up
 */
AT_Status AT_checkWaitResponse()
{
	if(at.wait_response.waiting_status == 0)
		return AT_ERROR;

	if(at.existMessage == 1)
	{
		if(strstr((char*)at.response, at.wait_response.expected_response) != NULL)			//Significa que a mensagem contém o Token procurado
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

/**
 * @brief This is the blocking version of AT_checkWaitResponse() 
 * 
 * Where the code blocks until the response is not found, or the timeout is exceeded.
 *  We call AT_checkWaitResponse() until the response is found or the timeout is exceeded.
 * 
 * @return AT_Status
 */

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

/**
 * @brief Sends the AT command to the UART. 
 * 
 * The command must be previously defined in the AT_INFO command (at.command).
 * This function uses to AT_checkWaitResponse_Blocking for waitng expected response.
 * 
 * @param expected_response this is the response string expected as a response to the command sent
 * @param timeout set a maximum response wait time
 * @return return a AT_Status
 * @retval AT_OK if the command is sent successfully and the expected response is found
 * @retval AT_ERROR if unable to send command on UART
 * @retval AT_NOT_RESPONSE if the command is sent but the expected response is not found
 */

AT_Status AT_sendCommand(const char *command, const char *expected_response, uint16_t timeout)
{

	strcpy(at.command, command);
	uint8_t exist_expected_response = strlen(expected_response);
	uint8_t length = strlen(at.command);

	at.command[length]     = '\r';
	at.command[length + 1] = '\0';

	if(HAL_UART_Transmit(at.huart, (uint8_t*)at.command, length+1, 500) != HAL_OK)
	{
		return AT_ERROR;
	}
	else
	{
		if(exist_expected_response && timeout)
		{
			AT_configWaitResponse(expected_response, timeout);
			if(AT_checkWaitResponse_Blocking() == AT_OK)
				return AT_OK;
			else
				return AT_NOT_RESPONSE;
		}
		return AT_OK;
	}
	return AT_ERROR;
}

/**
 * @brief Checks for new messages on the UART
 * 
 * This fuction allows us to monitor if exist new messages in UART Buffer
 * 
 * @param timeout this parameter defines the maximum time we want to wait for a new message
 * @return return a AT_Status, where AT_OK if exist a new Message, and AT_ERROR if not exist a new message
 */

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








