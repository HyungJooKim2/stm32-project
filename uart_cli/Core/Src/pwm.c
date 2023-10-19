/*
 * pwm.c
 *
 *  Created on: Aug 7, 2023
 *      Author: User
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"

// 외부에서 선언된 타이머 핸들러 변수
extern TIM_HandleTypeDef htim3;

// PWM 초기화 함수
void pwm_init(void)
{
  printf("Timer3 PWM start\r\n"); // PWM 타이머 초기화 메시지 출력
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3); // TIM3의 채널 3을 이용하여 PWM 시작
}

// PWM 신호로 밝기 조절하는 함수
bool pwm_dimming(uint8_t no, uint8_t duty)
{
  bool res = true; // 반환값으로 사용할 불리언 변수 초기화
  uint16_t ccr = (uint16_t)duty * 10; // duty 주기를 타이머의 Compare/Capture 레지스터 값으로 변환

  switch (no) {
    case 0 :
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, ccr); // TIM3의 채널 3의 Compare/Capture 레지스터에 ccr 값을 설정하여 밝기 조절
      break;
    default :
      res = false; // 지원하지 않는 no 값인 경우 res를 false로 설정
  }

  return res; // 처리 결과 반환 (성공: true, 실패: false)
}
