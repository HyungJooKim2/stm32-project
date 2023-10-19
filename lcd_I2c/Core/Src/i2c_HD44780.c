/*
 * i2c_HD44780.c
 *
 *  Created on: Aug 16, 2023
 *      Author: User
 */

#include "main.h"
#include "stm32f4xx_hal.h"
#include "i2c_HD44780.h"

#include <stdarg.h>
#include <stdio.h>
extern I2C_HandleTypeDef hi2c1;

#define SLAVE_ADDRESS_LCD (0x27<<1) //I2C 주소를 지정 F429zi의 경우 0x27
/**
  * @brief  HD44780 LCD 모듈에 명령을 I2C를 통해 전송
  * @param  cmd: 전송할 명령어(최대 8비트까지 전송 가능)
  * @retval None
  */

static void lcd_send_cmd(char cmd)
{
  char data_u, data_l;	// 명령어를 상위 4비트와 하위 4비트로 분리하기 위한 변수 선언
  uint8_t data_t[4];     // I2C를 통해 전송할 데이터를 저장하는 배열 선언
  data_u = (cmd & 0xf0);        // 명령의 상위 4비트 추출
  data_l = ((cmd << 4) & 0xf0);  // 명령의 하위 4비트를 상위 4비트로 이동하여 추출
  data_t[0] = data_u | 0x0C;  //en=1, rs=0 -> en, rs 비트 추가, 데이터가 들어와 명령 처리 상태
  data_t[1] = data_u | 0x08;  //en=0, rs=0 -> enm rs 비트 추가, 대기 상태
  data_t[2] = data_l | 0x0C;  //en=1, rs=0
  data_t[3] = data_l | 0x08;  //en=0, rs=0
  /*
  명령어를 상위 하위로 구분하여 4비트로 나눈 이유는 데이터에 en, rs 값을 추가해 8비트로 만들고
  4비트 단위로 LCD 모듈에 전달해야 하니 4비트로 2번 나눠서 전달하는 방식이 효율적이기 때문
  */
  while (HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100) != HAL_OK)
  {
    // I2C로 데이터를 LCD 모듈에 전송하고, 성공할 때까지 대기
    // Timeout 오류 발생 시 Error_Handler() 호출
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
  }
}

/**
  * @brief  HD44780 LCD 모듈에 데이터를 I2C를 통해 전송
  * @param  data: 전송할 데이터
  * @retval None
  */
static void lcd_send_data(char data)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (data & 0xf0);
  data_l = ((data << 4) & 0xf0);
  data_t[0] = data_u | 0x0D;  //en=1, rs=0
  data_t[1] = data_u | 0x09;  //en=0, rs=0
  data_t[2] = data_l | 0x0D;  //en=1, rs=0
  data_t[3] = data_l | 0x09;  //en=0, rs=0

  while (HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100) != HAL_OK)
  {
    // Timeout 오류 발생 시 Error_Handler() 함수를 호출
    if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF)
    {
      Error_Handler();
    }
  }
}

/**
  * @brief  4-bit 모드에서 HD44780 LCD 모듈 초기화
  * @param  None
  * @retval None
  */
void lcd_init(void)
{
  lcd_send_cmd(0x33);   // 확장 명령 모드 설정
  lcd_send_cmd(0x32);   // 4비트 모드 설정
  lcd_send_cmd(0x28);   // 4-bit 모드, 2줄 디스플레이, 5x8 폰트 설정
  lcd_send_cmd(0x0C);   // 디스플레이 켜기, 커서 비활성화, 커서 깜빡임 비활성화
  lcd_send_cmd(0x06);   // 커서 이동 방향 설정 (커서를 오른쪽으로 이동, 화면 이동)
  lcd_send_cmd(0x02);   // 디스플레이 clear
  // LCD 모듈을 구동하기 전에 필요한 최소한의 지연 시간 설정
  HAL_Delay(200);
}

/**
  * @brief  디스플레이 활성화
  * @param  None
  * @retval None
  */
void lcd_disp_on(void)
{
  lcd_send_cmd(0x0C);
}

/**
  * @brief  디스플레이 비활성화
  * @param  None
  * @retval None
  */
void lcd_disp_off(void)
{
  lcd_send_cmd(0x08);
}

/**
  * @brief  커서를 홈 위치로 이동
  * @param  None
  * @retval None
  */
void lcd_home(void)
{
  lcd_send_cmd(0x02);
  HAL_Delay(2);
}

/**
  * @brief  LCD 모듈 디스플레이 clear
  * @param  None
  * @retval None
  */
void lcd_clear_display(void)
{
  lcd_send_cmd(0x01);
  HAL_Delay(2);
}

/**
  * @brief  특정 위치로 커서 이동
  * @param  row: 이동할 행 (1에서 4까지)
  * @param  column: 이동할 열 (1에서 20까지, 4x20 문자 디스플레이 기준)
  * @retval None
  */
void lcd_locate(uint8_t row, uint8_t column)
{
  column--;
  switch (row)
  {
    case 1:
      // 1행 주소로 커서를 설정하고 인덱스를 추가
      lcd_send_cmd(0x80);
      break;
    case 2:
      // 2행 주소로 커서를 설정하고 인덱스를 추가
      lcd_send_cmd(0xc0);
      break;
    case 3:
      // 3행 주소로 커서를 설정하고 인덱스를 추가
      lcd_send_cmd(column |= 0x94);
      break;
    case 4:
      // 4행 주소로 커서를 설정하고 인덱스를 추가
      lcd_send_cmd(column |= 0xd4);
      break;
    default:
      break;
  }
}

/**
  * @brief  LCD 모듈에 문자 출력
  * @param  출력할 문자의 ASCII 값
  * @retval None
  */
void lcd_printchar(unsigned char ascode)
{
	lcd_send_data(ascode);
}

/**
  * @brief  문자열을 LCD 모듈에 표시
  * @param  표시할 텍스트
  * @retval None
  */
void lcd_print_string(char *str)
{
	while (*str)
    lcd_send_data(*str++); // 문자열의 각 문자를 LCD 모듈에 전송하여 표시
}

/**
  * @brief  형식 지정된 문자열을 LCD에 출력하는 함수
  * @param  표준 정의 형식 지정 문자열
  * @param  가변 인수 목록
  * @retval None
  */
void lcd_printf(const char *fmt, ...)
{
  uint32_t i;
  uint32_t text_size, letter;
  static char text_buffer[32];
  va_list args;
  va_start(args, fmt);
  text_size = vsprintf(text_buffer, fmt, args); // 형식 지정 문자열을 텍스트 버퍼에 포맷

  // 문자열 처리
  for (i = 0; i < text_size; i++)
  {
    letter = text_buffer[i];
    if (letter == 10) // 개행 문자인 경우 처리 중단
      break;
    else
    {
      if ((letter > 0x1F) && (letter < 0x80)) // 인쇄 가능한 ASCII 문자 범위인 경우 출력
        lcd_printchar(letter);
    }
  }
}
