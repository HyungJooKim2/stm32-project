/*
 * app.c
 *
 *  Created on: Aug 16, 2023
 *      Author: User
 */
#include "main.h"
#include "app.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "i2c_hd44780.h"
#include <stdio.h>

//extern I2C_HandleTypeDef hi2c1;
//extern DMA_HandleTypeDef hdma_i2c1_rx;
//extern DMA_HandleTypeDef hdma_i2c1_tx;

extern UART_HandleTypeDef huart3;
//extern DMA_HandleTypeDef hdma_usart2_rx;
//extern DMA_HandleTypeDef hdma_usart2_tx;

//osMutexId_t mutex_lcd;


//void DispThread_1(void *arg)
//{
//	uint8_t i;
//
//	for (i = 0;;) {
//		osMutexAcquire(mutex_lcd, osWaitForever);
//		lcd_locate(1,0);
//		lcd_printf("1. Count=%d", i++);
//		printf("1. Count=%d\r\n", i);
//		osMutexRelease(mutex_lcd);
//		osDelay(1000);
//	}
//}
//
//void DispThread_2(void *arg)
//{
//	uint8_t i;
//
//	for (i = 0;;) {
//		osDelay(1000);
//		osMutexAcquire(mutex_lcd, osWaitForever);
//		lcd_locate(2,0);
//		lcd_printf("2. Count=%d", i++);
//		printf("2. Count=%d\r\n", i);
//		osMutexRelease(mutex_lcd);
//	}
//}

void app_main(void)
{
//	const osMutexAttr_t m_attr = {
//		"Mutex_LCD",                          // human readable mutex name
//		osMutexPrioInherit,    // attr_bits
//		NULL,                                     // memory for control block
//		0U                                        // size for control block
//	};
    lcd_init(); // LCD 초기화
    lcd_disp_on(); //화면 표시 활성화
    lcd_clear_display(); //화면 초기화
    lcd_home(); //커서 이동
    lcd_print_string("Hello22"); //문자열 LCD예 출력
//	osKernelInitialize();
//
//	osThreadNew(DispThread_1, NULL, NULL);
//	osThreadNew(DispThread_2, NULL, NULL);
//	mutex_lcd = osMutexNew(&m_attr);
//
//	osKernelStart();
}

int fputc(int ch, FILE *f)
{
	 // UART로 문자 출력
	HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 100);
	return ch;
}
void app_init(void)
{
  printf("System start....\r\n");
}

void app(void)
{
  app_main();
  app_init();
  while (1);
}
