/*
 * A7670_Commands_SMS.c
 *
 *  Created on: Nov 7, 2025
 *      Author: kenny
 */

#include "A7670_Commands_SMS.h"

SmsRingBufferReceive sms_queue;
SMS_Message sms_message;

void (*SMS_Callback_Response)(SMS_Message sms_message);

CMD_Status A7670_SMS_Register_Callback_Response(void (*callback_function)(SMS_Message sms_message))
{
	SMS_Callback_Response = callback_function;

	if(SMS_Callback_Response != NULL)
		return CMD_OK;
	else
		return CMD_ERROR;
}

static uint8_t RingBufferIsFull(RingBuffer *ring_buffer)
{
    if(ring_buffer->count >= SMS_MAX_MESSAGES)
        return 1;
    else
        return 0;
}

static uint8_t RingBufferIsEmpty(RingBuffer *ring_buffer)
{
    if(ring_buffer->count == 0)
        return 1;
    else
        return 0;
}

CMD_Status A7670_SMS_QueuePushMessage(char *smsMessage)
{
    if(RingBufferIsFull((RingBuffer*)&sms_queue.ring_buffer) == 0)
    {
    	uint8_t *head  = (uint8_t*) &sms_queue.ring_buffer.head;
    	uint8_t *count = (uint8_t*) &sms_queue.ring_buffer.count;

        strncpy((char*)sms_queue.response[*head], (char*)smsMessage, (SMS_MESSAGE_LENTGH - 1));
        sms_queue.response[*head][SMS_MESSAGE_LENTGH - 1] = '\0';

       *head = ((*head) + 1) % SMS_MAX_MESSAGES;

        (*count)++;

        return CMD_OK;
    }

    return CMD_ERROR;
}

CMD_Status A7670_SMS_QueuePopMessage()
{
    if(RingBufferIsEmpty((RingBuffer*)&sms_queue.ring_buffer) == 0)
    {
    	uint8_t *tail  = (uint8_t*) &sms_queue.ring_buffer.tail;
    	uint8_t *count = (uint8_t*) &sms_queue.ring_buffer.count;

        strncpy((char*)sms_queue.last_message, (char*)sms_queue.response[*tail], SMS_MESSAGE_LENTGH);
        sms_queue.last_message[SMS_MESSAGE_LENTGH - 1] = '\0';

        *tail = ((*tail) + 1) % SMS_MAX_MESSAGES;

        (*count)--;

        return CMD_OK;
    }
    else
    	sms_queue.last_message[0] = '\0';

    return CMD_ERROR;

}

void A7670_SMS_ReadNewMessages()
{
	uint32_t start_tick = HAL_GetTick();
	while(RingBufferIsEmpty((RingBuffer*)&sms_queue.ring_buffer) != 1 && ((HAL_GetTick() - start_tick) < 1000))
	{
		if(A7670_SMS_QueuePopMessage() == CMD_OK)
			A7670_SMS_ResponseHandler();
	}
}

void A7670_SMS_ResponseHandler()
{
    char *remove_echo;
    uint8_t message_id = 0;

    remove_echo = strstr((char*)sms_queue.last_message, "+CMTI:");
    if(remove_echo == NULL)
        return;

    strncpy(sms_message.response, remove_echo, (SMS_MESSAGE_LENTGH - 1));
    sms_message.response[SMS_MESSAGE_LENTGH - 1] = '\0';

    const char filter[] = "+CMTI: \"SM\",%hhu";
    sscanf(sms_message.response, filter, &message_id);

    if(A7670_SMS_CMD_CMGR(message_id) == CMD_OK)
    {
    	SMS_readMessage((char*)at.response);
    }

}

static void nextSmsValue(char *value)
{
    static uint8_t count = 0;

    static char *ptr = NULL;
    static char *ptr_end = NULL;

    if(ptr_end == NULL)
    {
        count = 0;
        ptr = strstr(sms_message.response, "+CMGR: \"") + 8;

        if(ptr == NULL)
            return;

        ptr_end = strstr(ptr, "\"");
    }
    else
    {
        ptr = (ptr_end + 1);
        if(count < 3)
        {
            ptr = strstr(ptr, "\"") + 1;
            ptr_end = strstr(ptr, "\"");
            count++;
        }
        else if(count == 3)
        {
            ptr = strstr(ptr, "\r\n") + 2;
            ptr_end = strstr(ptr, "\r");
            count = 4;
        }
    }

    if((value != NULL) && (ptr_end != NULL) && ptr != NULL)
    {
        *ptr_end = '\0';
        strcpy(value, ptr);
    }

    if(count == 4)
    {
        ptr_end = NULL;
    }
}

static void convertDateHourUTC(char* date_hour)
{
    char *ptr;

    ptr = strstr(date_hour, ",");
    *ptr = 'T';
    ptr = strstr(date_hour, "+");
    *ptr = '\0';

}


void SMS_readMessage(char* message)
{
    char *remove_echo;
    remove_echo = strstr(message, "+CMGR: ");

    if(remove_echo == NULL)
        return;

    strcpy(sms_message.response, remove_echo);

    nextSmsValue(sms_message.status);
    nextSmsValue(sms_message.origin_adress);
    nextSmsValue(NULL);
    nextSmsValue(sms_message.date_hour);
    convertDateHourUTC(sms_message.date_hour);
    nextSmsValue(sms_message.payload);

    SMS_Callback_Response(sms_message);
}


CMD_Status A7670_SMS_CMD_CMGR(uint8_t message_id)
{
	char command[30];
	sprintf(command, "AT+CMGR=%d", message_id);
	if(AT_sendCommand(command, "OK", 500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}


