/*
 * bluetooth.c
 *
 *  Created on: Aug 24, 2023
 *      Author: User
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "cmsis_os.h"
#include "main.h"
#include "uart.h"
#include "bluetooth.h"


//#define D_TX_MAX	100
//static uint8_t gBufObj[2][D_TX_MAX];

//static UART_CBF uart_cbf[3];
static osMessageQueueId_t msgBufQ_handle;
static osThreadId_t btTask_handle;

static const osThreadAttr_t btTask_attributes = {
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

static void btProc_Task(void *arg);

void bt_init(void)
{
  btTask_handle = osThreadNew(btProc_Task, NULL, &btTask_attributes);
  if (btTask_handle == NULL) {
  	printf("%s : %d\r\n", __func__, __LINE__);
  	while (1);
  }

  msgBufQ_handle = osMessageQueueNew(5, sizeof(MSG_T), NULL);
  if (msgBufQ_handle == NULL) {
  	printf("%s : %d\r\n", __func__, __LINE__);
  	while (1);
  }

  printf("Bluetooth Initialized...\r\n");
}

void bt_QPut(const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
	osMessageQueuePut(msgBufQ_handle, msg_ptr, msg_prio, timeout);
}

void (*cbf)(void);
uint8_t s_state = 0;
char msg_buf[101];

void func_s0(void);
void func_s1(void);
//void func_s2(void);
//void func_s3(void);
//void func_s4(void);


void func_s0(void)
{
	//char buf[50];
	MSG_T qTx, qRx;

	if (s_state == 0) {
		printf("BT status : S0\r\n");

		qTx.id = E_MSG_UART2_TX_N_CHECK;
		sprintf(msg_buf, "AT");
		qTx.len = strlen(msg_buf);
		qTx.timeout = 1000;
		qTx.pData = (uint8_t *)msg_buf;
		uart_QPut(&qTx, 0, osWaitForever);

		s_state++;
	} else {
		osStatus_t sts = osMessageQueueGet(msgBufQ_handle, &qRx, NULL, osWaitForever);
		if (sts == osErrorTimeout) {
			s_state = 0;
		} else if (sts == osOK) {
			switch (qRx.id) {
				case E_MSG_BT_RX : {
					if (memcmp(qRx.pData, "OK", 2) == 0) {
						printf("BT status : S1\r\n");
						cbf = func_s1;  // callback function
						s_state = 0;
						return;
					}
				} break;
			}  // switch

			s_state = 0;
		} // else if (sts == osOK) {
	}
}

void func_s1(void)
{
	MSG_T qTx, qRx;

	osStatus_t sts = osMessageQueueGet(msgBufQ_handle, &qRx, NULL, 300);

	if (sts == osErrorTimeout) {
		qTx.id = E_MSG_UART2_RX_CHECK;
		qTx.timeout = 1;
		uart_QPut(&qTx, 0, 0);
	} else if (sts == osOK) {
		switch (qRx.id) {
			case E_MSG_BT_RX : { // loop back
				qTx.id = E_MSG_UART2_TX;
				qTx.len = qRx.len;
				qTx.pData = qRx.pData;
				uart_QPut(&qTx, 0, 0);
				qTx.pData[qTx.len] = '\0';
				//printf("%s\r\n", qTx.pData);
			} break;
		}
	}
}


void btProc_Task(void *arg)
{
	cbf = func_s0;

	printf("Bluetooth Thread Start...\r\n");

	osDelay(500);

	for (;;) {
		cbf();
	}
}
