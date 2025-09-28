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

typedef struct NEMA
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
}NEMA;

extern NEMA nema;


//  GNSS
typedef struct GNSS
{
    char latitude[20];
    char longitude[20];
    char latitude_longitude[30];
    float speed_kmh;

}GNSS;

extern GNSS gnss;

#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_TYPES_GNSS_H_ */
