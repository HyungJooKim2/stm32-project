/*
 * app.c
 *
 *  Created on: Aug 10, 2023
 *      Author: User
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"
#include "app.h"

// ADC, DMA, UART 핸들러 변수 선언
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern UART_HandleTypeDef huart3;

// ADC 채널에서 받은 값 저장용 배열 선언
static volatile uint16_t adc_value[4];

// EXTI에서 받은 값 저장용 변수 선언
static volatile uint16_t exti_value = 0;

void app(void)
{
	// ADC 변환을 시작하는 것과 동시에 DMA를 사용하여 데이터를 메모리에 저장
    // CPU의 개입 없이 데이터를 수집할 수 있음
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_value, 4);

	for (;;) {
		HAL_Delay(100);
		// ADC 온도값과 EXTI 상태 값을 UART를 통해 출력
		printf("ExtUnitTemp = %d ExtUnitTemp = %d InternalTemp = %d Vref = %d ExtValue = %d\r\n",
		       adc_value[0], adc_value[1], adc_value[2], adc_value[3],
		       exti_value);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	exti_value = GPIO_Pin; //GPIO 13번 Pin의 상태값 0 또는 1 저장
}

int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xffff); // UART 출력
  return ch;
}
