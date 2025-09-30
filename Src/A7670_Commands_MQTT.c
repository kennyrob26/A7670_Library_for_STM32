/*
 * A7670_Commands_MQTT.c
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */
#include "A7670_Commands_MQTT.h"

MQTT mqtt;
MQTT_RESPONSE mqtt_resp;

void (*MQTT_Callback_Response)(MQTT_RESPONSE mqtt_resp);



CMD_Status A7670_MQTT_configMQTT(uint8_t client_id, char *client_name, char *broker_adress, uint8_t keep_alive, uint8_t clear_session, uint8_t QoS)
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
				if(A7670_MQTT_CMD_AcquireClient())
				{
					mqtt_state = MQTT_CONNECT;
				}
				else
				{
					mqtt_state = MQTT_RESET_MODULE;
				}
			break;
			case MQTT_CONNECT:
				if(A7670_MQTT_CMD_Connect())
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
	return CMD_OK;
}



CMD_Status A7670_MQTT_CMD_Start()
{
	strcpy(at.at_command, "AT+CMQTTSTART");
	if(AT_sendCommand("CMQTTSTART: 0", 9000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}
CMD_Status A7670_MQTT_CMD_AcquireClient(void)
{
	sprintf(at.at_command, "%s%d,\"%s\"", "AT+CMQTTACCQ=", mqtt.client.id, mqtt.client.name);
	if(AT_sendCommand("OK", 5000) ==  AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}
CMD_Status A7670_MQTT_CMD_Connect(void)
{
	sprintf(at.at_command, "%s%d,\"%s\",%d,%d", "AT+CMQTTCONNECT=", mqtt.client.id, mqtt.broker.adress, mqtt.broker.kepp_alive, mqtt.broker.clear_session);
	if(AT_sendCommand("CMQTTCONNECT: 0,0", 5000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_MQTT_PublishMessage(const char* topic, const char* message_payload)
{
	//HAL_Delay(500);
	strcpy(mqtt.message.topic, topic);
	strcpy(mqtt.message.payload, message_payload);

	Publish_Message_state pub_msg_state = MSG_SET_TOPIC;

	while(pub_msg_state != MSG_OK)
	{
		switch (pub_msg_state) {
			case MSG_SET_TOPIC:
				if(A7670_MQTT_CMD_Pub_Topic() == CMD_OK)
					pub_msg_state = MSG_SET_PAYLOAD;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_SET_PAYLOAD:
				if(A7670_MQTT_CMD_Payload() == CMD_OK)
					pub_msg_state = MSG_PUBLISH;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_PUBLISH:
				if(A7670_MQTT_CMD_Publish() == CMD_OK)
					pub_msg_state = MSG_OK;
				else
					pub_msg_state = MSG_RESET_MODULE;
			break;
			case MSG_RESET_MODULE:
				return CMD_ERROR;
			default:
				return CMD_ERROR;
		}
	}

	return CMD_OK;

}


CMD_Status A7670_MQTT_CMD_Pub_Topic(void)
{
	uint8_t topic_length   = strlen(mqtt.message.topic);
	sprintf(at.at_command, "%s%d,%d", "AT+CMQTTTOPIC=", mqtt.client.id, topic_length);
	if(AT_sendCommand(">", 30) == AT_OK)
	{
		//HAL_Delay(50);
		strcpy(at.at_command, mqtt.message.topic);

		if(AT_sendCommand("OK", 30) == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;

}


CMD_Status A7670_MQTT_CMD_Payload(void)
{
	uint8_t payload_length = strlen(mqtt.message.payload);
	sprintf(at.at_command, "%s%d,%d", "AT+CMQTTPAYLOAD=", mqtt.client.id, payload_length);
	if(AT_sendCommand(">", 30) == AT_OK)
	{
		strcpy(at.at_command, mqtt.message.payload);
		if(AT_sendCommand("OK", 30) == AT_OK)
				return CMD_OK;

	}
	return CMD_ERROR;
}

CMD_Status A7670_MQTT_CMD_Publish(void)
{
	//strcpy(at->at_command, "AT+CMQTTPUB=0,1,60");
	sprintf(at.at_command, "%s%d,%d,%d", "AT+CMQTTPUB=", mqtt.client.id, mqtt.message.QoS, mqtt.broker.kepp_alive);
	if(AT_sendCommand("CMQTTPUB: 0,0", 50) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_MQTT_subscribeTopic(char* topic)
{
	strcpy(mqtt.message.topic, topic);
	if(A7670_MQTT_CMD_SubTopic() == CMD_OK)
	{
		if(A7670_MQTT_CMD_ConfirmSubTopic() == CMD_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}

CMD_Status A7670_MQTT_CMD_SubTopic(void)
{
	uint8_t topic_length = strlen(mqtt.message.topic);
	sprintf(at.at_command, "%s%d,%d,%d", "AT+CMQTTSUBTOPIC=", mqtt.client.id, topic_length, mqtt.message.QoS);
	if(AT_sendCommand("", 0) == AT_OK)
	{
		HAL_Delay(50);
		strcpy(at.at_command, mqtt.message.topic);
		if(AT_sendCommand("OK", 1500) == AT_OK)
			return CMD_OK;

	}
	return CMD_ERROR;
}

CMD_Status A7670_MQTT_CMD_ConfirmSubTopic(void)
{
	strcpy(at.at_command, "AT+CMQTTSUB=0");
	if(AT_sendCommand("CMQTTSUB: 0,0", 1500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_MQTT_Register_Callback_Response(void (*callback_function)(MQTT_RESPONSE mqtt_resp))
{
	MQTT_Callback_Response = callback_function;

	if(MQTT_Callback_Response != NULL)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_MQTT_ProcessResponse()
{
	uint8_t index = 0;
	while(mqtt_resp.last_message[index] != '\0')
	{
		if(mqtt_resp.last_message[index] == '\r')
		{
			mqtt_resp.last_message[index] = '\n';
		}
		index++;
	}

	char *start;
	char *topic;
	char *payload;
	char *end;

	start = strtok((char*)mqtt_resp.last_message, "\n");
	(void)strtok(NULL, "\n");
	topic = strtok(NULL, "\n");
	(void)strtok(NULL, "\n");
	payload =  strtok(NULL, "\n");
	end = strtok(NULL, "\n");

	char *ptr;
	start = strtok(start, " ");
	if((ptr = strtok(NULL, ",")) != NULL)
		mqtt_resp.client_id  = atoi(ptr);

	if((ptr = strtok(NULL, ",")) !=NULL)
		mqtt_resp.topic_lentgth = atoi(ptr);

	if((ptr = strtok(NULL, ",")) !=NULL)
		mqtt_resp.payload_length = atoi(ptr);

	if(topic != NULL)
		strcpy(mqtt_resp.topic, topic);

	if(payload != NULL)
		strcpy(mqtt_resp.payload, payload);

	end = strtok(end, " ");
	if((ptr = strtok(NULL, " ")) != NULL)
		mqtt_resp.end = atoi(ptr);

	MQTT_Callback_Response(mqtt_resp);

	return CMD_OK;

}


/*================-- QUEUE Functions ---==========================*/

uint8_t A7670_MQTT_QueueIsFull()
{
    if(mqtt_resp.ring_buffer.count_messages >= MAX_MESSAGE)
        return 1;
    else
        return 0;
}

uint8_t A7670_MQTT_QueueIsEmpty()
{
    if(mqtt_resp.ring_buffer.count_messages == 0)
        return 1;
    else
        return 0;
}

CMD_Status A7670_MQTT_QueuePushMessage(char *mqttMessage)
{
    if(A7670_MQTT_QueueIsFull() == 0)
    {
        strcpy(mqtt_resp.ring_buffer.message[mqtt_resp.ring_buffer.head], mqttMessage);

        mqtt_resp.ring_buffer.head = (mqtt_resp.ring_buffer.head + 1) % MAX_MESSAGE;

        mqtt_resp.ring_buffer.count_messages++;

        return CMD_OK;
    }

    return CMD_ERROR;
}

CMD_Status A7670_MQTT_QueuePopMessage()
{
    if(A7670_MQTT_QueueIsEmpty() == 0)
    {
        strcpy(mqtt_resp.last_message, mqtt_resp.ring_buffer.message[mqtt_resp.ring_buffer.tail]);

        mqtt_resp.ring_buffer.tail = (mqtt_resp.ring_buffer.tail + 1) % MAX_MESSAGE;

        mqtt_resp.ring_buffer.count_messages--;

        return CMD_OK;
        //
    }
    else
    	mqtt_resp.last_message[0] = '\0';

    return CMD_ERROR;

}

void A7670_MQTT_ReadNewMessages()
{
	uint32_t start_tick = HAL_GetTick();
	while(A7670_MQTT_QueueIsEmpty() != 1 && ((HAL_GetTick() - start_tick) < 100))
	{
		if(A7670_MQTT_QueuePopMessage() == CMD_OK)
			A7670_MQTT_ProcessResponse();
	}
}




