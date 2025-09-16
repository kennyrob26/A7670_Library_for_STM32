/*
 * A7670_Commands_MQTT.h
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_

#include "A7670_At_Commands.h"

typedef struct MQTT_Client
{
	uint8_t id;
	char name[20];
}MQTT_Client;

typedef struct MQTT_Message
{
	uint8_t QoS;
	char topic[20];
	char payload[100];

}MQTT_Message;


typedef struct MQTT_Broker
{
	char adress[40];
	uint16_t kepp_alive;
	uint8_t clear_session;
	uint8_t status;
}MQTT_Broker;

typedef struct MQTT
{
	MQTT_Client client;
	MQTT_Message message;
	MQTT_Broker broker;

}MQTT;


typedef enum
{
	MSG_OK    		  = 0,
	MSG_SET_TOPIC	  = 1,
	MSG_SET_PAYLOAD   = 2,
	MSG_PUBLISH       = 3,
	MSG_RESET_MODULE  = 4

}Publish_Message_state;

typedef enum
{
	MQTT_OK      = 0,
	MQTT_START   = 1,
	MQTT_ACCQ    = 2,
	MQTT_CONNECT = 3,
	MQTT_RESET_MODULE

}MQTT_Connect_State;


CMD_Status A7670_MQTT_Config_MQTT( uint8_t client_id, char *client_name, char *broker_adress, uint8_t keep_alive, uint8_t clear_session, uint8_t QoS);
CMD_Status A7670_MQTT_Publish_Message( char* topic, char* message_payload);
CMD_Status A7670_MQTT_Subscribe_Topic(char* topic);
CMD_Status A7670_MQTT_CMD_Start(void);
CMD_Status A7670_MQTT_CMD_Acquire_Client(void);
CMD_Status A7670_MQTT_CMD_Connect(void);
CMD_Status A7670_MQTT_CMD_Pub_Topic(void);
CMD_Status A7670_MQTT_CMD_Payload(void);
CMD_Status A7670_MQTT_CMD_Publish(void);
CMD_Status A7670_MQTT_CMD_Sub_Topic(void);
CMD_Status A7670_MQTT_CMD_Confirm_sub_topic(void);




#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_ */
