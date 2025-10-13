/*======================================================
    Library A7670 for STM32
    Author: Kenny
    Date: 08/10/2025

    Example for MQTT Sending messages

    Infos:
        Nucleo: STM32G4XX
        Framework: Cube IDE
=======================================================*/

#include "main.h"

#include "A7670_At_Commands.h"

uint32_t time = 0;
int count = 0;

int main(void)
{
  /*========== --- Default HAL Config --- ========== */
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UART4_Init();

/*=============== --- Libary Code --- ============= */

  AT_defineUART(&huart4);
  A7670_CMD_Creset();

  A7670_MQTT_setClient(0, "client1");                      //Sets a index and client name
  A7670_MQTT_SetBroker("tcp://mybroker:1883", 60, 1, 1);   //Sets broker configs
  A7670_MQTT_Connect();                                    //Connect MQTT

  while (1)
  {
    A7670_MQTT_Handler();

    if((HAL_GetTick() - time) >5000)                        //Publish MQTT Message for each 5 seconds
    {
        time = HAL_GetTick();                               //Reset time

        char payload[11];                                   //buffer to payload MQTT
        sprintf(payload, "Count: %d", count);               //convert count to int
        A7670_MQTT_PublishMessage("teste1", payload);       //publish MQTT payload
        count++;                                            //count increments    
    }
  }
}