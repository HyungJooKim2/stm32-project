/*
 * led.c
 *
 *  Created on: Aug 7, 2023
 *      Author: User
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "led.h"

// LED 초기화 함수
void led_init(void)
{
  printf("LED initialized...\r\n"); // LED 초기화 메시지 출력
}

// LED 상태를 켜거나 끄는 함수
bool led_onoff(uint8_t no, uint8_t onoff)
{
  GPIO_PinState sts = onoff ? GPIO_PIN_SET : GPIO_PIN_RESET; // 전달된 onoff 값에 따라 LED 상태 설정

  switch (no) {
    case 2 :
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, sts); // LD2 LED 상태 변경
      break;
    case 3 :
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, sts); // LD3 LED 상태 변경
      break;
    default :
      return false; // 지원하지 않는 no 값인 경우 false 반환
  }

  return true; // 처리 결과 반환 (성공: true, 실패: false)
}
