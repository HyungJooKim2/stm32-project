/*
 * app.c
 *
 *  Created on: Jul 28, 2023
 *      Author: User
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "app.h"

extern TIM_HandleTypeDef htim2; // 타이머 핸들러 가져옴
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart3; // uart 핸들러 가져옴

#define D_DUTY_MAX	1000 // 최대 duty 값
bool exti_flag = false; // 외부 인터럽트 플래그 선언

static void app_init(void);

void app(void)
{
  int16_t duty = 0;	//pwm duty 변수 선언

  volatile bool dir_flag = true; //duty 증가 감소 방향 flag 선언
  volatile uint32_t prev, curr;	//현재 시간, 이전 시간 변수 선언

  printf("System started.....\r\n");

  app_init();
  prev = curr = HAL_GetTick();

  while (1)
  {
#if 1
	// 시간 방식 duty가 Max 1000을 기준으로 50씩 감소 또는 증가
    curr = HAL_GetTick();

    if (curr - prev > 25) {
      if (dir_flag == true) {
        duty += 50;
        if (duty > D_DUTY_MAX) {
          dir_flag = false;
          duty = D_DUTY_MAX;
        }
      }
      else if(dir_flag == false) {
        duty -= 50;
        if (duty < 0) {
          dir_flag = true;
          duty = 0;
        }
      }
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, (uint16_t)duty);
                       //TIM3 PWM 채널 3의 duty 주기를 계속해서 적용시킴
      prev = curr;
    }

#else	// 버튼 방식 버튼을 눌렀을 때 99씩 duty 증가 MAx 1000이 되면 duty를 0으로
    if (exti_flag == true) {
      exti_flag = false;
      duty += 99;
      if (duty > 1000) duty = 0;
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, duty);
    }
#endif
 }
}

// 애플리케이션 초기화 함수 정의
static void app_init(void)
{
  printf("Timer2,3 Base start\r\n");

  // TIM2&TIM3 Base 타이머 시작
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_Base_Start(&htim3);
  // TIM3 채널3 PWM 시작
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

// 외부 입출력 Callback 함수
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t prev;
  volatile uint32_t curr = HAL_GetTick();
  if (curr - prev > 150 ) { //인터럽트 발생 최소 간격 0.15초
    if (GPIO_Pin == GPIO_PIN_13) exti_flag = true;
    prev = curr;
  }
}

// UART 출력 함수
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}
