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
#include "pwm.h"

extern TIM_HandleTypeDef htim3;

void pwm_init(void)
{
  printf("Timer3 PWM start\r\n");
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

bool pwm_dimming(uint8_t no, uint8_t duty)
{
  bool res = true;
  uint16_t ccr = (uint16_t)duty * 10;

  switch (no) {
    case 0 :
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, ccr);
      break;
    default :
      res = false;
  }

  return res;
}
