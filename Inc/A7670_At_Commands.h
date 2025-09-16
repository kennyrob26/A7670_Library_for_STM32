/*
 * A7670_At_Commands.h
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_AT_COMMANDS_H_
#define INC_A7670_AT_COMMANDS_H_

#include "AT_decoder.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum
{
	CMD_ERROR = 0,
	CMD_OK    = 1
} CMD_Status;

typedef enum AT_ResponseType
{
    AT_RT_Echo_Command  = 0,
    AT_RT_Response      = 1,
    AT_RT_MQTT_Response = 2,
    AT_RT_NULL          = 3
}AT_ResponseType;

typedef struct MQTT_RESPONSE
{
	char last_message[BUFFER_LENGTH];
    uint8_t client_id;
    char topic[20];
    uint8_t topic_lentgth;
    char payload[20];
    uint8_t payload_length;
    uint8_t end;
}MQTT_RESPONSE;

typedef enum
{
	GNSS_OK    		  = 0,
	GNSS_PWR   		  = 1,
	GNSS_CAGPS 		  = 2,
	GNSS_PORTSWITCH   = 3,
	GNSS_RESET_MODULE = 4
}Connect_GNSS_state;

typedef struct NEMA
{
	char *command;
    char *lat;
    char *N_or_S;
    char *log;
    char *E_or_W;
    char *date;
    char *utc_time;
    char *alt;
    char *speed;
    char *course;
}NEMA;


//  GNSS
typedef struct GNSS
{
    float latitude;
    float longitude;
    char latitude_longitude[30];

}GNSS;

CMD_Status A7670_CMD_Creset();
CMD_Status A7670_GPS_Init();
CMD_Status A7670_GPS_CMD_CGNSSPWR();
CMD_Status A7670_GPS_CMD_CAGPS();
CMD_Status A7670_GPS_CMD_CGNSSPORTSWITCH();
CMD_Status A7670_GPS_CMD_CGPSINFO();

CMD_Status A7670_Pocess_Buffer();

void readNEMA(char *dataNEMA);
void nextValueNema(char **value, char *previous_value);
void convertNemaToGNSS(GNSS *gnss, NEMA nema);
float calculateLatitude(char *latitude_nema, char *N_or_S);
float calculateLongitude(char *longitude_nema, char *E_or_W);


#endif /* INC_A7670_AT_COMMANDS_H_ */
