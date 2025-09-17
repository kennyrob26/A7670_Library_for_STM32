/*
 * A7670_Commands_GNSS.h
 *
 *  Created on: Sep 15, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_GNSS_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_GNSS_H_

#include "AT_decoder.h"

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


CMD_Status A7670_GPS_Init();
CMD_Status A7670_GPS_CMD_CGNSSPWR();
CMD_Status A7670_GPS_CMD_CAGPS();
CMD_Status A7670_GPS_CMD_CGNSSPORTSWITCH();
CMD_Status A7670_GPS_CMD_CGPSINFO();

void readNEMA(char *dataNEMA);
void nextValueNema(char **value, char *previous_value);
void convertNemaToGNSS(GNSS *gnss, NEMA nema);
float calculateLatitude(char *latitude_nema, char *N_or_S);
float calculateLongitude(char *longitude_nema, char *E_or_W);


#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_GNSS_H_ */
