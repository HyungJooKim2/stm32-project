/*
 * dht11.c
 *
 *  Created on: Aug 11, 2023
 *      Author: User
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"
#include "key.h"
#include "dht11.h"

extern TIM_HandleTypeDef htim14;
#define D_TIM_HANDLE	&htim14

static uint16_t get_time(void)
{
	return (uint16_t)__HAL_TIM_GET_COUNTER(D_TIM_HANDLE);
}

static void set_time(uint16_t time)
{
	__HAL_TIM_SET_COUNTER(D_TIM_HANDLE, time);
}

// PC8 핀 출력을 1로
// 하위 16비트가 핀을 1로 설정하는 비트임
// 예로 PA3 핀을 1로 설정하려면 0x00000004; 를 BSRR 레지스터에 입력하면 됨.
static void pin_high(void)
{
	GPIOC->BSRR = 0x0000100;
}

// PC8 핀 출력을 0으로
// 상위 16비트가 핀을 0으로 설정하는 비트임
// 예는 pin_high와 같은 개념임
static void pin_low(void)
{
	GPIOC->BSRR = 0x01000000;
}

// PC8 핀 상태를 읽음
// 하위 16비트 해당 핀은 비트와 동일
static uint8_t pin_get(void)
{
	return (uint8_t)((GPIOC->IDR & 0x00100) >> 8);
}


// us단위 대기..
// 16비트이므로 최대 (65535 - 1)us 만큼 가능
static void pin_out_wait(uint16_t time)
{
	volatile uint16_t start, curr;

	start = get_time();
	while (1) {
		curr = get_time();
		if ((uint16_t)(curr - start) > time) break;
	}
}

// rising edge면 	0
// falling edge면 	1
// rising edge면 low level 유지 시간을 time으로 return
// falling edge면 high level 유지 시간을 time으로 return
// DHT11의 각 시그널의 길이는 80us 넘지 않음
// 150us 이상 넘으면 센서 응답의 끝으로 볼 수 있음
static int8_t pin_get_change(uint16_t *time)
{
	volatile uint8_t pin_prev;
	volatile uint16_t start;

	pin_prev= pin_get();     	// 현재 핀 상태 저장
	start = get_time();				// 시작하는 시간 저장

	while (1) {
		if (pin_prev != pin_get()) { 		// 핀 상태가 변하는가?
			*time = get_time() - start;		// 변했을 때 핀의 상태가 얼마나 	유지된 시간
			break;
		} else {
			if (get_time() - start > 150) return -1;		// 그렇지 않고 150us이상 변화가 없으면 time-out
		}
	}

	return !pin_get();      // 핀 상태 return
}

// sts는 DHT11에서 출력되는 signal이 low인지 high인지
// time은 signal이 유지되는 시간
typedef struct {
	int8_t sts;
	uint16_t time;
} PIN_T;

void htc11_init(void)
{
	printf("Push User Button to get a data from DHT11..\r\n");
	HAL_TIM_Base_Start(D_TIM_HANDLE);
}

void htc11_proc(void)
{
	PIN_T pin_sts[100];   // DHT11에서 출력되는 되는 시그널을 저장하기 위한 변수
	uint8_t data[5];			// pin_sts에 저장된 시그널을 분석해서 바이트 단위로 저장하기 위한 변수
	uint8_t i, j, k, l;
	int8_t err;
	uint8_t checksum;
	{
		// 1초이상 기다려서 읽어야 한다고 함...
		// 데이터 쉬트에 그렇게 써져 있음.
		if (getkey() == 1) {

			printf("HDT11 Sensing Start!\n");
			memset(data, 0, 5);
			err = 0;

			//set_time(0);
			//default : output, open_drain, high signal
			//pin_mode_out();       // open drain이라 pin in/out mode 변경 필요 없음
			// 디바이스(DHT11)에 시작 요청
			pin_low();
			pin_out_wait(18000);
			pin_high();
			pin_out_wait(40);

			//pin_mode_in();
			//set_time(0);
			//start = get_time();
			// 디바이스가 여기서부터 응답한 값들임
			for (i=0; i<83; i++) { //83개만 읽으면 됨... : 테스트 결과
				pin_sts[i].sts = pin_get_change(&pin_sts[i].time);
				if (pin_sts[i].sts == -1) {
					err = -1;      // 센서 응답 끝이나 응답이 없을 때
					break;
				}
			}

//			for (i=0; i<83; i++) { //83개만 읽으면 됨... : 테스트 결과
//				printf("[%d][%05d]\r\n", pin_sts[i].sts, pin_sts[i].time);
//			}

			//pin_high();

			//pin_mode_out();

			printf("err code = %d\n", err);
			printf("i = %d\n", i);

			// 테스트 해 보니 DHT11은 83개 signal 출력
			if (i < 83) {
				printf("read bit error....\n");
			} else
			{
				for (j=0; j<i; j++) {
					printf("%2d, %2d, %6d\n", j, pin_sts[j].sts, pin_sts[j].time);
				}

				l = 0; k = 0;

				// 첫 두 비트는 device의 응답 시작을 의미함

				// 하나의 비트를 low와 high의 길이로 표현
				// low 상태의 길이는 거의 동일..(약 50us 근처)
				// high 상태의 길이가 달라짐
				// low(50us) 다음 high의 길이가 25us이면 '0'
				// low(50us) 다음 high의 길이가 70us이면 '1'
				// 8비트씩 총 5바이트 data[]에 집어 넣는다.
				// 시작은 low[0] high[1] low[2] high[3]  <-- low[2]부터 실제 데이터로 사용할 비트인데..
				// low 상태는 분석할 필요가 없다.
				// 따라서 high만 분석하면 됨 그래서 배열에서 [3]부터 시작
				for (j=3; j<i; j+=2) {
					if (pin_sts[j].time > 50) {
						// MSB first : 첫비트가 제일 먼저옴
						data[l] |= (0x80 >> k);
					}
					k++;
					k %= 8;	// 8비트 단위

					if (k == 0) {  // k가 0이면 다음 바이트
						l++;
						if (l >= 5) break;    // 5바이트 넘으면 끝.
					}
				}


				// 결과를 분석해 보니 이렇게 이루어진 것 같고
				// data[0], data[1] -> 습도
				// data[2], data[3] -> 온도
				// data[4] -> 체크섬

				// 대충이래 보임
				// data[0]이 습도의 정수부, data[1]이 습도의 소수부
				// data[2]가 온도의 정수부, data[3]이 온도의 소수부
				// 같다.
				printf("result------\n");
				for (i=0; i<l; i++) {
					printf("[%3d]%3d,%02x\n", i, data[i], data[i]);
				}

				checksum = 0;
				for (i=0; i<4; i++) {
					checksum += data[i];
				}

				if (checksum != data[4]) {
					printf("Checksum error\n");
				} else {
					printf("Checksum ok!\n");

					printf("Humidity:%d.%d%%\n", data[0], data[1]);
					printf("Temperature:%d.%dC\n", data[2], data[3]);
				}
			}
		}
	}
}
