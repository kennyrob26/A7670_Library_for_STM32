/*======================================================
    Library A7670 for STM32
    Author: Kenny
    Date: 25/10/2025

    Example for MQTT Authenticate with user and password

    This example uses function A7670_MQTT_SetAuthauthentication
    to connect to the MQTT broker
    
    Infos:
        Framework: Cube IDE

        MCU
          Nucleo: STM32G4XX
          Clock used: 80MHz on all peripherals

        Connection UART
          UART 4
          Mode: Asynchronous
          DMA: in RX, and uses Normal Mode
          Interruptions: Enable UART interruption
          Baud Rate: 115200 8N1
=======================================================*/

#include "main.h"

#include "A7670_At_Commands.h"


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

  A7670_MQTT_setClient("client1");                      //Sets a index and client name
  A7670_MQTT_SetBroker("tcp://mybroker:1883", 60, 1, 1);   //Sets broker configs
  A7670_MQTT_SetAuth("MyUser", "MyPassword");              //Sets username and password to broker
  A7670_MQTT_Connect(MQTT_RECONNECT_ENABLE);               //Connect MQTT broker and enable auto 

  while (1)
  {
    A7670_MQTT_Handler();
  }
}