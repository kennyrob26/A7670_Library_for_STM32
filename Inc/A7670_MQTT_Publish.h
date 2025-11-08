/*
 * A7670_MQTT_Publish.h
 *
 *  Created on: Sep 24, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_MQTT_PUBLISH_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_MQTT_PUBLISH_H_

#include "A7670_Commands_MQTT.h"
#include "A7670_Commands_GNSS.h"


CMD_Status A7670_MQTT_Publish_Latitude(const char* topic_latitude);
CMD_Status A7670_MQTT_Publish_Longitude(const char* topic_longitude);
CMD_Status A7670_MQTT_Publish_LatLon(const char* topic_LatLon);
CMD_Status A7670_MQTT_Publish_UTC_DateTime(const char* topic_utc_date_time);
CMD_Status A7670_MQTT_Publish_Speed(const char* topic_speed);

#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_MQTT_PUBLISH_H_ */
