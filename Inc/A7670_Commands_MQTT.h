/*
 * A7670_Commands_MQTT.h
 *
 *  Created on: Sep 10, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_

#include "A7670_At_Commands.h"
#include "A7670_MQTT_Types.h"
//#include "A7670_Commands_MQTT.h"




extern void (*MQTT_Callback_Response)(MQTT_RESPONSE mqtt_resp);

CMD_Status A7670_MQTT_Register_Callback_Response(void (*callback_function)(MQTT_RESPONSE mqtt_resp));

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
CMD_Status A7670_MQTT_Process_response();





#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_MQTT_H_ */
