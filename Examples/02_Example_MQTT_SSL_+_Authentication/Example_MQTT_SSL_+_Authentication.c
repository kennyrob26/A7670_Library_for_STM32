/*======================================================
    Library A7670 for STM32
    Author: Kenny
    Date: 29/10/2025

    Example for MQTT uses SSL protocol and Authenticate with user and password

    This example uses function A7670_MQTT_SetAuthauthentication
    to connect to the MQTT broker, and uses A7670_SSLConfig() for 
    ssl context config
    
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

        Broker MQTT
          HiveMQ with SSL and Authentication
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

  AT_defineUART(&huart4);                                                           //huart4 is a UART utilized

  A7670_CMD_Creset();                                                               //reset to module A7670

  A7670_SSLConfig(0, SSL_VERSION_ALL, SSL_AUTH_SERVER_ONLY, "isrgrootx1.pem");      //Sets SSL Context
  A7670_MQTT_setClient("client1");                                               //Sets Client Index name
  A7670_MQTT_SetAuth("kenny", "Kenny12345");                                        //Sets username and password to auth broker
  A7670_MQTT_SetBroker("tcp://mylink.s1.eu.hivemq.cloud:8883", MQTT_KEEP_ALIVE_DEFAULT, MQTT_CLEAR_SESSION_ENABLE, MQTT_QoS_1);  

  A7670_MQTT_Connect(MQTT_RECONNECT_ENABLE);                                        //Connect to broker with reconnect enable

  while (1)
  {
    A7670_MQTT_Handler();
  }
}