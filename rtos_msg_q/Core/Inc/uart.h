/*
 * uart.h
 *
 *  Created on: Jul 28, 2023
 *      Author: User
 */

#ifndef INC_UART_H_
#define INC_UART_H_

typedef bool (*UART_CBF)(uint8_t *);

//#define D_BUF_MAX	50

//typedef struct {
//  uint8_t buf[D_BUF_MAX+1];
//  uint8_t idx;
//} MSG_BUFQ_T;

enum {
	E_MSG_UART2_TX = 0x10,
	E_MSG_UART2_TX_N_CHECK,
	E_MSG_UART2_RX,
	E_MSG_UART2_RX_CHECK,

	E_MSG_UART3_TX = 0x20,
	E_MSG_UART3_RX,

	E_MSG_BT_TX = 0x30,
	E_MSG_BT_RX,


	E_MSG_LCD = 0x40,

	E_MSG_MAX = 0xFF
};

typedef struct __PACKED {
	uint8_t id;
	uint8_t len;
	uint8_t *pData;
	uint32_t timeout;
} MSG_T;

typedef struct __PACKED {
	uint16_t cmd;
	uint8_t len;
	uint8_t data[1];
} PKT_T;

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(void);
void uart_regcbf(uint8_t idx, UART_CBF cbf);
void uart_QPut(const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);
bool slip_tx(const uint8_t *pTx, const uint8_t Len, uint8_t *pEnBuf, uint8_t *pEnLen);

#ifdef __cplusplus
}
#endif

#endif /* INC_UART_H_ */
