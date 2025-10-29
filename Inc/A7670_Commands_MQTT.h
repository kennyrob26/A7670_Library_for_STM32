/*
 * A7670_Commands_MQTT.h
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_

#include <A7670_Types_MQTT.h>
#include "A7670_At_Commands.h"
//#include "A7670_Commands_MQTT.h"




extern void (*MQTT_Callback_Response)(MQTT_RESPONSE mqtt_resp);

CMD_Status A7670_MQTT_TesteSSL();

CMD_Status A7670_MQTT_Register_Callback_Response(void (*callback_function)(MQTT_RESPONSE mqtt_resp));

CMD_Status A7670_MQTT_setClient(uint8_t client_id, char *client_name);
CMD_Status A7670_MQTT_SetBroker(char *broker_adress, uint8_t keep_alive, MQTT_Clear_Session clear_session, MQTT_QoS QoS);
CMD_Status A7670_MQTT_SetAuth(char* username, char* password);

MQTT_Status A7670_MQTT_Connect(MQTT_Auto_Reconnect state);
MQTT_Broker_State A7670_MQTT_CheckBrokerConnection();
CMD_Status A7670_MQTT_SetAutoReconnect(MQTT_Auto_Reconnect state);
CMD_Status A7670_MQTT_Disconnect(void);
CMD_Status A7670_MQTT_SubscribeTopic(char* topic);
void A7670_MQTT_ReadNewMessages();

CMD_Status A7670_MQTT_CMD_Start(void);
CMD_Status A7670_MQTT_CMD_Stop(void);
CMD_Status A7670_MQTT_CMD_AcquireClient(void);
MQTT_Client_State A7670_MQTT_CheckAcquireClient(void);
CMD_Status A7670_MQTT_CMD_ReleaseClient(void);
void A7670_MQTT_SetSSL(MQTT_SSL_State ssl_status);
CMD_Status A7670_MQTT_CMD_SSLConfig(void);
MQTT_Status A7670_MQTT_CMD_Connect(void);
CMD_Status A7670_MQTT_CMD_Disconnect(void);

CMD_Status A7670_MQTT_CMD_Pub_Topic(void);
CMD_Status A7670_MQTT_CMD_Payload(void);
MQTT_Status A7670_MQTT_CMD_Publish(void);
CMD_Status A7670_MQTT_CMD_SubTopic(void);
CMD_Status A7670_MQTT_CMD_ConfirmSubTopic(void);
MQTT_Status A7670_MQTT_PublishHandler(const char* topic, const char* message_payload);
CMD_Status A7670_MQTT_ResponseHandler();

uint8_t A7670_MQTT_QueueIsFull(RingBuffer *ring_buffer);
uint8_t A7670_MQTT_QueueIsEmpty(RingBuffer *ring_buffer);
CMD_Status A7670_MQTT_QueuePushMessage(char *mqttMessage);
CMD_Status A7670_MQTT_QueuePopMessage();

CMD_Status A7670_MQTT_PublishMessage(const char* topic, const char* payload);
void A7670_MQTT_PubQueueMessages();

CMD_Status A7670_MQTT_Handler();
MQTT_Status A7670_MQTT_TranslateErrorCode(uint8_t code_error);
MQTT_Status A7670_MQTT_CheckErrorCode();







#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_ */
