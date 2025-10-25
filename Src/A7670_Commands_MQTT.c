/*
 * A7670_Commands_MQTT.c
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */
#include "A7670_Commands_MQTT.h"
#include "A7670_Commands_SIM.h"

MqttRingBufferSend mqtt_send;
MQTT mqtt;
MQTT_RESPONSE mqtt_resp;


void (*MQTT_Callback_Response)(MQTT_RESPONSE mqtt_resp);

/**
 * @brief Set MQTT Client
 * @param client_id It is the client ID and is unique for each client
 * @param client_name It is the client name
 * @return CMD_Status
 */
CMD_Status A7670_MQTT_setClient(uint8_t client_id, char *client_name)
{
	if(client_name == NULL || strcmp(client_name, "") == 0)
		return CMD_ERROR;

	mqtt.client.id 	 = client_id;
	strcpy(mqtt.client.name, client_name);

	return CMD_OK;
}



/**
 * @brief Set MQTT Broker
 * @param broker_adress The broker MQTT adress (tcp://broker.com:1883)
 * @param keep_alive The keep alive of connection in seconds (use 60s with default)
 * @param clear_session uses 1 for a clear connection
 * @param QoS Defines QoS values, 0 at 2
 * @return CMD_Status
 */
CMD_Status A7670_MQTT_SetBroker(char *broker_adress, uint8_t keep_alive, uint8_t clear_session, uint8_t QoS)
{
	if(broker_adress == NULL || strcmp(broker_adress, "") == 0)
		return CMD_ERROR;

	strcpy(mqtt.broker.adress, broker_adress);
	mqtt.broker.kepp_alive = keep_alive;
	mqtt.broker.clear_session = clear_session;
	mqtt.broker.QoS = QoS;

	return CMD_OK;
}

/**
 * @brief Set MQTT Username and Password
 * @param username auth username
 * @param password auth password
 * @return CMD_Status
 */
CMD_Status A7670_MQTT_SetAuth(char* username, char* password)
{
	if(username == NULL || strcmp(username, "") == 0)
		return CMD_ERROR;

	if(password == NULL)
		return CMD_ERROR;

	strcpy(mqtt.auth.username, username);
	strcpy(mqtt.auth.password, password);

	mqtt.auth_state = MQTT_AUTH_ENABLE;

	return CMD_OK;
 }

/**
 * @brief connects to the MQTT broker
 *
 * A state machine that connects to the MQTT broker
 * 

 * @return CMD_Status
 * @retval CMD_OK if it was possible to connect correctly to the broker
 * @retval CMD_ERROR if we are unable to connect to the broker
 */
MQTT_Status A7670_MQTT_Connect(MQTT_Auto_Reconnect state)
{
	A7670_MQTT_SetAutoReconnect(state);
	MQTT_Status response;
	mqtt.broker_state = MQTT_BROKER_CONNECTING;
	//MQTT_Connect_State mqtt_connect_state = MQTT_START;
	MQTT_Connect_State mqtt_connect_state = MQTT_CHECK_NETWORK;

	while(mqtt_connect_state != MQTT_CONNECT_OK)
	{
		switch (mqtt_connect_state)
		{
			case MQTT_CHECK_NETWORK:
				if(A7670_SIM_Check_Network() == CMD_OK)
				{
					mqtt_connect_state = MQTT_START;
				}
				else
				{
					response = MQTT_CON_ERROR_NETWORK;
					mqtt_connect_state = MQTT_CONNECT_ERROR;
				}
			break;
			case MQTT_START:
				if(A7670_MQTT_CMD_Start() == CMD_OK)
				{
					mqtt_connect_state = MQTT_ACCQ;
				}
				else
				{
					response =  MQTT_CON_ERROR_STARTING_MODULE;
					mqtt_connect_state = MQTT_CONNECT_ERROR;
				}
			break;
			case MQTT_ACCQ:
				if(A7670_MQTT_CMD_AcquireClient() == CMD_OK)
				{
					if(mqtt.ssl_state == MQTT_SSL_ENABLE)
						mqtt_connect_state = MQTT_SSL_CONFIG;
					else
						mqtt_connect_state = MQTT_CONNECT;
				}
				else
				{
					response = MQTT_CON_ERROR_CLIENT;
					mqtt_connect_state = MQTT_CONNECT_ERROR;
				}
			break;
			case MQTT_SSL_CONFIG:
				if(A7670_MQTT_CMD_SSLConfig() == CMD_OK)
				{
					mqtt_connect_state = MQTT_CONNECT;
				}
				else
				{
					response = MQTT_CON_ERROR_SSL;
					mqtt_connect_state = MQTT_CONNECT_ERROR;
				}
			break;
			case MQTT_CONNECT:
				if(A7670_MQTT_CMD_Connect() == MQTT_CON_OK)
				{
					mqtt.broker_state = MQTT_BROKER_CONNECTED;
					mqtt_connect_state = MQTT_CONNECT_OK;
				}
				else
				{
					response = MQTT_CON_ERROR_BROKER;
					mqtt_connect_state = MQTT_CONNECT_ERROR;
				}
			break;
			case MQTT_CONNECT_ERROR:
				mqtt.broker_state = MQTT_BROKER_DISCONNECT;
				return response;
			default:
				return MQTT_CON_ERROR;
				break;

		}
	}
	return MQTT_CON_OK;
}

MQTT_Broker_State A7670_MQTT_CheckBrokerConnection()
{
	const char command[] = "AT+CMQTTCONNECT?";
	const char expected_response[] = "+CMQTTCONNECT: ";

	if(AT_sendCommand(command, expected_response, 2000) == AT_OK)
	{
		char response[150];
		strcpy(response, (char*)at.response);
		char* ptr_id;
		char* ptr_adress;
		char* ptr_end;

		ptr_id = strstr(response, ":");
		ptr_id += 2;

		ptr_adress = (ptr_id + 1);
		if(*ptr_adress == ',')
		{
			*ptr_adress = '\0';
			(ptr_adress += 2);
			ptr_end = ptr_adress;
			ptr_end = strstr(ptr_adress, "\"");
			*ptr_end = '\0';

			if(strcmp(mqtt.broker.adress, ptr_adress) == 0)
			{
				mqtt.broker_state = MQTT_BROKER_CONNECTED;
				return mqtt.broker_state;
			}
		}
	}
	mqtt.broker_state = MQTT_BROKER_DISCONNECT;
	return mqtt.broker_state;
}

CMD_Status A7670_MQTT_SetAutoReconnect(MQTT_Auto_Reconnect state)
{
	mqtt.auto_reconect = state;

	return CMD_OK;
}

CMD_Status A7670_MQTT_Disconnect(void)
{
	mqtt.broker_state = MQTT_BROKER_DISCONNECTING;
	MQTT_Disconnect_State mqtt_disconnect_state = MQTT_DISC_DISCONNECT;

	while(mqtt_disconnect_state != MQTT_DISC_DISCONNECT_OK)
	{
		switch (mqtt_disconnect_state) {
			case MQTT_DISC_DISCONNECT:
				if(A7670_MQTT_CMD_Disconnect() == CMD_OK)
					mqtt_disconnect_state = MQTT_DISC_REALESE_CLIENT;
			break;
			case MQTT_DISC_CHECK_ACQUIRE_CLIENT:
				MQTT_Client_State client_status = A7670_MQTT_CheckAcquireClient();
				if(client_status == MQTT_CLIENT_ACQUIRED)
				{
					mqtt_disconnect_state = MQTT_DISC_REALESE_CLIENT;
				}
				else if(client_status == MQTT_CLIENT_NOT_ACQUIRED)
				{
					mqtt_disconnect_state = MQTT_DISC_STOP;
				}
				else
					mqtt_disconnect_state = MQTT_DISC_ERROR;
			break;
			case MQTT_DISC_REALESE_CLIENT:
				if(A7670_MQTT_CMD_ReleaseClient() == CMD_OK)
					mqtt_disconnect_state = MQTT_DISC_STOP;
			break;
			case MQTT_DISC_STOP:
				if(A7670_MQTT_CMD_Stop() == CMD_OK)
					mqtt_disconnect_state = MQTT_DISC_DISCONNECT_OK;
			break;
			case MQTT_DISC_ERROR:
				return CMD_ERROR;
			default:
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
	const char command[] 			= "AT+CMQTTSTART";
	const char expected_response[]  = "CMQTTSTART: 0";

	if(AT_sendCommand(command, expected_response, 9000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

/**
 * @brief Stop MQTT
 *
 * This command stop MQTT communication
 * uses AT+CMQTTSTOP
 *
 * must be used after AT+CMQTTDISC and AT+CMQTTREL
 *
 */
CMD_Status A7670_MQTT_CMD_Stop(void)
{
	char command[] = "AT+CMQTTSTOP";
	if(AT_sendCommand(command, "OK", 500))
		return CMD_OK;

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
	char command[50];
	sprintf(command, "%s%d,\"%s\"", "AT+CMQTTACCQ=", mqtt.client.id, mqtt.client.name);
	const char expected_response[] = "OK";

	if(AT_sendCommand(command, expected_response, 5000) ==  AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

MQTT_Client_State A7670_MQTT_CheckAcquireClient(void)
{
    char response[150];
    strcpy(response, (char*)at.response);
    char* ptr_id;
    char* ptr_name;
    char* ptr_end;

    ptr_id = strstr(response, ":");
    ptr_id += 2;

    ptr_name = (ptr_id + 1);
    *ptr_name = '\0';
    (ptr_name+=2);
    if(*ptr_name != '\"')
    {
        ptr_end = strstr(ptr_name, "\"");
        *ptr_end = '\0';
    }
    else
    {
    	*ptr_name = '\0';
    }

    if(strcmp(ptr_name, mqtt.client.name) == 0)
    	return MQTT_CLIENT_ACQUIRED;
    else if(strcmp(ptr_name, "") == 0)
    	return MQTT_CLIENT_NOT_ACQUIRED;

    return MQTT_CLIENT_ERROR;

}
/**
 * @brief remove all settings from the current client
 *
 * This command is used when we want to end communication with the MQTT broker, or reset the client settings
 * Must be used before AT+CMQTTSTOP and after AT+CMQTTDISC
 */
CMD_Status A7670_MQTT_CMD_ReleaseClient(void)
{
	char command[20];
	sprintf(command, "AT+CMQTTREL=%d", mqtt.client.id);
	const char expected_response[] = "OK";

	if(AT_sendCommand(command, expected_response, 500))
		return CMD_OK;
	else
		return CMD_ERROR;
}

void A7670_MQTT_SetSSL(MQTT_SSL_State ssl_status)
{
	mqtt.ssl_state = ssl_status;
}

CMD_Status A7670_MQTT_CMD_SSLConfig(void)
{
	char command[30] = "AT+CMQTTSSLCFG=0,0";
	if(AT_sendCommand(command, "OK", 500) == AT_OK)
	{
		strcpy(command, "AT+CMQTTCFG=\"argtopic\",0,1,1");
		if(AT_sendCommand(command, "OK", 1000) == AT_OK)
		{
			return CMD_OK;
		}
	}
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
MQTT_Status A7670_MQTT_CMD_Connect(void)
{
	char command[150];
	if(mqtt.auth_state == MQTT_AUTH_DISABLE)
	{
		sprintf(command, "%s%d,\"%s\",%d,%d", "AT+CMQTTCONNECT=", mqtt.client.id, mqtt.broker.adress, mqtt.broker.kepp_alive, mqtt.broker.clear_session);
	}
	else if(mqtt.auth_state == MQTT_AUTH_ENABLE)
	{
		sprintf(command, "%s%d,\"%s\",%d,%d,\"%s\",\"%s\"", "AT+CMQTTCONNECT=", mqtt.client.id, mqtt.broker.adress, mqtt.broker.kepp_alive, mqtt.broker.clear_session, mqtt.auth.username, mqtt.auth.password);
	}

	const char expected_response[] = "CMQTTCONNECT: ";

	if(AT_sendCommand(command, expected_response , 10000) == AT_OK)
	{
		MQTT_Status status = A7670_MQTT_CheckErrorCode();
		return status;
	}
	else
		return MQTT_CON_ERROR;
}

MQTT_Status A7670_MQTT_CheckErrorCode()
{
    char response[30];
    strcpy(response, (char*)at.response);
    char* ptr_id;
    char* ptr_error;
    char* end;

    ptr_id = strstr(response, ": ");
    ptr_id += 2;
    ptr_error = strstr(ptr_id, ",");
    *ptr_error = '\0';
    ptr_error++;
    end = strstr(ptr_error, "\r");
    *end = '\0';

    uint8_t id     = atoi(ptr_id);
    uint8_t error  = atoi(ptr_error);

    return(A7670_MQTT_TranslateErrorCode(error));
}

/**
 * @brief Disconnet from the broker MQTT
 *
 * This command is used befor AT+CMQTTREL and AT+CMQTTSTOP
 */

CMD_Status A7670_MQTT_CMD_Disconnect(void)
{
	char command[30];
	sprintf(command, "AT+CMQTTDISC=%d,%d", mqtt.client.id, 60);
	const char expected_response[] = "+CMQTTDISC: 0,0";

	if(AT_sendCommand(command, expected_response , 2000) == AT_OK)
	{
		mqtt.broker_state = MQTT_BROKER_DISCONNECT;
		return CMD_OK;
	}
	else
		return CMD_ERROR;
}


MQTT_Status A7670_MQTT_PublishHandler(const char* topic, const char* message_payload)
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
				return A7670_MQTT_CMD_Publish();
			break;
			case MSG_RESET_MODULE:
				//mqtt.broker_state = MQTT_BROKER_DISCONNECT;
				return MQTT_CON_ERROR_INVALID_VALUE;
			default:
				return MQTT_CON_ERROR_INVALID_VALUE;
		}
	}

	return MQTT_CON_ERROR;

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
	char command[50];
	sprintf(command, "%s%d,%d", "AT+CMQTTTOPIC=", mqtt.client.id, topic_length);

	if(AT_sendCommand(command, ">", 30) == AT_OK)
	{
		strcpy(command, mqtt.message.topic);
		if(AT_sendCommand(command, "OK", 30) == AT_OK)
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
	char command[100];
	sprintf(command, "%s%d,%d", "AT+CMQTTPAYLOAD=", mqtt.client.id, payload_length);

	if(AT_sendCommand(command, ">", 30) == AT_OK)
	{
		strcpy(command, mqtt.message.payload);
		if(AT_sendCommand(command, "OK", 30) == AT_OK)
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

MQTT_Status A7670_MQTT_CMD_Publish(void)
{
	char command[50];
	sprintf(command, "%s%d,%d,%d", "AT+CMQTTPUB=", mqtt.client.id, mqtt.broker.QoS, mqtt.broker.kepp_alive);
	if(AT_sendCommand(command, "CMQTTPUB: ", 3000) == AT_OK)
	{
		return A7670_MQTT_CheckErrorCode();
	}
	else
		return MQTT_CON_ERROR_TIMEOUT;
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

CMD_Status A7670_MQTT_SubscribeTopic(char* topic)
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
	char command[50];
	sprintf(command, "%s%d,%d,%d", "AT+CMQTTSUBTOPIC=", mqtt.client.id, topic_length, mqtt.broker.QoS);
	if(AT_sendCommand(command, "", 0) == AT_OK)
	{
		HAL_Delay(50);
		strcpy(command, mqtt.message.topic);
		if(AT_sendCommand(command, "OK", 1500) == AT_OK)
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
	const char command[] = "AT+CMQTTSUB=0";
	if(AT_sendCommand(command, "CMQTTSUB: 0,0", 1500) == AT_OK)
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
    if(A7670_MQTT_QueueIsFull((RingBuffer*)&mqtt_resp.queue.ring_buffer) == 0)
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
    if(A7670_MQTT_QueueIsEmpty((RingBuffer*)&mqtt_resp.queue.ring_buffer) == 0)
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
	while(A7670_MQTT_QueueIsEmpty((RingBuffer*)&mqtt_resp.queue.ring_buffer) != 1 && ((HAL_GetTick() - start_tick) < 100))
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

			MQTT_Status response = A7670_MQTT_PublishHandler(topic, payload);

			mqtt_send.start_tick = HAL_GetTick();

			*tail = ((*tail) + 1) % MAX_MQTT_SEND_MESSAGE;

			mqtt_send.ring_buffer.count--;

			if(response != MQTT_CON_OK &&
			   response != MQTT_CON_ERROR_INVALID_VALUE &&
			   response != MQTT_CON_ERROR_TIMEOUT)
			{
				mqtt.broker_state = MQTT_BROKER_DISCONNECT;
			}
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
	static uint32_t time_tick = 0;
	if((HAL_GetTick() - time_tick) > 5000)
	{
		//A7670_MQTT_CheckBrokerConnection();
		time_tick = HAL_GetTick();
	}

	if(mqtt.broker_state == MQTT_BROKER_CONNECTED)
	{
		A7670_MQTT_PubQueueMessages();
		A7670_MQTT_ReadNewMessages();
		return CMD_OK;
	}
	else if(mqtt.auto_reconect == MQTT_RECONNECT_ENABLE)
	{
		if(A7670_MQTT_Disconnect() == CMD_OK)
		{
			MQTT_Status response = A7670_MQTT_Connect(MQTT_RECONNECT_ENABLE);
			if(response == MQTT_CON_OK)
				return CMD_OK;
		}
	}
	return CMD_ERROR;
}

//6, 12, 13, 18, 22


MQTT_Status A7670_MQTT_TranslateErrorCode(uint8_t code_error)
{

	switch (code_error) {
		case 0:
			return MQTT_CON_OK;
	/*========= -- NETWORK ERROR -- =========*/
		case 3:  				//sock connect fail
		case 4:					//sock create fail
		case 5: 				//sock close fail
		case 6:					//message receive fail
		case 7:					//network open fail
		case 8:					//network close fail
		case 9:					//network not opened
		case 11:				//no connection
		case 15:				//require connection fail
		case 16:				//sock sending fail
		case 23:				//network is opened
		case 24:				//packet fail
		case 25:				//DNS error

			return MQTT_CON_ERROR_NETWORK;

	/*========= -- CLIENT ERROR -- =========*/
		case 10:				//client index error
		case 14:				//client is busy
		case 19: 				//client is used
		case 20: 				//client not acquired
		case 21: 				//client not released
		case 34:				//Open session failed

			return MQTT_CON_ERROR_CLIENT;

	/*========= -- BROKER ERROR -- =========*/
		case 26:				//socket is closed by server
		case 27:				//connection refused: unaccepted protocol version
		case 28:				//connection refused: identifier rejected
		case 29:				//connection refused: server unavailable

			return MQTT_CON_ERROR_BROKER;

	/*========= -- AUTH ERROR -- =========*/
		case 30:				//connection refused: bad user name or password
		case 31:				//connection refused: not authorized
			return MQTT_CON_ERROR_AUTH;

	/*========= -- SSL ERROR -- =========*/
		case 32: 				//handshake fail
		case 33:				//not set certificate
			return MQTT_CON_ERROR_SSL;

	/*========= -- INVALID VALUE ERROR -- =========*/
		case 12:    			//invalid parameter
		case 13:    			//not supported operation
		case 18:    			//topic is empty
		case 22:    			//length out of range
			return MQTT_CON_ERROR_INVALID_VALUE;

	/*========= -- TIMEOUT ERROR -- =========*/
		case 17:				//timeout
			return MQTT_CON_ERROR_TIMEOUT;
		default:
			return MQTT_CON_ERROR;
	}
}

