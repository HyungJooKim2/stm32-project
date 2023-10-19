/*
 * i2c_HD44780.h
 *
 *  Created on: Aug 16, 2023
 *      Author: User
 */

#ifndef __I2C_HD44780__H__
#define __I2C_HD44780__H__

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init (void);   // LCD를 초기화
//void lcd_send_cmd (char cmd);  // LCD에 명령을 전송
//void lcd_send_data (char data);  // LCD에 데이터 전송
void lcd_disp_on(void);  // LCD 디스플레이 활성화
void lcd_disp_off(void);  // LCD 디스플레이 비활성화
void lcd_home(void);  // 커서를 홈 위치로 이동
void lcd_clear_display(void);  // LCD 디스플레이 클리어
void lcd_locate(uint8_t row, uint8_t column);  // 커서를 지정한 행과 열로 이동
void lcd_printchar(unsigned char ascode);  // 문자를 LCD에 출력
void lcd_print_string (char *str);  // 문자열을 LCD에 출력
void lcd_printf(const char *fmt, ...);  // 서식을 지정하여 LCD에 출력

#ifdef __cplusplus
}
#endif

#endif
