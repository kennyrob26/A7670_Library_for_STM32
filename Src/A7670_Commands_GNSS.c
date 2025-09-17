/*
 * A7670_Commands_GNSS.c
 *
 *  Created on: Sep 15, 2025
 *      Author: kenny
 */



#include "A7670_Commands_GNSS.h"

NEMA nema;
GNSS gnss;

CMD_Status processAtCommand();


CMD_Status A7670_GPS_Init()
{
	Connect_GNSS_state gnss_state = GNSS_PWR;
	uint8_t attemps = 0;
	const uint8_t max_attemps = 3;
	while(gnss_state != GNSS_OK)
	{
		switch (gnss_state) {
			case GNSS_PWR:
				if(A7670_GPS_CMD_CGNSSPWR() == CMD_OK && attemps < max_attemps)
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
				A7670_GPS_CMD_CAGPS();
				gnss_state = GNSS_PORTSWITCH;
			break;
			case GNSS_PORTSWITCH:
				if(A7670_GPS_CMD_CGNSSPORTSWITCH() == CMD_OK && attemps < max_attemps)
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

CMD_Status A7670_GPS_CMD_CGNSSPWR()
{
	strcpy(at.at_command, "AT+CGNSSPWR=1");

	if(AT_sendCommand("READY!", 15000) == AT_OK)
			return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_GPS_CMD_CAGPS()
{
	strcpy(at.at_command, "AT+CAGPS");
	if(AT_sendCommand("success", 5000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_GPS_CMD_CGNSSPORTSWITCH()
{
	strcpy(at.at_command, "AT+CGNSSPORTSWITCH=1,1");
	if (AT_sendCommand("", 0) == AT_OK)
		return CMD_OK;

	return CMD_ERROR;
}
CMD_Status A7670_GPS_CMD_CGPSINFO()
{
	strcpy(at.at_command, "AT+CGPSINFO");

	if(AT_sendCommand("", 0) == AT_OK)
	{
		readNEMA(at.response);
		return CMD_OK;
	}
	return CMD_ERROR;
}

void readNEMA(char *dataNEMA)
{
	nema.command = dataNEMA;
	if(nema.command != NULL)
	{
		nema.lat = strstr(dataNEMA, " ");
		*nema.lat = '\0';
		nema.lat += 1;
	}

    nextValueNema(&nema.N_or_S, nema.lat);
    nextValueNema(&nema.log, nema.N_or_S);
    nextValueNema(&nema.E_or_W, nema.log);
    nextValueNema(&nema.date, nema.E_or_W);
    nextValueNema(&nema.utc_time, nema.date);
    nextValueNema(&nema.alt, nema.utc_time);
    nextValueNema(&nema.speed, nema.alt);
    nextValueNema(&nema.course, nema.speed);


}

void nextValueNema(char **value, char *previous_value)
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


void convertNemaToGNSS(GNSS *gnss, NEMA nema)
{
    gnss->latitude           =  calculateLatitude(nema.lat, nema.N_or_S);
    gnss->longitude          =  calculateLongitude(nema.log, nema.E_or_W);
    //formatLatitudeLongitude(gnss);
    sprintf(gnss->latitude_longitude, "%f, %f", gnss->latitude, gnss->longitude);
}

float calculateLatitude(char *latitude_nema, char *N_or_S)
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

float calculateLongitude(char *longitude_nema, char *E_or_W)
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
