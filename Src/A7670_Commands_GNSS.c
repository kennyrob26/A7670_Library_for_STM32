/*
 * A7670_Commands_GNSS.c
 *
 *  Created on: Sep 15, 2025
 *      Author: kenny
 */



#include "A7670_Commands_GNSS.h"
#include <stdint.h>
#include <stdio.h>
#include "string.h"
#include <stdlib.h>

GNSS_INFO gnss;

//CMD_Status processAtCommand();


/*================= -- Static Functions -- =====================*/


/**
 * @brief Private Function -> Calcule Latitude
 * 
 *	 A private function that calculates Latitude with base GNSS data.
 *	 Converts degress/minutes Latitude to decimal Latitude
 */
static float calculateLatitude(char *latitude_nema, char N_or_S)
{
    if(strlen(latitude_nema) && (N_or_S == 'N' || N_or_S == 'S'))
    {
        char lat_degress_buffer[3];
        char lat_minutes_buffer[15];

        strncpy(lat_degress_buffer, latitude_nema, 2);
        strcpy(lat_minutes_buffer, latitude_nema + 2);

        uint16_t lat_degress = atoi(lat_degress_buffer);
        float lat_minutes = atof(lat_minutes_buffer);

        float latitude = lat_degress + (lat_minutes / 60);

        if(N_or_S == 'S')
            latitude = latitude * -1;

        return latitude;
    }
    return 0;

}

/**
 * @brief Private Function -> Calcule Longitude
 * 
 *	 A private function that calculates Longitude with base GNSS data.
 *	 Converts degress/minutes Longitude to decimal Longitude
 */
static float calculateLongitude(char *longitude_nema, char E_or_W)
{
    if(strlen(longitude_nema) && (E_or_W == 'E' || E_or_W == 'W'))
    {
        char log_degress_buffer[4];
        char log_minutes_buffer[15];

        strncpy(log_degress_buffer, longitude_nema, 3);
        strcpy(log_minutes_buffer, longitude_nema + 3);

        uint16_t log_degress =  atoi(log_degress_buffer);
        float log_minutes    =  atof(log_minutes_buffer);

        float longitude = log_degress + (log_minutes / 60);

        if(E_or_W == 'W')
            longitude = (longitude * -1);

        return longitude;
    }
    return(0);
}

/**
 * @brief Private Function -> Calcule speed in Km/H
 * 
 *	 A private function that calculates speed with base GNSS data.
 *	 Converts knots to km/h
 */
static float calculateSpeedKmh(float speed_kont)
{

	if((speed_kont != 0) && (speed_kont < 200))
	{
		const float const_knot_to_kmh = 1.825;
		float speed_kmh = speed_kont * const_knot_to_kmh;
		return speed_kmh;
	}
	else
		return 0;
}


// Example "CGPSINFO: 2125.22430,S,05003.66783,W,060925,172333.00,424.5,0.000,198.11";
static void NmeaUtcDateTime(char* date, char* time)
{
    char date_utc[20];
    char time_utc[15];

	char day[3];
	char mounth[3];
	char year[3];

    sscanf(date, "%2c%2c%2c", day, mounth, year);
    sprintf(date_utc, "20%s-%s-%s", year, mounth, day);

	char hour[3];
	char minute[3];
	char second[3];
    char decimal_second;

    sscanf(time, "%2s%2s%2s.%1c", hour, minute, second, &decimal_second);
    sprintf(time_utc, "%s:%s:%s.%1c", hour, minute, second, decimal_second);

    sprintf(gnss.date_time_utc, "%sT%s", date_utc, time_utc);
}


static void readGNSSINFO(char *gnss_buffer)
{
	char *remove_echo;
	remove_echo = strstr(gnss_buffer, "+CGNSSINFO: ");

    strcpy(gnss.response, gnss_buffer);

	if(strcmp(gnss.response, "+CGNSSINFO: ,,,,,,,,") == 0)
	{
		gnss.mode = GNSS_FIX_NOT_FIXED;
	}
	else
	{
		uint8_t count_values = 0;
		uint8_t mode, gps, glonass, beidou;
		float  altitude, speed, course, pdop, hdop, vdop;
		char N_or_S, E_or_W;
		char latitude[13];
		char longitude[14];
		char date[7];
		char time[9];

		//const char filter[] = "%*[^+]+CGNSSINFO: %hhu,%hhu,%hhu,%hhu,%11s,%c,%12s,%c,%6s,%8s,%f,%f,%f,%f,%f,%f";
		const char filter[] = "+CGNSSINFO: %hhu,%hhu,%hhu,%hhu,%11s,%c,%12s,%c,%6s,%8s,%f,%f,%f,%f,%f,%f";
		count_values = sscanf(gnss.response, filter , &mode, &gps, &glonass, &beidou, latitude,&N_or_S,longitude, &E_or_W, date, time, &altitude, &speed, &course, &pdop, &hdop, &vdop);

		if(count_values == 16)
		{
			gnss.mode = mode;
			if(gnss.mode != GNSS_FIX_NOT_FIXED)
			{
				gnss.GPS_Satelites     = gps;
				gnss.GLONASS_Satelites = glonass;
				gnss.BEIDOU_Satelites  = beidou;
				gnss.latitude          = calculateLatitude(latitude, N_or_S);
				gnss.longitude         = calculateLongitude(longitude, E_or_W);
				sprintf(gnss.latitude_longitude, "%f, %f", gnss.latitude, gnss.longitude);
				NmeaUtcDateTime(date, time);
				gnss.alt               = altitude;
				gnss.speed             = calculateSpeedKmh(speed);
				gnss.course            = course;
				gnss.PDOP              = pdop;
				gnss.HDOP              = hdop;
				gnss.VDOP              = vdop;
			}

		}
	}
}

static void readGPSINFO(char *gnss_buffer)
{
	char *remove_echo;
	remove_echo = strstr(gnss_buffer, "+CGPSINFO: ");
	strcpy(gnss.response, gnss_buffer);

	if(strcmp(gnss.response, "+CGPSINFO: ,,,,,,,,") == 0)
	{
		gnss.mode = GNSS_FIX_NOT_FIXED;
	}
	else
	{
		gnss.mode = GNSS_FIX_FIXED;

		uint8_t count_values = 0;

		float  altitude, speed, course;
		char N_or_S, E_or_W;
		char latitude[13];
		char longitude[14];
		char date[7];
		char time[9];

		//const char filter[] = "AT+CGPSINFO\r\r\n+CGPSINFO: %10s,%c,%11s,%c,%6s,%9s,%f,%f,%f";
		const char filter[] = "+CGPSINFO: %10s,%c,%11s,%c,%6s,%9s,%f,%f,%f";
		count_values = sscanf(gnss.response, filter , latitude,&N_or_S,longitude, &E_or_W, date, time, &altitude, &speed, &course);

		//if(count_values == 9)
		//{
			gnss.latitude          = calculateLatitude(latitude, N_or_S);
			gnss.longitude         = calculateLongitude(longitude, E_or_W);
			sprintf(gnss.latitude_longitude, "%f, %f", gnss.latitude, gnss.longitude);
			NmeaUtcDateTime(date, time);
			gnss.alt               = altitude;
			gnss.speed             = calculateSpeedKmh(speed);
			gnss.course            = course;

		//}
	}
}


/**
 * @brief Start to GNSS
 * 
 * 	A State Machine that start GNSS
 * 	The State machine ollow the sequence:
 * 	
 * 	1. PWR -> Start the GNSS Module
 * 	2. AGPS -> tries to connect to the AGPS server for a fast fix
 *  3. PORTSWITCH -> allows NMEA data to be sent via UART
 */

CMD_Status A7670_GNSS_Init()
{
	Connect_GNSS_state gnss_state = GNSS_PWR;
	uint8_t attemps = 0;
	const uint8_t max_attemps = 3;
	while(gnss_state != GNSS_OK)
	{
		switch (gnss_state) {
			case GNSS_PWR:
				if(A7670_GNSS_CMD_CGNSSPWR() == CMD_OK && attemps < max_attemps)
				{
					gnss_state = GNSS_CAGPS;
					attemps = 0;
				}
				else if(attemps >= max_attemps)
				{
					gnss_state = GNSS_RESET_MODULE;
				}
				else
				{
					gnss_state = GNSS_PWR;
					attemps++;
				}
			break;
			case GNSS_CAGPS:
				A7670_GNSS_CMD_CAGPS();
				gnss_state = GNSS_PORTSWITCH;
			break;
			case GNSS_PORTSWITCH:
				if(A7670_GNSS_CMD_CGNSSPORTSWITCH() == CMD_OK && attemps < max_attemps)
				{
					gnss_state = GNSS_OK;
					attemps = 0;
				}
				else if(attemps >= max_attemps)
				{
					gnss_state = GNSS_RESET_MODULE;
				}
				else
					gnss_state = GNSS_PORTSWITCH;

			break;
			case GNSS_RESET_MODULE:
				return CMD_ERROR;
			default:
				return CMD_ERROR;
		}
	}

	return CMD_OK;
}

/**
 * @brief Starting to GNSS in A7670 module
 * 
 * Sends the AT+CGNSSPWR command to start the GNSS and waits for initialization
 * IMPORTANT: GNSS may take a few seconds to initialize (~9s)
 * 
 * @return CMD_Status
 * @retval CMD_OK if GNSS Started
 * @retval CMD_ERROR if expected response not found
 */
CMD_Status A7670_GNSS_CMD_CGNSSPWR()
{
	if(AT_sendCommand("AT+CGNSSPWR=1", "READY!", 15000) == AT_OK)
			return CMD_OK;
	else
		return CMD_ERROR;
}

/**
 * @brief Uses the Assited GNSS (AGNSS)
 * 
 * Allows you to obtain data from satellites through the AGNSS server, thus ensuring a much shorter fixation time
 * 
 * @return CMD_Status
 * @retval CMD_OK when AGNSS connected
 * @retval CMD_ERROR when unable to connect to the AGNSS server
 */
CMD_Status A7670_GNSS_CMD_CAGPS()
{
	if(AT_sendCommand("AT+CAGPS", "success", 5000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

/**
 * @brief Active NMEA output infos
 * 
 * Enables NMEA output information using UART, without it it is not possible to use commands like GNSSINFO and GNPSINFO
 * 
 * @return CMD_Status
 */
CMD_Status A7670_GNSS_CMD_CGNSSPORTSWITCH()
{
	if (AT_sendCommand("AT+CGNSSPORTSWITCH=1,1", "", 0) == AT_OK)
		return CMD_OK;

	return CMD_ERROR;
}

/**
 * @brief Send GPSINFO command
 * 
 * Returns GPS data such as date, time, latitude and longitude
 * 
 * @return CMD_Status
 */
CMD_Status A7670_GNSS_CMD_CGPSINFO()
{
	if(AT_sendCommand("AT+CGPSINFO", "OK", 100) == AT_OK)
	{
		readGPSINFO((char*)at.response);
		return CMD_OK;
	}
	return CMD_ERROR;
}

CMD_Status A7670_GNSS_CMD_GNSSINFO()
{
	if(AT_sendCommand("AT+CGNSSINFO", "OK", 100) == AT_OK)
	{
		readGNSSINFO((char*)at.response);
		return CMD_OK;
	}
	return CMD_ERROR;

}





