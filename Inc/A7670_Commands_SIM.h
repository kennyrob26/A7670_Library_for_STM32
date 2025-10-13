/*
 * A7670_Commands_SIM.h
 *
 *  Created on: Oct 13, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SIM_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SIM_H_

#include <AT_Handler.h>

typedef enum
{
	NETWORK_CHECK_OK    = 0,
	NETWORK_CHECK_ERROR = 1,
	NETWORK_CHECK_CSQ   = 2,
	NETWORK_CHECK_CREG  = 3,
	NETWORK_CHECK_CPSI  = 4
}SIM_Check_Network_State;

CMD_Status A7670_SIM_Check_Network(void);
CMD_Status A7670_SIM_CMD_Check_CREG(void);
CMD_Status A7670_SIM_Check_CSQ(void);
CMD_Status A7670_SIM_Check_CPSI(void);
#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_SIM_H_ */
