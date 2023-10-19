/*
 * dht11.c
 *
 *  Created on: Oct 4, 2023
 *      Author: User
 */

#include "dht11.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim2;

static void DHT11_Delay(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

static void DHT11_Start(void) {
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    DHT11_Delay(18000); // 최소 18ms의 LOW 신호를 유지
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    DHT11_Delay(30); // HIGH 신호 후 대기 시간
}

static uint8_t DHT11_Check_Response(void) {
    // DHT11 센서가 LOW 신호를 보내고 HIGH 신호를 받으면 센서가 응답한 것
    if (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) {
        DHT11_Delay(80); // LOW 신호 유지
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) {
            DHT11_Delay(50); // HIGH 신호 유지
            return 1;
        }
    }
    return 0;
}

static uint8_t DHT11_Read_Bit(void) {
    uint8_t result = 0;
    while (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)); // HIGH 신호까지 대기
    DHT11_Delay(40); // HIGH 신호 유지 후, LOW 신호까지 대기
    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) {
        result = 1; // HIGH 신호 감지됨
    }
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)); // LOW 신호까지 대기
    return result;
}

static uint8_t DHT11_Read_Byte(void) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte |= (DHT11_Read_Bit() << (7 - i));
    }
    return byte;
}

void DHT11_Init(void) {
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET); // 초기에 HIGH로 설정
}

uint8_t DHT11_ReadData(DHT11_Data_TypeDef* data) {
    uint8_t data_buffer[5];

    DHT11_Start();
    if (!DHT11_Check_Response()) {
        return 0; // 응답이 없으면 실패
    }

    // 센서로부터 40비트 데이터를 읽음
    for (int i = 0; i < 5; i++) {
        data_buffer[i] = DHT11_Read_Byte();
    }

    // 데이터 디코딩
    data->humidity_integer = data_buffer[0];
    data->humidity_decimal = data_buffer[1];
    data->temperature_integer = data_buffer[2];
    data->temperature_decimal = data_buffer[3];

    // 체크섬 검사
    if (data->humidity_integer + data->humidity_decimal + data->temperature_integer + data->temperature_decimal == data_buffer[4]) {
        return 1; // 성공적으로 데이터를 읽었음
    }
    return 0; // 데이터 읽기 실패
}
