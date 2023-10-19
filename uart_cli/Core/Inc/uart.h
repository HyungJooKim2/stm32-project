/*
 * uart.h
 *
 *  Created on: Jul 28, 2023
 *      Author: User
 */

#ifndef INC_UART_H_
#define INC_UART_H_

typedef bool (*UART_CBF)(uint8_t *);

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(void);
void uart_regcbf(UART_CBF cbf);
void uart_proc(void);
void UART_ParseCommand(char *command);

#ifdef __cplusplus
}
#endif

#endif /* INC_UART_H_ */
