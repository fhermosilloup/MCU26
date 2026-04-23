/*
 * serial.h
 *
 *  Created on: Apr 22, 2026
 *      Author: ferna
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>
#include "stm32f103_hal.h"

/* Configurations ---------------------------------*/
// Size = 2^SERIAL_BUFFER_DEPTH
#define SERIAL_BUFFER_DEPTH 5

// Make sure to select just one
#define SERIAL_USE_USART1
// #define SERIAL_USE_USART2
// #define SERIAL_USE_USART3

#define SERIAL_USE_RX
//#define SERIAL_USE_TX

/* Prototype function ----------------------------*/
void serial_init(void);

#ifdef SERIAL_USE_TX
uint16_t serial_write(uint8_t *data, uint16_t len);
#endif

#ifdef SERIAL_USE_RX
uint16_t serial_available(void);
int16_t serial_read(void);
uint16_t serial_request(uint8_t *data, uint16_t len);
#endif

#endif /* SERIAL_H_ */
