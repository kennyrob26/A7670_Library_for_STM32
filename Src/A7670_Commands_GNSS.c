/*
 * A7670_Commands_GNSS.c
 *
 *  Created on: Sep 15, 2025
 *      Author: kenny
 */



#include "A7670_Commands_GNSS.h"

NMEA nmea;
GNSS gnss;

//CMD_Status processAtCommand();


/*================= -- Static Functions -- =====================*/


/**
 * @brief Private Function -> Calcule Latitude
 * 
 *	 A private function that calculates Latitude with base GNSS data.
 *	 Converts degress/minutes Latitude to decimal Latitude
 */
static float calculateLatitude(char *latitude_nema, char *N_or_S)
{
    if(strlen(latitude_nema) && strlen(N_or_S))
    {
        char lat_degress_buffer[3];
        char lat_minutes_buffer[15];

        strncpy(lat_degress_buffer, latitude_nema, 2);
        strcpy(lat_minutes_buffer, latitude_nema + 2);

        uint16_t lat_degress = atoi(lat_degress_buffer);
        float lat_minutes = atof(lat_minutes_buffer);

        float latitude = lat_degress + (lat_minutes / 60);

        if(N_or_S[0] == 'S')
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
static float calculateLongitude(char *longitude_nema, char *E_or_W)
{
    if(strlen(longitude_nema) && strlen(E_or_W))
    {
        char log_degress_buffer[4];
        char log_minutes_buffer[15];

        strncpy(log_degress_buffer, longitude_nema, 3);
        strcpy(log_minutes_buffer, longitude_nema + 3);

        uint16_t log_degress =  atoi(log_degress_buffer);
        float log_minutes    =  atof(log_minutes_buffer);

        float longitude = log_degress + (log_minutes / 60);

        if(E_or_W[0] == 'W')
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
static float calculateSpeedKmh(char* speed_kont)
{
	float speed_kontf = atof(speed_kont);

	if((speed_kontf != 0) && (speed_kontf < 200))
	{
		const float const_knot_to_kmh = 1.825;
		float speed_kmh = speed_kontf * const_knot_to_kmh;
		return speed_kmh;
	}
	else
		return 0;
}

/**
 * @brief Private Function -> Scrolls through NMEA data
 * 
 * 	* This function finds delimiter for NMEA values.
 * 	* The function handles one value per call, where "value" is the current value to be handled,
 *    and "previous_value" was the previous value.
 * 	*When we finish processing the current value we point to the next one, and thus close the cycle
 * 	
 */
static void nextValueNmea(char **value, char *previous_value)
{
    if(previous_value != NULL)
    {
        *value = strstr(previous_value, ",");
        if(*value != NULL)
        {
            **value = '\0';
            *value += 1;
        }
    }
}

/**
 * @brief Private Function -> Handles NMEA values
 * 
 * 	This function finds delimiter for NMEA values
 */
static void readNMEA(char *dataNMEA)
{
	strcpy(nmea.command, dataNMEA);

	if(strlen(nmea.command) != 0)
	{
		nmea.lat = strstr(nmea.command, " ");
		*nmea.lat = '\0';
		nmea.lat += 1;
	}

    nextValueNmea(&nmea.N_or_S, nmea.lat);
    nextValueNmea(&nmea.log, nmea.N_or_S);
    nextValueNmea(&nmea.E_or_W, nmea.log);
    nextValueNmea(&nmea.date, nmea.E_or_W);
    nextValueNmea(&nmea.utc_time, nmea.date);
    nextValueNmea(&nmea.alt, nmea.utc_time);
    nextValueNmea(&nmea.speed, nmea.alt);
    nextValueNmea(&nmea.course, nmea.speed);

    gnss.latitude  = calculateLatitude(nmea.lat, nmea.N_or_S);
    gnss.longitude = calculateLongitude(nmea.log, nmea.E_or_W);
    sprintf(gnss.latitude_longitude, "%f, %f", gnss.latitude, gnss.longitude);
    gnss.speed_kmh = calculateSpeedKmh(nmea.speed);

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
	if(AT_sendCommand("AT+CGPSINFO", "OK", 10) == AT_OK)
	{
		readNMEA((char*)at.response);
		return CMD_OK;
	}
	return CMD_ERROR;
}






