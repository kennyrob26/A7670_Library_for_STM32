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
	return (A7670_MQTT_PublishMessage(topic_latitude, latitude));

}

CMD_Status A7670_MQTT_Publish_Longitude(const char* topic_longitude)
{
	char longitude[20];
	sprintf(longitude, "%.6f", gnss.longitude);
	return(A7670_MQTT_PublishMessage(topic_longitude, longitude));
}

CMD_Status A7670_MQTT_Publish_LatLon(const char* topic_LatLon)
{
	return(A7670_MQTT_PublishMessage(topic_LatLon, gnss.latitude_longitude));
}

CMD_Status A7670_MQTT_Publish_UTC_DateTime(const char* topic_utc_date_time)
{
	return(A7670_MQTT_PublishMessage(topic_utc_date_time, gnss.date_time_utc));
}

CMD_Status A7670_MQTT_Publish_Speed(const char* topic_speed)
{
	char speed[5];
	sprintf(speed, "%.2f", gnss.speed);
	return(A7670_MQTT_PublishMessage(topic_speed, speed));
}
