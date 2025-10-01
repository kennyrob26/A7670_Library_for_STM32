/*
 * A7670_MQTT_Publish.c
 *
 *  Created on: Sep 24, 2025
 *      Author: kenny
 */


#include "A7670_MQTT_Publish.h"


CMD_Status A7670_MQTT_Publish_Latitude(const char* topic_latitude)
{
	char latitude[20];
	sprintf(latitude, "%.6f", gnss.latitude);
	//return (A7670_MQTT_PublishMessage(topic_latitude, latitude));
	A7670_MQTT_PushPubMessage(topic_latitude, latitude);
	return CMD_OK;
}
CMD_Status A7670_MQTT_Publish_Longitude(const char* topic_longitude)
{
	char longitude[20];
	sprintf(longitude, "%.6f", gnss.longitude);
	A7670_MQTT_PushPubMessage(topic_longitude, longitude);
	return CMD_OK;
}
CMD_Status A7670_MQTT_Publish_LatLon(const char* topic_LatLon)
{
	A7670_MQTT_PushPubMessage(topic_LatLon, gnss.latitude_longitude);
	return CMD_OK;
}

CMD_Status A7670_MQTT_Publish_Speed(const char* topic_speed)
{
	char speed[5];
	sprintf(speed, "%.2f", gnss.speed_kmh);
	A7670_MQTT_PushPubMessage(topic_speed, speed);

	return CMD_OK;
}
