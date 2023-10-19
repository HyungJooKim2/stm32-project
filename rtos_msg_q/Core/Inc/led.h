/*
 * led.h
 *
 *  Created on: Aug 7, 2023
 *      Author: User
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

void led_init(void);
bool led_onoff(uint8_t no, uint8_t onoff);

#ifdef __cplusplus
}
#endif

#endif /* INC_LED_H_ */
