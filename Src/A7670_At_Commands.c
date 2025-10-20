/*
 * A7670_At_Commands.c
 *
 *  Created on: Sep 5, 2025
 *      Author: kenny
 */

#include "A7670_At_Commands.h"


/**
 * @brief Reset A7670 Module
 * 
 * This commad resets the module A7670 sending AT+CRESET
 * 
 * @return CMD_Status
 * @retval CMD_OK if reboot is successful
 * @retval CMD ERROR if the command is sent but we do not receive a module initialization response
 */

CMD_Status A7670_CMD_Creset()
{
	if(AT_sendCommand("AT+CRESET", "PB DONE", 20000) == AT_OK)
		return CMD_OK;
	else
		return CMD_ERROR;

}

