/*
 * app.c
 *
 *  Created on: Aug 8, 2023
 *      Author: User
 */
#include <stdio.h>
#include "main.h"
#include "string.h"
#include "cmsis_os.h"
#include "app.h"
#define EVENT_FLAGS
// 스레드1, 스레드2 ID 변수
osThreadId_t thread1_handle, thread2_handle;

const osThreadAttr_t thread1_attributes = {
  .name = "thread1",  // 스레드 이름 지정
  .stack_size = 256 * 4,  // 스레드 스택 크기 256바이트로 설정
  .priority = (osPriority_t) osPriorityNormal,  // 중간 우선순위로 설정
};

const osThreadAttr_t thread2_attributes = {
  .name = "thread2",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

static void thread1_Task(void *arg)
{
  uint8_t i = 0;
/*
EVENT_FLAGS가 define 되어 있을 경우
osEventFlagsNew를 통해 이벤트 플래그 객체 생성
*/
#if defined(EVENT_FLAGS)
  osEventFlagsId_t evt_handle = osEventFlagsNew(NULL);
#endif

  for(;;) {
    osDelay(100);
#if defined(EVENT_FLAGS)
    osEventFlagsSet(evt_handle, 1 << i); // 이벤트 플래그 설정
#else
    osThreadFlagsSet(thread2_handle, 1 << i); // 다른 스레드에 플래그 설정
#endif
    i++; i%=4; //i 값 -> 0~3
    if (i==0) osDelay(1000);
    //i가 0일때 1초 후 toggle 0이 아닐때 0.1초 후 toggle
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}

static void thread2_Task(void *arg)
{
  uint32_t flags;
  for(;;) {
#if defined(EVENT_FLAGS)
    // 이벤트 플래그 대기
    flags = osEventFlagsWait(evt_handle, 0x000f, osFlagsWaitAny, 1000); //osWaitForever);
#else
    // 다른 스레드로부터 플래그 대기
    flags = osThreadFlagsWait(0x000f, osFlagsWaitAny, 1000);
#endif
    printf("flags : %08x : ", flags);
    if (flags != osFlagsErrorTimeout ) {
      // flags의 각 비트를 검사하여 설정된 플래그를 출력
      if (flags & 0x0001) { printf("1\r\n");  }
      if (flags & 0x0002) { printf("2\r\n");  }
      if (flags & 0x0004) { printf("4\r\n");  }
      if (flags & 0x0008) { printf("8\r\n");  }
    } else {
			printf("flags timeout\r\n");
    }
    // LED 토글
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
  }
}

void app(void)
{
  // 운영체제 초기화
  osKernelInitialize();
  // 스레드 생성
  thread1_handle = osThreadNew(thread1_Task, NULL, &thread2_attributes);
  thread2_handle = osThreadNew(thread2_Task, NULL, &thread2_attributes);
  // 운영체제 시작
  osKernelStart();
}
