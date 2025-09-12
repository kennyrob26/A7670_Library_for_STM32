/*
 * A7670_Commands_MQTT.c
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */
#include "A7670_Commands_MQTT.h"

CMD_Status A7670_MQTT_Config_MQTT(AT_INFO *at, MQTT *mqtt)
{
	MQTT_Connect_State mqtt_state = MQTT_START;

	while(mqtt_state != MQTT_OK)
	{
		switch (mqtt_state)
		{
			case MQTT_START:
				if(A7670_MQTT_CMD_Start(at) == CMD_OK)
				{
					mqtt_state = MQTT_ACCQ;
				}
				else
				{
					mqtt_state = MQTT_RESET_MODULE;
				}
			break;
			case MQTT_ACCQ:
				if(A7670_MQTT_CMD_Acquire_Client(at, mqtt))
				{
					mqtt_state = MQTT_CONNECT;
				}
				else
				{
					mqtt_state = MQTT_RESET_MODULE;
				}
			break;
			case MQTT_CONNECT:
				if(A7670_MQTT_CMD_Connect(at, mqtt))
				{
					mqtt_state = MQTT_OK;
				}
				else
				{
					mqtt_state = MQTT_RESET_MODULE;
				}
			break;
			case MQTT_RESET_MODULE:
				return CMD_ERROR;
			default:
				return CMD_ERROR;
				break;

		}
	}
}

CMD_Status A7670_MQTT_CMD_Start(AT_INFO *at)
{
	strcpy(at->at_command, "AT+CMQTTSTART");
	if(AT_sendCommand(at) == AT_OK)
	{
		AT_config_Wait_Response(at, "CMQTTSTART: 0", 9000);
		if(AT_check_Wait_Response_Blocking(at) == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}
CMD_Status A7670_MQTT_CMD_Acquire_Client(AT_INFO *at, MQTT *mqtt)
{
	sprintf(at->at_command, "%s%d,\"%s\"", "AT+CMQTTACCQ=", mqtt->client.id, mqtt->client.name);
	if(AT_sendCommand(at) ==  AT_OK)
	{
		AT_config_Wait_Response(at, "OK", 5000);
		if(AT_check_Wait_Response_Blocking(at) == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}
CMD_Status A7670_MQTT_CMD_Connect(AT_INFO *at, MQTT *mqtt)
{
	sprintf(at->at_command, "%s%d,\"%s\",%d,%d", "AT+CMQTTCONNECT=", mqtt->client.id, mqtt->broker.adress, mqtt->broker.kepp_alive, mqtt->broker.clear_session);
	if(AT_sendCommand(at) == AT_OK)
	{
		AT_config_Wait_Response(at, "CMQTTCONNECT: 0,0", 5000);
		if(AT_check_Wait_Response_Blocking(at) == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}


CMD_Status A7670_MQTT_Publish_Message(AT_INFO *at, MQTT *mqtt)
{
	Publish_Message_state pub_msg_state = MSG_SET_TOPIC;

	while(pub_msg_state != MSG_OK)
	{
		switch (pub_msg_state) {
			case MSG_SET_TOPIC:
				if(A7670_MQTT_CMD_Pub_Topic(at, mqtt) == CMD_OK)
					pub_msg_state = MSG_SET_PAYLOAD;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_SET_PAYLOAD:
				if(A7670_MQTT_CMD_Payload(at, mqtt) == CMD_OK)
					pub_msg_state = MSG_PUBLISH;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_PUBLISH:
				if(A7670_MQTT_CMD_Publish(at, mqtt) == CMD_OK)
					pub_msg_state = MSG_OK;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_RESET_MODULE:
				return CMD_ERROR;
			default:
				return CMD_ERROR;
				break;
		}
	}

	return CMD_OK;

}

CMD_Status A7670_MQTT_CMD_Pub_Topic(AT_INFO *at, MQTT *mqtt)
{
	uint8_t topic_length   = strlen(mqtt->message.topic);
	sprintf(at->at_command, "%s%d,%d", "AT+CMQTTTOPIC=", mqtt->client.id, topic_length);
	if(AT_sendCommand(at) == AT_OK)
	{
		HAL_Delay(50);
		strcpy(at->at_command, mqtt->message.topic);
		if(AT_sendCommand(at) == AT_OK)
		{
			AT_config_Wait_Response(at, "OK", 1500);
			if(AT_check_Wait_Response_Blocking(at) == AT_OK)
				return CMD_OK;
		}

	}
	return CMD_ERROR;

}


CMD_Status A7670_MQTT_CMD_Payload(AT_INFO *at, MQTT *mqtt)
{
	uint8_t payload_length = strlen(mqtt->message.payload);
	sprintf(at->at_command, "%s%d,%d", "AT+CMQTTPAYLOAD=", mqtt->client.id, payload_length);
	if(AT_sendCommand(at) == AT_OK)
	{
		HAL_Delay(50);
		strcpy(at->at_command, mqtt->message.payload);
		if(AT_sendCommand(at) == AT_OK)
		{
			AT_config_Wait_Response(at, "OK", 1500);
			if(AT_check_Wait_Response_Blocking(at) == AT_OK)
				return CMD_OK;
		}

	}
	return CMD_ERROR;
}

CMD_Status A7670_MQTT_CMD_Publish(AT_INFO *at, MQTT *mqtt)
{
	//strcpy(at->at_command, "AT+CMQTTPUB=0,1,60");
	sprintf(at->at_command, "%s%d,%d,%d", "AT+CMQTTPUB=", mqtt->client.id, mqtt->message.QoS, mqtt->broker.kepp_alive);
	if(AT_sendCommand(at) == AT_OK)
	{
		AT_config_Wait_Response(at, "CMQTTPUB: 0,0", 1500);
		if(AT_check_Wait_Response_Blocking(at) == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}






