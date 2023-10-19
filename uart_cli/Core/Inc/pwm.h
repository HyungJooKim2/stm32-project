/*
 * pwm.h
 *
 *  Created on: Aug 7, 2023
 *      Author: User
 *
 */

#ifndef INC_PWM_H_
#define INC_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

void pwm_init(void);
bool pwm_dimming(uint8_t no, uint8_t duty);

#ifdef __cplusplus
}
#endif


#endif /* INC_PWM_H_ */
