/*
 * A7670_Commands_MQTT.c
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */
#include "A7670_Commands_MQTT.h"

MqttRingBufferSend mqtt_send;
MQTT mqtt;
MQTT_RESPONSE mqtt_resp;

void (*MQTT_Callback_Response)(MQTT_RESPONSE mqtt_resp);

/**
 * @brief A complete MQTT configure
 * 
 * A state machine that starts, configures, and connects to an MQTT broker
 * 
 * @param client_id It is the client ID and is unique for each client
 * @param client_name It is the client name
 * @param broker_adress The broker MQTT adress (tcp://broker.com:1883)
 * @param keep_alive The keep alive of connection in seconds (use 60s with default)
 * @param clear_session uses 1 for a clear connection
 * @param QoS Defines QoS values, 0 at 2
 * 
 * @return CMD_Status
 * @retval CMD_OK if it was possible to connect correctly to the broker
 * @retval CMD_ERROR if we are unable to connect to the broker
 */

CMD_Status A7670_MQTT_configMQTT(uint8_t client_id, char *client_name, char *broker_adress, uint8_t keep_alive, uint8_t clear_session, uint8_t QoS)
{
	mqtt.client.id 	 = client_id;
	strcpy(mqtt.client.name, client_name);
	strcpy(mqtt.broker.adress, broker_adress);
	mqtt.broker.kepp_alive = keep_alive;
	mqtt.broker.clear_session = clear_session;
	mqtt.broker.QoS = QoS;

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

/**
 * @brief Start MQTT
 * 
 *  This command start MQTT cominication
 * 	uses AT command CMQTTSTART
 * 
 *  IMPORTANT: MQTT may take a few seconds to initialize (~5s)
 * 
 * @return CMD_Status
 * @retval CMD_OK if the MQTT module could be started
 * @retval CMD_ERROR if unable to start MQTT connection
 */

CMD_Status A7670_MQTT_CMD_Start()
{
	strcpy(at.at_command, "AT+CMQTTSTART");
	if(AT_sendCommand("CMQTTSTART: 0", 9000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

/**
 * @brief Sets the name and id of the client MQTT
 * 
 * The parameters must be previously set in:
 *	-mqtt.client.id
 *	-mqtt.client.name
 *
 * @return CMD_Status
 */
CMD_Status A7670_MQTT_CMD_AcquireClient(void)
{
	sprintf(at.at_command, "%s%d,\"%s\"", "AT+CMQTTACCQ=", mqtt.client.id, mqtt.client.name);
	if(AT_sendCommand("OK", 5000) ==  AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

/**
 * @brief Creates a connection with to broker
 * 
 * This command depends on: 
 * - the MQTT module being active (A7670_MQTT_CMD_Start)
 * - an existing client (7670 MQTT_CMD_AcquireClient)
 * 
 * in addition, the struct MQTT mqtt must be configured
 */
CMD_Status A7670_MQTT_CMD_Connect(void)
{
	sprintf(at.at_command, "%s%d,\"%s\",%d,%d", "AT+CMQTTCONNECT=", mqtt.client.id, mqtt.broker.adress, mqtt.broker.kepp_alive, mqtt.broker.clear_session);
	if(AT_sendCommand("CMQTTCONNECT: 0,0", 5000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_MQTT_PublishHandler(const char* topic, const char* message_payload)
{
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

/**
 * @brief sets a topic for publish MQTT Message
 * 
 * Use AT command CMQTTTOPIC command to set the publishing topic
 * The parameters must be set to:
 *  -mqtt.message.topic -> is a topic name
 * Waiting response "OK"
 * 
 * @return CMD_Status
 */

CMD_Status A7670_MQTT_CMD_Pub_Topic(void)
{
	uint8_t topic_length   = strlen(mqtt.message.topic);
	sprintf(at.at_command, "%s%d,%d", "AT+CMQTTTOPIC=", mqtt.client.id, topic_length);
	if(AT_sendCommand(">", 30) == AT_OK)
	{
		strcpy(at.at_command, mqtt.message.topic);

		if(AT_sendCommand("OK", 30) == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;

}

/**
 * @brief sets a payload for publish MQTT Message
 * 
 * Uses AT command CMQTTPAYLOAD for set message payload
 * The parameter must be set to:
 * 		-mqtt.message.payload -> is a payload to message
 * Waiting response "OK"
 * 
 * @return CMD_Status
 */
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

/**
 * @brief Publish a MQTT Message
 * 
 * IMPORTANT: It is necessary to define the topic and payload before publishing
 * 		-set topic: A7670_MQTT_CMD_Topic()
 * 		-set payload: A7670_MQTT_CMD_Payload()
 * 
 * Uses the AT CMQTTPUB command to publish the defined payload to the defined topic
 * 
 * @return CMD_Status 
 */

CMD_Status A7670_MQTT_CMD_Publish(void)
{
	sprintf(at.at_command, "%s%d,%d,%d", "AT+CMQTTPUB=", mqtt.client.id, mqtt.broker.QoS, mqtt.broker.kepp_alive);
	if(AT_sendCommand("CMQTTPUB: 0,0", 50) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

/**
 * @brief Subscribe a MQTT topic
 * 
 * Subscribes the client defined in mqtt.client.id to the defined topic
 * 
 * @param topic the topic we want to subscribe to
 * 
 * @return CMD_Satus
 * @retval CMD_OK if possible subscribe to the topic
 * @retval CMD_ERROR if not possible subscribe to the topic
 */

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

/**
 * @brief Sets a topic subscribe
 * 
 * 	sets a topic subscribe using CMQTTSUBTOPIC
 * 	defines which topic we want to subscribe to, 
 *  but you still need to confirm the subscription
 * 	with A7670_MQTT_CMD_ConfirmSubTopic.
 * 
 *  the topic must be previously set in mqtt.client.topic
 * 
 * Waiting "OK"
 * 
 * @return CMD_Status
 */

CMD_Status A7670_MQTT_CMD_SubTopic(void)
{
	uint8_t topic_length = strlen(mqtt.message.topic);
	sprintf(at.at_command, "%s%d,%d,%d", "AT+CMQTTSUBTOPIC=", mqtt.client.id, topic_length, mqtt.broker.QoS);
	if(AT_sendCommand("", 0) == AT_OK)
	{
		HAL_Delay(50);
		strcpy(at.at_command, mqtt.message.topic);
		if(AT_sendCommand("OK", 1500) == AT_OK)
			return CMD_OK;

	}
	return CMD_ERROR;
}

/**
 * @brief Confirm subscription to the defined topic
 * 
 * Confirm subscription to the defined topic in function A7670_MQTT_CMD_SubTopic()
 * Use command AT CMQTTSUB=0
 * 
 * Waiting "CMQTTSUB: 0,0"
 * @return CMD_Status
 */
CMD_Status A7670_MQTT_CMD_ConfirmSubTopic(void)
{
	strcpy(at.at_command, "AT+CMQTTSUB=0");
	if(AT_sendCommand("CMQTTSUB: 0,0", 1500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

/**
 * @brief registers a Callback function to handle MQTT responses
 * 
 * The callback function receives an MQTT_RESPONSE mqtt_resp 
 * with the content of the last message received, among which we can highlight:
 * 		-mqtt_resp.message.topic -> Contains the topic of message
 * 		-mqtt_resp.message.payload -> Contains the payload of message
 */
CMD_Status A7670_MQTT_Register_Callback_Response(void (*callback_function)(MQTT_RESPONSE mqtt_resp))
{
	MQTT_Callback_Response = callback_function;

	if(MQTT_Callback_Response != NULL)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_MQTT_ResponseHandler()
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
		strcpy(mqtt_resp.message.topic, topic);

	if(payload != NULL)
		strcpy(mqtt_resp.message.payload, payload);

	end = strtok(end, " ");
	if((ptr = strtok(NULL, " ")) != NULL)
		mqtt_resp.end = atoi(ptr);

	MQTT_Callback_Response(mqtt_resp);

	return CMD_OK;

}


/*================-- QUEUE Functions ---==========================*/

uint8_t A7670_MQTT_QueueIsFull(RingBuffer *ring_buffer)
{
    if(ring_buffer->count >= MAX_MQTT_RECEIVE_MESSAGE)
        return 1;
    else
        return 0;
}

uint8_t A7670_MQTT_QueueIsEmpty(RingBuffer *ring_buffer)
{
    if(ring_buffer->count == 0)
        return 1;
    else
        return 0;
}

CMD_Status A7670_MQTT_QueuePushMessage(char *mqttMessage)
{
    if(A7670_MQTT_QueueIsFull(&mqtt_resp.queue.ring_buffer) == 0)
    {
    	uint8_t *head  = (uint8_t*) &mqtt_resp.queue.ring_buffer.head;
    	uint8_t *count = (uint8_t*) &mqtt_resp.queue.ring_buffer.count;

        strcpy((char*)mqtt_resp.queue.message[*head], (char*)mqttMessage);

       *head = ((*head) + 1) % MAX_MQTT_RECEIVE_MESSAGE;

        (*count)++;

        return CMD_OK;
    }

    return CMD_ERROR;
}

CMD_Status A7670_MQTT_QueuePopMessage()
{
    if(A7670_MQTT_QueueIsEmpty(&mqtt_resp.queue.ring_buffer) == 0)
    {
    	uint8_t *tail  = (uint8_t*) &mqtt_resp.queue.ring_buffer.tail;
    	uint8_t *count = (uint8_t*) &mqtt_resp.queue.ring_buffer.count;
        strcpy((char*)mqtt_resp.last_message, (char*)mqtt_resp.queue.message[*tail]);

        *tail = ((*tail) + 1) % MAX_MQTT_RECEIVE_MESSAGE;

        (*count)--;

        return CMD_OK;
    }
    else
    	mqtt_resp.last_message[0] = '\0';

    return CMD_ERROR;

}

void A7670_MQTT_ReadNewMessages()
{
	uint32_t start_tick = HAL_GetTick();
	while(A7670_MQTT_QueueIsEmpty(&mqtt_resp.queue.ring_buffer) != 1 && ((HAL_GetTick() - start_tick) < 100))
	{
		if(A7670_MQTT_QueuePopMessage() == CMD_OK)
			A7670_MQTT_ResponseHandler();
	}
}

/**
 * @brief schedule the publication of a message
 * 
 * The A7670 module needs a delay between one published message and another,
 * then we schedule the message on the queue which will be published later by A7670_MQTT_PubQueueMessages()
 * 
 * @param topic the topic where the message will be published
 * @param payload the payload of message
 * 
 * @return CMD_Status
 * @retval CMD_OK if the message is scheduled in the queue
 * @retval CMD_ERROR if not possible scheduled message
 */
CMD_Status A7670_MQTT_PublishMessage(const char* topic, const char* payload)
{
	if(mqtt_send.ring_buffer.count < MAX_MQTT_SEND_MESSAGE)
	{
		uint8_t *head = &mqtt_send.ring_buffer.head;
		char* mqtt_topic = mqtt_send.message[*head].topic;
		char* mqtt_payload = mqtt_send.message[*head].payload;

		strcpy(mqtt_topic, topic);
		strcpy(mqtt_payload, payload);

		*head = ((*head) + 1) % MAX_MQTT_SEND_MESSAGE;
		mqtt_send.ring_buffer.count++;

		return CMD_OK;
	}
	else
		return CMD_ERROR;
}

/**
 * @brief Publish a next message in queue
 * 
 * The A7670 module needs a delay between one published message and another
 * This function publishes a message from the queue every 500 ms
 */
void A7670_MQTT_PubQueueMessages()
{
	if(A7670_MQTT_QueueIsEmpty(&mqtt_send.ring_buffer) == 0)
	{
		if((HAL_GetTick() - mqtt_send.start_tick) > 500)
		{
			uint8_t *tail = &mqtt_send.ring_buffer.tail;
			char* topic = mqtt_send.message[*tail].topic;
			char* payload = mqtt_send.message[*tail].payload;

			A7670_MQTT_PublishHandler(topic, payload);

			mqtt_send.start_tick = HAL_GetTick();

			*tail = ((*tail) + 1) % MAX_MQTT_SEND_MESSAGE;

			mqtt_send.ring_buffer.count--;

		}
	}
}

/**
 * @brief Handles the queue of incoming messages and messages that need to be sent
 * 
 * This is an important function, where we handle both sending and receiving MQTT messages.
 * IMPORTANT: you must ensure that the function is always called in the loop to ensure correct functioning of MQTT
 */

CMD_Status A7670_MQTT_Handler()
{
	A7670_MQTT_PubQueueMessages();
	A7670_MQTT_ReadNewMessages();

	return CMD_OK;
}



