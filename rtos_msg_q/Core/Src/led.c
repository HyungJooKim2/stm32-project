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

void led_init(void)
{
  printf("LED initialized...\r\n");
}

bool led_onoff(uint8_t no, uint8_t onoff)
{
  GPIO_PinState sts = onoff ? GPIO_PIN_SET : GPIO_PIN_RESET;

  switch (no) {
    case 2 :
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, sts);
      break;
    case 3 :
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, sts);
      break;
    default :
      return false;
  }

  return true;
}
