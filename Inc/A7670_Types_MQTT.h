/*
 * A76070_MQTT_Types.h
 *
 *  Created on: Sep 17, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_TYPES_MQTT_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_TYPES_MQTT_H_

#include <stdint.h>
#include "stdio.h"
#include <AT_Handler.h>

#define MAX_MQTT_RECEIVE_MESSAGE 3
#define SIZE_MESSAGE 100
#define MAX_MQTT_SEND_MESSAGE 5
#define TOPIC_SIZE 20
#define PAYLOAD_SIZE 40

typedef enum
{
	MQTT_CONNECT_OK    = 0,
	MQTT_CHECK_NETWORK = 1,
	MQTT_START         = 2,
	MQTT_ACCQ          = 3,
	MQTT_SSL_CONFIG    = 4,
	MQTT_CONNECT       = 5,
	MQTT_CONNECT_ERROR = 6

}MQTT_Connect_State;

typedef enum
{
	MQTT_CON_OK                    = 0,
	MQTT_CON_ERROR                 = 1,
	MQTT_CON_ERROR_NETWORK         = 2,
	MQTT_CON_ERROR_STARTING_MODULE = 3,
	MQTT_CON_ERROR_CLIENT      	   = 4,
	MQTT_CON_ERROR_BROKER          = 5,
	MQTT_CON_ERROR_AUTH		       = 6,
	MQTT_CON_ERROR_SSL			   = 7,
	MQTT_CON_ERROR_INVALID_VALUE   = 8,
	MQTT_CON_ERROR_TIMEOUT         = 9
}MQTT_Status;

typedef enum
{
	MQTT_DISC_DISCONNECT_OK        = 0,
	MQTT_DISC_DISCONNECT           = 1,
	MQTT_DISC_CHECK_ACQUIRE_CLIENT = 2,
	MQTT_DISC_REALESE_CLIENT       = 3,
	MQTT_DISC_STOP                 = 4,
	MQTT_DISC_ERROR                = 5
}MQTT_Disconnect_State;


typedef enum
{
	MQTT_RECONNECT_DISABLE  = 0,
	MQTT_RECONNECT_ENABLE   = 1
}MQTT_Auto_Reconnect;

typedef enum
{
	MQTT_CLIENT_NOT_ACQUIRED = 0,
	MQTT_CLIENT_ACQUIRED     = 1,
	MQTT_CLIENT_ERROR        = 2
}MQTT_Client_State;

typedef enum
{
	MQTT_SSL_DISABLE = 0,
	MQTT_SSL_ENABLE  = 1
}MQTT_SSL_State;

typedef enum
{
	MQTT_AUTH_DISABLE = 0,
	MQTT_AUTH_ENABLE  = 1
}MQTT_Auth_State;

typedef enum
{
	MQTT_BROKER_DISCONNECT    = 0,
	MQTT_BROKER_CONNECTING    = 1,
	MQTT_BROKER_CONNECTED     = 2,
	MQTT_BROKER_DISCONNECTING = 3
} MQTT_Broker_State;


typedef enum
{
	MSG_OK    		  = 0,
	MSG_SET_TOPIC	  = 1,
	MSG_SET_PAYLOAD   = 2,
	MSG_PUBLISH       = 3,
	MSG_RESET_MODULE  = 4

}Publish_Message_state;



//=================== -- Structs -- =======================

typedef struct MQTT_Client
{
	uint8_t id;
	char name[20];
}MQTT_Client;

typedef struct MQTT_Auth
{
	char username[20];
	char password[30];
}MQTT_Auth;

typedef struct MQTT_Message
{
	char topic[TOPIC_SIZE];
	char payload[PAYLOAD_SIZE];

}MQTT_Message;


typedef struct MQTT_Broker
{
	char adress[90];
	uint16_t kepp_alive;
	uint8_t clear_session;
	uint8_t status;
	uint8_t QoS;
}MQTT_Broker;

typedef struct MQTT
{
	MQTT_Client client;
	MQTT_Message message;
	MQTT_Broker broker;
	MQTT_Auth auth;
	MQTT_Auth_State auth_state;
	MQTT_Broker_State broker_state;
	MQTT_Auto_Reconnect auto_reconect;
	MQTT_SSL_State ssl_state;

}MQTT;

typedef struct
{
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} RingBuffer;

typedef struct
{
	MQTT_Message message[MAX_MQTT_SEND_MESSAGE];
	RingBuffer ring_buffer;
    uint32_t start_tick;
} MqttRingBufferSend;

typedef struct
{
    char message[MAX_MQTT_RECEIVE_MESSAGE][SIZE_MESSAGE];
    RingBuffer ring_buffer;
} MqttRingBufferResponse;


typedef struct MQTT_RESPONSE
{
	volatile MqttRingBufferResponse queue;
	volatile char last_message[BUFFER_LENGTH];
	uint8_t client_id;
	MQTT_Message message;
    uint8_t topic_lentgth;
    uint8_t payload_length;
    uint8_t end;
}MQTT_RESPONSE;

extern MQTT_RESPONSE mqtt_resp;


#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_TYPES_MQTT_H_ */
