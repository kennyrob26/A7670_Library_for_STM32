/*
 * A7670_Types_GNSS.h
 *
 *  Created on: Sep 18, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_TYPES_GNSS_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_TYPES_GNSS_H_

typedef enum
{
	GNSS_OK    		  = 0,
	GNSS_PWR   		  = 1,
	GNSS_CAGPS 		  = 2,
	GNSS_PORTSWITCH   = 3,
	GNSS_RESET_MODULE = 4
}Connect_GNSS_state;

typedef struct NMEA
{
	char command[100];
    char *lat;
    char *N_or_S;
    char *log;
    char *E_or_W;
    char *date;
    char *utc_time;
    char *alt;
    char *speed;
    char *course;
}NMEA;

extern NMEA nmea;


typedef enum
{
    GNSS_FIX_NOT_FIXED = 0,
    GNSS_FIX_2D_FIXED  = 2,
    GNSS_FIX_3D_FIXED  = 3
}GNSS_Fix;

typedef struct GNSS_INFO
{
    char response[100];
    GNSS_Fix mode;
    uint8_t GPS_Satelites;
    uint8_t GLONASS_Satelites;
    uint8_t BEIDOU_Satelites;
    float lat;
    char N_or_S;
    float log;
    char E_or_W;
    char date[15];
    char utc_time[15];
    float alt;
    float speed;
    float course;
    float PDOP;
    float HDOP;
    float VDOP;
}GNSS_INFO;

extern GNSS_INFO gnss_info;

//  GNSS
typedef struct GNSS
{
    float latitude;
    float longitude;
    char latitude_longitude[30];
    char date[14];
    char utc_time[12];
    char utc_date_time[25];
    char altitude[15];
    float speed_kmh;

}GNSS;

extern GNSS gnss;

#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_TYPES_GNSS_H_ */
