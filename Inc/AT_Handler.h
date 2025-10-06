/*
 * AT_decoder.h
 *
 *  Created on: Sep 4, 2025
 *      Author: kenny
 */

#ifndef INC_AT_DECODER_H_
#define INC_AT_DECODER_H_

#include "string.h"
#include "stdint.h"
#include "stdlib.h"

#if defined(STM32G431xx)
	#include "stm32g4xx_hal.h"
#endif

#define BUFFER_LENGTH 150
#define TIMEOUT 50

typedef struct __UART_HandleTypeDef UART_HandleTypeDef;

typedef enum
{
	AT_OK           = 0,		//OK     -> Tudo correu como o esperado
	AT_ERROR        = 1,		//ERROR  -> Algo deu errado
	AT_WAITING      = 2,		//WAITING-> Estamos esperando a tarefa ser conclida
	AT_TIMEOUT      = 3,		//TIMEOUT-> O tempo para executar a tarefa estourou
	AT_NOT_RESPONSE = 4
}AT_Status;

typedef enum
{
	CMD_ERROR = 0,
	CMD_OK    = 1
} CMD_Status;

typedef struct AT_Wait_Response
{
	uint32_t start_tick;
	uint32_t timeout;
	char expected_response[30];
	uint8_t waiting_status;
}AT_Wait_Response;

typedef struct AT_INFO
{
	UART_HandleTypeDef *huart;
	char at_command[50];
	volatile uint8_t response_buffer[BUFFER_LENGTH];
    uint8_t status;
    volatile uint8_t existMessage;
    AT_Wait_Response wait_response;
}AT_INFO;

extern AT_INFO at;






//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
//void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

AT_Status AT_defineUART(UART_HandleTypeDef *huartx);
AT_Status AT_configWaitResponse(const char *expected_response, uint32_t timeout);
AT_Status AT_checkWaitResponse();
AT_Status AT_checkWaitResponse_Blocking();
AT_Status AT_sendCommand(char *expected_response, uint16_t timeout);
AT_Status AT_existNewMessage(uint16_t timeout);



//A7670_MQTT_QueueRead



#endif /* INC_AT_DECODER_H_ */
