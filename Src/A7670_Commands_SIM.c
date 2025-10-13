/*
 * A7670_Commands_SIM.c
 *
 *  Created on: Oct 13, 2025
 *      Author: kenny
 */


#include "A7670_Commands_SIM.h"

CMD_Status A7670_SIM_Check_Network(void)
{
	SIM_Check_Network_State check_network = NETWORK_CHECK_CSQ;
	while(check_network != NETWORK_CHECK_OK)
	{
		switch (check_network) {
			case NETWORK_CHECK_CSQ:
				if(A7670_SIM_Check_CSQ() == CMD_OK)
				{
					check_network = NETWORK_CHECK_CREG;
				}
				else
					check_network = NETWORK_CHECK_ERROR;
			break;
			case NETWORK_CHECK_CREG:
				if(A7670_SIM_CMD_Check_CREG() == CMD_OK)
				{
					check_network = NETWORK_CHECK_CPSI;
				}
				else
					check_network = NETWORK_CHECK_ERROR;
			break;
			case NETWORK_CHECK_CPSI:
				if(A7670_SIM_Check_CPSI() == CMD_OK)
				{
					check_network = NETWORK_CHECK_OK;
				}
				else
					check_network = NETWORK_CHECK_ERROR;
			break;
			case NETWORK_CHECK_ERROR:
				return CMD_ERROR;
			default:
				break;
		}
	}
	return CMD_OK;
}

CMD_Status A7670_SIM_CMD_Check_CREG(void)
{
	const char command[] = "AT+CREG?";
	const char expected_response[] = "+CREG: 0,1";

	if(AT_sendCommand(command, expected_response, 500) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;
}

CMD_Status A7670_SIM_Check_CSQ(void)
{
	const char command[]           = "AT+CSQ";
	const char expected_response[] = "+CSQ:";

	if(AT_sendCommand(command, expected_response, 500) == AT_OK)
	{
	    //"AT+CSQ\r\n+CSQ: 22,99\r\n\0"
	    char response[30];
	    strcpy(response, (char*)at.response);
	    char* ptr_rssi;
	    char* ptr_ber;
	    char* end;

	    ptr_rssi = strstr(response, ": ");
	    ptr_rssi += 2;
	    ptr_ber = strstr(ptr_rssi, ",");
	    *ptr_ber = '\0';
	    ptr_ber++;
	    end = strstr(ptr_ber, "\r");
	    *end = '\0';
	    uint8_t rssi = atoi(ptr_rssi);
	   //uint8_t ber  = atoi(ptr_ber);

	    if(rssi < 31 && rssi > 0)
	    	return CMD_OK;
	}

	return CMD_ERROR;
}

CMD_Status A7670_SIM_Check_CPSI(void)
{
	const char command[]         = "AT+CPSI?";
	const char expected_response[] = "+CPSI: LTE,Online";

	if(AT_sendCommand(command, expected_response, 500) == AT_OK)
	{
		return CMD_OK;
	}
	return CMD_ERROR;
}






























