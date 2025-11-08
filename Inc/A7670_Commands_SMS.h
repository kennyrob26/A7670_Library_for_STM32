/*
 * A7670_Commands_SMS.h
 *
 *  Created on: Nov 7, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SMS_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SMS_H_

#include "AT_Handler.h"

#define SMS_MAX_MESSAGES   5
#define SMS_MESSAGE_LENTGH 150

typedef struct
{
    char response[150];
    char status[12];
    char origin_adress[20];
    char date_hour[25];
    char payload[50];
} SMS_Message;

extern SMS_Message sms_message;
/*
typedef struct
{
    uint8_t head;
    uint8_t tail;
    uint8_t count;
}RingBuffer;
*/
typedef struct
{
	volatile char response[SMS_MAX_MESSAGES][SMS_MESSAGE_LENTGH];
	volatile char last_message[SMS_MESSAGE_LENTGH];
	RingBuffer ring_buffer;
}SmsRingBufferReceive;

//SmsRingBufferReceive sms_queue;

CMD_Status A7670_SMS_Register_Callback_Response(void (*callback_function)(SMS_Message sms_message));
CMD_Status A7670_SMS_QueuePushMessage(char *smsMessage);
CMD_Status A7670_SMS_QueuePopMessage();
void A7670_SMS_ReadNewMessages();
void A7670_SMS_ResponseHandler();
void SMS_readMessage(char* message);
CMD_Status A7670_SMS_CMD_CMGR(uint8_t message_id);

#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SMS_H_ */
