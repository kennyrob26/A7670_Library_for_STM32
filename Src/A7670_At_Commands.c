/*
 * A7670_At_Commands.c
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#include "A7670_At_Commands.h"

NEMA nema;
GNSS gnss;

CMD_Status processAtCommand();
CMD_Status mqtt_response();

MQTT_RESPONSE mqtt_resp;


CMD_Status processAtCommand();

CMD_Status A7670_CMD_Creset()
{
	strcpy(at.at_command, "AT+CRESET");

	if(AT_sendCommand() == AT_OK)
	{
		AT_config_Wait_Response("PB DONE", 15000);
		if(AT_check_Wait_Response_Blocking() == AT_OK)
		{
			return CMD_OK;
		}

	}
	return CMD_ERROR;

}
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

	if(AT_sendCommand() == AT_OK)
	{
		AT_config_Wait_Response("READY!", 15000);
		if(AT_check_Wait_Response_Blocking() == AT_OK)
			return CMD_OK;
	}

	return CMD_ERROR;
}

CMD_Status A7670_GPS_CMD_CAGPS()
{
	strcpy(at.at_command, "AT+CAGPS");
	if(AT_processCommand() == AT_OK)
	{
		AT_config_Wait_Response("success", 5000);
		if(AT_check_Wait_Response_Blocking() == AT_OK)
			return CMD_OK;
	}
	return CMD_ERROR;
}

CMD_Status A7670_GPS_CMD_CGNSSPORTSWITCH()
{
	strcpy(at.at_command, "AT+CGNSSPORTSWITCH=1,1");
	if (AT_processCommand())
		return CMD_OK;

	return CMD_ERROR;
}
CMD_Status A7670_GPS_CMD_CGPSINFO()
{
	strcpy(at.at_command, "AT+CGPSINFO");

	if(AT_processCommand() == AT_OK)
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






CMD_Status A7670_Pocess_Buffer()
{

    AT_ResponseType response_type = AT_RT_NULL;

    if(strncmp((char*)at.response_buffer, "AT", 2) == 0)
        response_type = AT_RT_Echo_Command;
    else if (strncmp((char*)at.response_buffer, "\r\n+CMQTTRXSTART", 15) == 0)
        response_type = AT_RT_MQTT_Response;
    else if(strncmp((char*)at.response_buffer, "\r\n", 2) == 0)
    	response_type = AT_RT_Response;


	switch (response_type)
	{
	case AT_RT_Echo_Command:
		processAtCommand();
		return CMD_OK;
	break;
	case AT_RT_MQTT_Response:
		AT_config_Wait_Response("+CMQTTRXEND: 0", 50);				//waiting for "END" of message
		if(AT_check_Wait_Response_Blocking() == AT_OK)
		{
			strcpy(mqtt_resp.last_message, (const char*)at.response_buffer);
			mqtt_response();
			return CMD_OK;
		}
		else
			return CMD_ERROR;
	break;
	case AT_RT_Response:
		//at.response = at.response_buffer;
		return CMD_OK;
	break;
	default:
		break;
	}

    return AT_ERROR;
}

CMD_Status processAtCommand()
{

    at.echo = strtok((char*)at.response_buffer, "\r\r\n");
    if(at.echo == NULL)
    {
        at.status = 0;
        return CMD_ERROR;
    }

    at.response = strtok(NULL, "\r\n");
    if(at.response != NULL)
    {
        if(!strcmp(at.response, "OK"))
        {
            at.OK = at.response;
            at.status = 1;
            return CMD_OK;
        }
        else if(!strcmp(at.response, "ERROR"))
        {
            at.OK = at.response;
            at.status = 0;
            return CMD_ERROR;
        }
        else
        {
            at.OK = strtok(NULL, "\r\n");
            at.status = 1;
            return CMD_OK;
        }
    }

    return CMD_ERROR;

}

CMD_Status mqtt_response()
{
    uint8_t index = 0;
    while(mqtt_resp.last_message[index] != '\0')
    {
        if(mqtt_resp.last_message[index] == '\r')
        {
            mqtt_resp.last_message[index] = '\n';
        }
        index++;
    }

    char *start;
    char *topic;
    char *payload;
    char *end;

    start = strtok((char*)mqtt_resp.last_message, "\n");
    (void)strtok(NULL, "\n");
    topic = strtok(NULL, "\n");
    (void)strtok(NULL, "\n");
    payload =  strtok(NULL, "\n");
    end = strtok(NULL, "\n");

    char *ptr;
    start = strtok(start, " ");
    if((ptr = strtok(NULL, ",")) != NULL)
        mqtt_resp.client_id  = atoi(ptr);

    if((ptr = strtok(NULL, ",")) !=NULL)
        mqtt_resp.topic_lentgth = atoi(ptr);

    if((ptr = strtok(NULL, ",")) !=NULL)
        mqtt_resp.payload_length = atoi(ptr);

    if(topic != NULL)
    	strcpy(mqtt_resp.topic, topic);

    if(payload != NULL)
    	strcpy(mqtt_resp.payload, payload);

    end = strtok(end, " ");
    if((ptr = strtok(NULL, " ")) != NULL)
        mqtt_resp.end = atoi(ptr);

    return CMD_OK;

}



