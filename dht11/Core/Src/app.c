/*
 * app.c
 *
 *  Created on: Jul 27, 2023
 *      Author: User
 */
#include <stdio.h>
#include <stdbool.h>

#include "main.h"
#include "key.h"
#include "dht11.h"
#include "app.h"

extern TIM_HandleTypeDef htim14;
extern UART_HandleTypeDef huart3;

extern void htc11_proc(void);

void app(void)
{
//	static uint16_t prev, curr;

	htc11_init();

	while (1) {
		htc11_proc();
//		HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
//		prev =  __HAL_TIM_GET_COUNTER(&htim14);
//		HAL_Delay(20);
//		HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
//		curr =  __HAL_TIM_GET_COUNTER(&htim14);
//		printf("[%06d][%06d][%d]\r\n", prev, curr, (uint16_t)(curr-prev));
//		HAL_Delay(1000);
	}
}

int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}

