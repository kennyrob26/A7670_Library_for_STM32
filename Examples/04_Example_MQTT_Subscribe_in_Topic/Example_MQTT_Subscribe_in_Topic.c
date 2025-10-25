/*======================================================
    Library A7670 for STM32
    Author: Kenny
    Date: 25/10/2025

    Example for MQTT subscribe to a topic

    This example receive a MQTT message in the "light" topic,
    if message is "ON", turn on the internal LED, but if message
    is "OFF", turn off the internal LED
    
    Infos:
        Nucleo: STM32G4XX
        Framework: Cube IDE
=======================================================*/

#include "main.h"

#include "A7670_At_Commands.h"

uint32_t time = 0;
int count = 0;

/*========== --- CallBack MQTT response --- ========== */
void CallBack_MQTT_Response(MQTT_RESPONSE mqtt)
{
    //if topic received is "light" 
	if(strcmp(mqtt.message.topic, "light") == 0)
	{
        //if payload is "ON", turn on the internal LED
		if(strcmp(mqtt.message.payload, "ON") == 0)
        {
			HAL_GPIO_WritePin(GPIOC, led_Pin, GPIO_PIN_SET);
        }
        //if payload is "OFF", turn on the internal LED
		else if(strcmp(mqtt.message.payload, "OFF") == 0)
        {
			HAL_GPIO_WritePin(GPIOC, led_Pin, GPIO_PIN_RESET);
        }
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

  A7670_MQTT_setClient(0, "client1");                      //Sets a index and client name
  A7670_MQTT_SetBroker("tcp://mybroker:1883", 60, 1, 1);   //Sets broker configs
  A7670_MQTT_Connect(MQTT_RECONNECT_ENABLE);               //Connect MQTT broker and enable auto reconnect

  A7670_MQTT_SubscribeTopic("light");                       //Subscribe to the "light" topic

  A7670_MQTT_Register_Callback_Response(CallBack_MQTT_Response);    //Define MQTT Callback recive 
  while (1)
  {
    A7670_MQTT_Handler();
  }
}