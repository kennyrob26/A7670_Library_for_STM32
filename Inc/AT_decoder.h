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
#include "stm32g4xx_hal.h"

//#define AT_OK 1
//#define AT_ERROR 0

#define STATE_SEND_COMMAND 0
#define STATE_RECEIVE_COMMAND 1
#define STATE_WAIT_COMMAND 2



#define TAMANHO_MENSAGEM 100
#define TIMEOUT 50

typedef enum
{
	AT_OK      = 0,		//OK     -> Tudo correu como o esperado
	AT_ERROR   = 1,		//ERROR  -> Algo deu errado
	AT_WAITING = 2,		//WAITING-> Estamos esperando a tarefa ser conclida
	AT_TIMEOUT = 3		//TIMEOUT-> O tempo para executar a tarefa estourou
}AT_Status;

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
	volatile uint8_t response_buffer[TAMANHO_MENSAGEM];
    char *echo;
    char *response;
    char *OK;
    uint8_t status;
    volatile uint8_t existMessage;
    AT_Wait_Response wait_response;
}AT_INFO;

extern AT_INFO at;


//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
//void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

AT_Status AT_defineUART(UART_HandleTypeDef *huartx);
AT_Status AT_processCommand();
AT_Status AT_config_Wait_Response(const char *expected_response, uint32_t timeout);
AT_Status AT_check_Wait_Response();
AT_Status AT_check_Wait_Response_Blocking();
AT_Status AT_sendCommand();
AT_Status AT_responseCommand();



#endif /* INC_AT_DECODER_H_ */
