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
#include <ctype.h>

#include "cmsis_os.h"
#include "main.h"
#include "bluetooth.h"
#include "uart.h"

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

#define D_TX_MAX	100
static uint8_t gBufObj[2][D_TX_MAX *2 + 2];

static UART_CBF uart_cbf[3];
static volatile uint8_t rxd[2];
static osMessageQueueId_t msgBufQ_handle;
static osThreadId_t uartTask_handle;
static osTimerId_t uartTimer_handle;

static const osThreadAttr_t uartTask_attributes = {
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

static void uartProc_Task(void *arg);
static void uartProc_Timer(void *arg);

void uart_init(void)
{
  uartTask_handle = osThreadNew(uartProc_Task, NULL, &uartTask_attributes);
  if (uartTask_handle == NULL) {
  	printf("%s : %d\r\n", __func__, __LINE__);
  	while (1);
  }

  msgBufQ_handle = osMessageQueueNew(5, sizeof(MSG_T), NULL);
  if (msgBufQ_handle == NULL) {
  	printf("%s : %d\r\n", __func__, __LINE__);
  	while (1);
  }

  uartTimer_handle = osTimerNew(uartProc_Timer, osTimerOnce, NULL, NULL);
  if (uartTimer_handle == NULL) {
  	printf("%s : %d\r\n", __func__, __LINE__);
  	while (1);
  }

  HAL_UART_Receive_IT(&huart2, (uint8_t *)&rxd[0], 1);
  HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxd[1], 1);

  printf("UART Initialized...\r\n");
}

void uart_regcbf(uint8_t idx, UART_CBF cbf)
{
  uart_cbf[idx] = cbf;
}

void uart_QPut(const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
	osMessageQueuePut(msgBufQ_handle, msg_ptr, msg_prio, timeout);
}

static void uartProc_Task(void *arg)
{
	//static uint8_t msg_buf[2][202];
	MSG_T qRx, qTx;

	for (;;) {
		osMessageQueueGet(msgBufQ_handle, &qRx, NULL, osWaitForever);

		switch (qRx.id) {

			case E_MSG_UART2_TX :
				HAL_UART_Transmit(&huart2, qRx.pData, qRx.len, 0xffff);
				break;

			case E_MSG_UART2_TX_N_CHECK :
				HAL_UART_Transmit(&huart2, qRx.pData, qRx.len, 0xffff);
				osTimerStart(uartTimer_handle, qRx.timeout);
				break;

			case E_MSG_UART2_RX_CHECK :
				//if (qRx.timeout == osWaitForever) break;
				osTimerStart(uartTimer_handle, qRx.timeout);
				break;

			case E_MSG_UART2_RX : {
				if (qRx.len != 0) {
					qRx.id = E_MSG_BT_RX;
					bt_QPut(&qRx, 0, 0);
				}
				//printf("[URX2] %s\r\n", qRx.pData);
			} break;

			case E_MSG_UART3_TX : {
				printf("E_MSG_UART3_TX\r\n");
			} break;

			case E_MSG_UART3_RX : {

//				qTx.id = E_MSG_UART3_TX;
//				osMessageQueuePut(msgBufQ_handle, &qTx, 0, 0);

//
//				printf("[CMD] : %04x\r\n", pPkt->cmd);
//				printf("[LEN] : %02x\r\n", pPkt->len);
//				printf("[DAT] : ");
//
//				for (int i=0; i<pPkt->len; i++) {
//					printf("%02x", pPkt->data[i]);
//				}
//
//				printf("\r\n[    ] : ");
//
//				for (int i=0; i<pPkt->len; i++) {
//					uint8_t ch = pPkt->data[i];
//					if (isalnum(ch) || isspace(ch)) printf("%c ", ch);
//					else printf(". ");
//				}
//				printf("\r\n");

				PKT_T *pPkt = (PKT_T *)qRx.pData;

				switch (pPkt->cmd) {
					case 0x0001 : { // cli
						pPkt->data[pPkt->len] = '\0'; // string
						if (uart_cbf[0] != NULL) uart_cbf[0](pPkt->data);
					} break;

					case 0x0002 : { // bluetooth
						pPkt->data[pPkt->len] = '\0';
						printf("%s\r\n", pPkt->data);
						HAL_UART_Transmit(&huart2, pPkt->data, pPkt->len, 0xffff);
					} break;

					case 0x0003 : { // LCD
					} break;

					default : {

					}
				}
			} break;
		}

		//PKT_T *pPkt = (PKT_T *)q.buf;
//		printf("[CMD] : %04x\r\n", pPkt->cmd);
//		printf("[LEN] : %02x\r\n", pPkt->len);
//		printf("[DAT] : ");
//
//		for (int i=0; i<pPkt->len; i++) {
//			printf("%02x", pPkt->data[i]);
//		}
//		printf("\r\n");

		//		for (int i=0; i<q.idx; i++) {
		//			if (i%8 == 0) printf("\r\n");
		//			printf("[%02x:", q.buf[i]);
		//			if (isalnum(q.buf[i])) printf("%2c]", q.buf[i]);
		//			else printf("]");
		//		}
		//		printf("\r\n");

		//if (uart_cbf != NULL) uart_cbf(q.buf);

    //HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
	}
}

#define D_FEND	0xC0
#define D_FESC	0xDB
#define D_TFEND	0xDC
#define D_TFESC	0xDD

bool slip_tx(const uint8_t *pTx, const uint8_t Len, uint8_t *pEnBuf, uint8_t *pEnLen)
{
	uint8_t idx = 0;

	if (Len > D_TX_MAX) return false;
	if (pEnBuf == NULL) return false;
	//if (*pEnLen < (D_TX_MAX * 2 + 2)) return false;

	pEnBuf[idx++] = D_FEND;
	for (int i=0; i < Len; i++) {
		if (pTx[i] == D_FEND) {
			pEnBuf[idx++] = D_FESC;
			pEnBuf[idx++] = D_TFEND;
		} else if (pTx[i] == D_FESC) {
			pEnBuf[idx++] = D_FESC;
			pEnBuf[idx++] = D_TFESC;
		} else {
			pEnBuf[idx++] = pTx[i];
		}
	}
	pEnBuf[idx++] = D_FEND;
	*pEnLen = idx;

	return true;
}

bool slip_rx(MSG_T *pQ, uint8_t *pState, uint8_t rxd)
{
	switch (*pState) {
		case 0 : {
			if (rxd == D_FEND) {
				(*pState)++; // 1st FEND
				pQ->len = 0;
			}
		} break;

		case 1 : {
			if (rxd == D_FESC) {
				(*pState)++;
			} else if (rxd == D_FEND) {	// 2nd FEND
				if (pQ->len < 3) {
					*pState = 0;
				} else {
					*pState = 0;
					return true;
				}
			} else {
				pQ->pData[pQ->len++] = rxd;
			}
		} break;

		case 2 : {
			if (rxd == D_TFEND) {
				pQ->pData[pQ->len++]  = D_FEND;
				(*pState)--;
			} else if (rxd == D_TFESC) {
				pQ->pData[pQ->len++]  = D_FESC;
				(*pState)--;
			} else {
				(*pState)++; // wait for ending esc
			}
		} break;

		default : {
			if (rxd == D_FEND) *pState = 0;
		}
	}
	return false;
}

static MSG_T qTxUart2;

static void uartProc_Timer(void *arg)
{
	if (qTxUart2.len != 0) {
		qTxUart2.id = E_MSG_UART2_RX;
		osMessageQueuePut(msgBufQ_handle, &qTxUart2, 0, 0);
		qTxUart2.len = 0;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2) {
//		static uint8_t state = 0;

//  	bool ret;

  	qTxUart2.pData = (uint8_t *)&gBufObj[0];

  	qTxUart2.pData[qTxUart2.len] = rxd[0];
  	qTxUart2.len++;
  	qTxUart2.len %= D_TX_MAX;

//		if (rxd[0] == '\r' || rxd[0] == '\n') {
//			qTx.id = E_MSG_UART2_RX;
//			osMessageQueuePut(msgBufQ_handle, &qTx, 0, 0);
//			qTx.len = 0;
//		}

    HAL_UART_Receive_IT(&huart2, (uint8_t *)&rxd[0], 1);

	} else if (huart == &huart3) {
		static uint8_t state = 0;
  	static MSG_T q;
  	bool ret;

  	q.pData = (uint8_t *)&gBufObj[1];
  	ret = slip_rx(&q, &state, rxd[1]);
  	if (ret == true) {
			q.id = E_MSG_UART3_RX;
			osMessageQueuePut(msgBufQ_handle, &q, 0, 0);
		}

    HAL_UART_Receive_IT(&huart3, (uint8_t *)&rxd[1], 1);

//    p->buf[p->idx] = rxd;
//      //p->idx++;
//      //p->idx %= D_BUF_MAX;
//    if (p->idx < D_BUF_MAX) {
//			p->idx++;
//    }
//
//    if (rxd == '\r' || rxd == '\n') {
//			p->buf[p->idx] = '\0';
//	  	osMessageQueuePut(msgBufQ_handle, p, 0, 0); // osWaitForever
//	  	p->idx = 0;
//    }
  }
}

int _write(int file, char *ptr, int len)
{
  (void)file;

  static uint8_t slip_tx_buf[202];
  uint8_t Len;

  slip_tx((uint8_t *)ptr, (uint8_t)len, slip_tx_buf, &Len);
  HAL_UART_Transmit(&huart3, (uint8_t *)slip_tx_buf, Len, 0xffff);

  return len;
}

int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}
