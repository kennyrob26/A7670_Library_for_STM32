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


CMD_Status A7670_MQTT_CMD_Start(AT_INFO *at);
CMD_Status A7670_MQTT_CMD_Acquire_Client(AT_INFO *at, MQTT *mqtt);
CMD_Status A7670_MQTT_CMD_Connect(AT_INFO *at, MQTT *mqtt);
CMD_Status A7670_MQTT_Publish_Message(AT_INFO *at, MQTT *mqtt);
CMD_Status A7670_MQTT_CMD_Pub_Topic(AT_INFO *at, MQTT *mqtt);
CMD_Status A7670_MQTT_CMD_PTOPIC(AT_INFO *at, MQTT *mqtt);
CMD_Status A7670_MQTT_CMD_Payload(AT_INFO *at, MQTT *mqtt);
CMD_Status A7670_MQTT_CMD_Publish(AT_INFO *at, MQTT *mqtt);
#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_ */
