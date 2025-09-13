/*
 * A7670_Commands_MQTT.c
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */
#include "A7670_Commands_MQTT.h"

MQTT mqtt;



CMD_Status A7670_MQTT_Config_MQTT(uint8_t client_id, char *client_name, char *broker_adress, uint8_t keep_alive, uint8_t clear_session, uint8_t QoS)
{
	mqtt.client.id 	 = client_id;
	strcpy(mqtt.client.name, client_name);
	strcpy(mqtt.broker.adress, broker_adress);
	mqtt.broker.kepp_alive = keep_alive;
	mqtt.broker.clear_session = clear_session;
	mqtt.message.QoS = QoS;

	MQTT_Connect_State mqtt_state = MQTT_START;

	while(mqtt_state != MQTT_OK)
	{
		switch (mqtt_state)
		{
			case MQTT_START:
				if(A7670_MQTT_CMD_Start() == CMD_OK)
				{
					mqtt_state = MQTT_ACCQ;
				}
				else
				{
					mqtt_state = MQTT_RESET_MODULE;
				}
			break;
			case MQTT_ACCQ:
				if(A7670_MQTT_CMD_Acquire_Client(&mqtt))
				{
					mqtt_state = MQTT_CONNECT;
				}
				else
				{
					mqtt_state = MQTT_RESET_MODULE;
				}
			break;
			case MQTT_CONNECT:
				if(A7670_MQTT_CMD_Connect(&mqtt))
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

CMD_Status A7670_MQTT_Publish_Message(char* topic, char* message_payload)
{

	strcpy(mqtt.message.topic, topic);
	strcpy(mqtt.message.payload, message_payload);

	Publish_Message_state pub_msg_state = MSG_SET_TOPIC;

	while(pub_msg_state != MSG_OK)
	{
		switch (pub_msg_state) {
			case MSG_SET_TOPIC:
				if(A7670_MQTT_CMD_Pub_Topic(&mqtt) == CMD_OK)
					pub_msg_state = MSG_SET_PAYLOAD;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_SET_PAYLOAD:
				if(A7670_MQTT_CMD_Payload(&mqtt) == CMD_OK)
					pub_msg_state = MSG_PUBLISH;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_PUBLISH:
				if(A7670_MQTT_CMD_Publish(&mqtt) == CMD_OK)
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

CMD_Status A7670_MQTT_CMD_Start()
{
	strcpy(at.at_command, "AT+CMQTTSTART");
	if(AT_sendCommand() == AT_OK)
	{
		AT_config_Wait_Response("CMQTTSTART: 0", 9000);
		if(AT_check_Wait_Response_Blocking() == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}
CMD_Status A7670_MQTT_CMD_Acquire_Client(MQTT *mqtt)
{
	sprintf(at.at_command, "%s%d,\"%s\"", "AT+CMQTTACCQ=", mqtt->client.id, mqtt->client.name);
	if(AT_sendCommand() ==  AT_OK)
	{
		AT_config_Wait_Response("OK", 5000);
		if(AT_check_Wait_Response_Blocking() == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}
CMD_Status A7670_MQTT_CMD_Connect(MQTT *mqtt)
{
	sprintf(at.at_command, "%s%d,\"%s\",%d,%d", "AT+CMQTTCONNECT=", mqtt->client.id, mqtt->broker.adress, mqtt->broker.kepp_alive, mqtt->broker.clear_session);
	if(AT_sendCommand() == AT_OK)
	{
		AT_config_Wait_Response("CMQTTCONNECT: 0,0", 5000);
		if(AT_check_Wait_Response_Blocking() == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}




CMD_Status A7670_MQTT_CMD_Pub_Topic(MQTT *mqtt)
{
	uint8_t topic_length   = strlen(mqtt->message.topic);
	sprintf(at.at_command, "%s%d,%d", "AT+CMQTTTOPIC=", mqtt->client.id, topic_length);
	if(AT_sendCommand() == AT_OK)
	{
		HAL_Delay(50);
		strcpy(at.at_command, mqtt->message.topic);
		if(AT_sendCommand() == AT_OK)
		{
			AT_config_Wait_Response("OK", 1500);
			if(AT_check_Wait_Response_Blocking() == AT_OK)
				return CMD_OK;
		}

	}
	return CMD_ERROR;

}


CMD_Status A7670_MQTT_CMD_Payload(MQTT *mqtt)
{
	uint8_t payload_length = strlen(mqtt->message.payload);
	sprintf(at.at_command, "%s%d,%d", "AT+CMQTTPAYLOAD=", mqtt->client.id, payload_length);
	if(AT_sendCommand() == AT_OK)
	{
		HAL_Delay(50);
		strcpy(at.at_command, mqtt->message.payload);
		if(AT_sendCommand() == AT_OK)
		{
			AT_config_Wait_Response("OK", 1500);
			if(AT_check_Wait_Response_Blocking() == AT_OK)
				return CMD_OK;
		}

	}
	return CMD_ERROR;
}

CMD_Status A7670_MQTT_CMD_Publish(MQTT *mqtt)
{
	//strcpy(at->at_command, "AT+CMQTTPUB=0,1,60");
	sprintf(at.at_command, "%s%d,%d,%d", "AT+CMQTTPUB=", mqtt->client.id, mqtt->message.QoS, mqtt->broker.kepp_alive);
	if(AT_sendCommand() == AT_OK)
	{
		AT_config_Wait_Response("CMQTTPUB: 0,0", 1500);
		if(AT_check_Wait_Response_Blocking() == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}






