/*======================================================
    Library A7670 for STM32
    Author: Kenny
    Date: 25/10/2025

    Example for MQTT echo

    This example receives an MQTT message in the "receive" topic
    and publishes that same message in the "publish" topic, 
    that is, we perform an "echo"
    
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

uint32_t time = 0;
int count = 0;

/*========== --- CallBack MQTT response --- ========== */
void CallBack_MQTT_Response(MQTT_RESPONSE mqtt)
{
    //if topic received is "receive" 
	if(strcmp(mqtt.message.topic, "receive") == 0)
	{
        A7670_MQTT_PublishMessage("publish", mqtt.message.payload);       //publishes the received message
	}

}

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
  A7670_MQTT_Connect(MQTT_RECONNECT_ENABLE);               //Connect MQTT broker and enable auto reconnect

  A7670_MQTT_SubscribeTopic("receive");                       //Subscribe to the "receive" topic

  A7670_MQTT_Register_Callback_Response(CallBack_MQTT_Response);    //Define MQTT Callback recive 
  while (1)
  {
    A7670_MQTT_Handler();
  }
}