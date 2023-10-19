/*
 * uart.c
 *
 *  Created on: Jul 28, 2023
 *      Author: User
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "main.h"
#include "uart.h"
#include "cli.h"

extern UART_HandleTypeDef huart3;  // UART 통신 핸들러 선언

#define D_BUF_MAX 100  // 데이터를 저장할 버퍼의 최대 크기 정의

// 데이터를 저장할 구조체 정의
typedef struct {
  uint8_t buf[D_BUF_MAX+1];  // 데이터 버퍼 변수
  uint8_t idx;               // 버퍼 인덱스 변수
  uint8_t flag;              // 수신 완료 플래그
} BUF_T;

BUF_T gBufObj[2];  // 두 개의 버퍼 구조체 배열로 선언
static UART_CBF uart_cbf;  // UART 수신 콜백 함수 포인터 선언
uint8_t rxd;  // 수신한 바이트 데이터를 저장할 변수

void uart_init(void)
{
  gBufObj[0].idx = 0;
  gBufObj[0].flag = false;
  // UART 수신 인터럽트 활성화 및 첫 번째 바이트(데아터) 수신 대기
  HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxd, 1);
  printf("UART Initialized...\r\n");
  fflush(stdout);  // printf 후 출력 버퍼 비움
}

// UART 수신 콜백 함수 등록
void uart_regcbf(UART_CBF cbf)
{
  uart_cbf = cbf;  // 전달된 콜백 함수를 uart_cbf에 등록
}

// UART 수신된 데이터 처리 함수
void uart_proc(void)
{
  BUF_T *p = (BUF_T *)&gBufObj[0];
  if (p->flag == true) {
    printf("%s:%s", __func__, p->buf);  // 디버깅용 수신된 데이터 출력

    if (uart_cbf != NULL) uart_cbf(p->buf); // UART 데이터 파싱 함수 호출 (등록된 콜백 함수)
    p->idx = 0;  // 다음 데이터 수신을 위한 버퍼 인덱스 초기화
    p->flag = false;  // 수신 완료 플래그 초기화
  }
}

// UART 수신 완료 콜백 함수
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart3) {
    BUF_T *p = (BUF_T *)&gBufObj[0];

    if (p->flag == false) {
      p->buf[p->idx] = rxd;  // 수신한 데이터를 버퍼에 저장

      if (p->idx < D_BUF_MAX) {
        p->idx++;  // 버퍼 인덱스 증가
      }

      if (rxd == '\r' || rxd == '\n') {
        p->buf[p->idx] = '\0';  // 수신 데이터의 끝에 NULL 문자 추가
        p->flag = true;  // 수신 완료 플래그 설정
      }
    }

    uart_proc();  // 수신된 데이터 처리 함수 호출
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxd, 1);  // 다음 수신 대기
  }
}

// UART 표준 입출력 함수
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xffff);  // 문자 전송
  return ch;
}
