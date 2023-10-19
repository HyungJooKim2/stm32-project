/*
 * bluetooth.h
 *
 *  Created on: Aug 24, 2023
 *      Author: User
 */

#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_

#ifdef __cplusplus
extern "C" {
#endif

void bt_init(void);
void bt_QPut(const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* INC_BLUETOOTH_H_ */
