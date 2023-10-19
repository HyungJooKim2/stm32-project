/*
 * app.c
 *
 *  Created on: Aug 10, 2023
 *      Author: User
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "string.h"
#include "cmsis_os.h"
#include "led.h"
#include "pwm.h"
#include "uart.h"
#include "cli.h"
#include "bluetooth.h"
#include "app.h"

osThreadId_t thread1_handle;

const osThreadAttr_t thread1_attributes = {
  .name = "thread1",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

static void thread1_Task(void *arg);

void app_init(void)
{
  printf("System start....\r\n");
}

void app(void)
{
  osKernelInitialize();

  app_init();
  led_init();
  pwm_init();
  uart_init();
  cli_init();
  bt_init();

  thread1_handle = osThreadNew(thread1_Task, NULL, &thread1_attributes);

  osKernelStart();

  while (1);
}

static void thread1_Task(void *arg)
{
  for(;;) {
    osDelay(200);
  }
}
