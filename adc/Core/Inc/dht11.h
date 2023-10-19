/*
 * dht11.h
 *
 *  Created on: Oct 4, 2023
 *      Author: User
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_1

typedef struct {
    uint8_t humidity_integer;
    uint8_t humidity_decimal;
    uint8_t temperature_integer;
    uint8_t temperature_decimal;
} DHT11_Data_TypeDef;

void DHT11_Init(void);
uint8_t DHT11_ReadData(DHT11_Data_TypeDef* data);

#ifdef __cplusplus
}
#endif

#endif /* INC_DHT11_H_ */
