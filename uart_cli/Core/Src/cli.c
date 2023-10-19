/*
 * cli.c
 *
 *  Created on: Jul 28, 2023
 *      Author: User
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "main.h"
#include "uart.h"
#include "led.h"
#include "pwm.h"
#include "cli.h"

#define D_DELIMITER	" ,\r\n" //공백, 쉼표, 구분자 정의

// 명령어 처리 함수의 시그니처 정의
typedef int (*CMD_FUNC_T)(int argc, char **argv);

// 명령어와 해당 명령어를 처리할 함수 정보를 저장하는 구조체 정의
typedef struct {
  char *cmd;        // 명령어 문자열
  uint8_t no;       // 명령어 번호 (고유 식별자)
  CMD_FUNC_T cbf;   // 명령어를 처리할 함수 포인터
  char *remark;     // 명령어에 대한 설명 문자열
} CMD_LIST_T;

// 각 명령어에 대한 처리 함수 선언
static int cli_help(int argc, char *argv[]);
static int cli_echo(int argc, char *argv[]);
static int cli_led(int argc, char *argv[]);
static int cli_pwm(int argc, char *argv[]);

// 명령어와 함수 정보를 저장하는 배열 선언 및 초기화
const CMD_LIST_T gCmdListObj[] = {
  { "help",  1, cli_help,  "help" },
  { "echo",  2, cli_echo,  "echo [echo data]" },
  { "led",   3, cli_led,   "led [2/3] [on/off]" },
  { "pwm",   4, cli_pwm,   "pwm [0] [0~100]" },
  { NULL,    0, NULL,      NULL }
};

// help 명령어 처리 함수
static int cli_help(int argc, char *argv[])
{
  // 등록된 모든 명령어에 대한 설명 출력
  for (int i = 0; gCmdListObj[i].cmd != NULL; i++) {
    printf("%s\r\n", gCmdListObj[i].remark);
  }

  return 0;
}

// echo 명령어 처리 함수
static int cli_echo(int argc, char *argv[])
{
  // 명령어에 필요한 인자 개수가 충족되지 않은 경우 에러 메시지 출력
  if (argc < 2) printf("Need more arguments\r\n");
  // 입력된 데이터 출력
  printf("%s\r\n", argv[1]);

  return 0;
}

// led 명령어 처리 함수
static int cli_led(int argc, char *argv[])
{
  // 명령어에 필요한 인자 개수가 충족되지 않은 경우 에러 메시지 출력
  if (argc < 3) printf("Need more arguments\r\n");

  // 문자열을 숫자로 변환하여 사용하기 위한 변수 선언 및 변환
  long no = strtol(argv[1], NULL, 10);
  int onoff = strcasecmp(argv[2], "off");

  if (onoff != 0) onoff = 1;

  // LED 제어 함수 호출
  led_onoff((uint8_t)no, (uint8_t)onoff);

  return 0;
}

// pwm 명령어 처리 함수
static int cli_pwm(int argc, char *argv[])
{
  // 명령어에 필요한 인자 개수가 충족되지 않은 경우 에러 메시지 출력
  if (argc < 3) printf("Need more arguments\r\n");

  // 문자열을 숫자로 변환하여 사용하기 위한 변수 선언 및 변환
  long no = strtol(argv[1], NULL, 10);
  long duty = strtol(argv[2], NULL, 10);

  // duty 주기 값이 유효한 범위를 벗어난 경우 조정
  if (duty > 100) duty = 100;
  if (duty < 0) duty = 0;

  // PWM 제어 함수 호출
  pwm_dimming((uint8_t)no, (uint8_t)duty);

  return 0;
}

// 명령어 문자열을 분석하여 해당 명령어의 처리 함수 호출
bool cli_parser(uint8_t *buf)
{
#if 1
  int argc = 0;
  char *argv[10];
  char *ptr;

  // 입력된 문자열을 공백 및 구분자로 분리하여 argv 배열에 저장
  ptr = strtok((char *)buf, D_DELIMITER);
  if (ptr == NULL) return false;

  while(ptr != NULL) {
    argv[argc] = ptr;
    argc++;
    ptr = strtok(NULL, D_DELIMITER);
  }

  // 입력된 명령어와 등록된 명령어를 비교하여 해당 명령어의 처리 함수 호출
  for (int i=0; gCmdListObj[i].cmd != NULL; i++) {
    if (strcasecmp(gCmdListObj[i].cmd, argv[0]) == 0) {
      printf("Calling %s function with argc=%d\n", gCmdListObj[i].cmd, argc);
      gCmdListObj[i].cbf(argc, argv);  // 해당 명령어의 처리 함수 호출
      return true;
    }
  }

  printf("Unsupported command..\r\n");  // 지원되지 않는 명령어인 경우 에러 메시지 출력

#else
  char *ptr = strtok(buf, " ");    //첫번째 strtok 사용.
  while (ptr != NULL)              //ptr이 NULL일때까지 (= strtok 함수가 NULL을 반환할때까지)
  {
      printf("%s\n", ptr);         //자른 문자 출력
      ptr = strtok(NULL, " ");     //자른 문자 다음부터 구분자 또 찾기
  }
  uart_regcbf(cli_parser);  // UART 수신 콜백 함수 등록
#endif

  return true;
}

// CLI 초기화 함수 (UART 수신 콜백 함수 등록)
void cli_init(void)
{
  uart_regcbf(cli_parser);
}
