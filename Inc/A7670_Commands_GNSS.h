/*
 * A7670_Commands_GNSS.h
 *
 *  Created on: Sep 15, 2025
 *      Author: kenny
 */

#ifndef INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_GNSS_H_
#define INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_GNSS_H_

#include <AT_Handler.h>
#include "A7670_Types_GNSS.h"


CMD_Status A7670_GNSS_Init();
CMD_Status A7670_GNSS_CMD_CGNSSPWR();
CMD_Status A7670_GNSS_CMD_CAGPS();
CMD_Status A7670_GNSS_CMD_CGNSSPORTSWITCH();
CMD_Status A7670_GNSS_CMD_CGPSINFO();

void readNEMA(char *dataNEMA);
void nextValueNema(char **value, char *previous_value);
void convertNemaToGNSS();
float calculateLatitude(char *latitude_nema, char *N_or_S);
float calculateLongitude(char *longitude_nema, char *E_or_W);


#endif /* INC_A7670_LIBRARY_FOR_STM32_A7670_COMMANDS_GNSS_H_ */
