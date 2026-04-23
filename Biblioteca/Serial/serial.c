/*
 * serial.c
 *
 *  Created on: Apr 22, 2026
 *      Author: ferna
 */

#include "serial.h"
#include <stdlib.h>

#define SERIAL_BUF_SIZE	(1UL << SERIAL_BUFFER_DEPTH)

#if defined(SERIAL_USE_USART1)
#define Serial		USART1
#define Serial_IRQn	USART1_IRQn
#define Serial_IRQHandler USART1_IRQHandler

#elif defined(SERIAL_USE_USART2)
#define Serial	USART2
#define Serial_IRQn	USART2_IRQn
#define Serial_IRQHandler USART2_IRQHandler

#elif defined(SERIAL_USE_USART3)
#define Serial	USART3
#define Serial_IRQn	USART3_IRQn
#define Serial_IRQHandler USART3_IRQHandler

#else
#error "No USART defined"
#endif


#if defined(SERIAL_USE_TX) && defined(SERIAL_USE_RX)
#define SERIAL_MODE	UART_MODE_TX_RX
#elif defined(SERIAL_USE_TX)
#define SERIAL_MODE	UART_MODE_TX
#else
#define SERIAL_MODE	UART_MODE_RX
#endif

typedef struct {
#ifdef SERIAL_USE_TX
 struct {
	 volatile uint8_t buffer[SERIAL_BUF_SIZE];
	 volatile uint16_t wridx;	// Write index
	 volatile uint16_t rdidx;	// Read index
 } tx;
#endif

#ifdef SERIAL_USE_RX
 struct {
 	 volatile uint8_t buffer[SERIAL_BUF_SIZE];
 	 volatile uint16_t wridx;	// Write index
 	 volatile uint16_t rdidx;	// Read index
  } rx;
#endif
} circular_buffer_t;



circular_buffer_t SerialBuffer;

void serial_init(void)
{
	uart_init(Serial, 9600, SERIAL_MODE, UART_PARITY_NONE, UART_STOP_1);
#ifdef SERIAL_USE_TX
	SerialBuffer.tx.rdidx = 0;
	SerialBuffer.tx.wridx = 0;
	uart_set_interrupt(Serial, UART_IRQ_TX, IRQ_ENABLE);
#endif
#ifdef SERIAL_USE_RX
	SerialBuffer.rx.rdidx = 0;
	SerialBuffer.rx.wridx = 0;
	uart_set_interrupt(Serial, UART_IRQ_RX, IRQ_ENABLE);
#endif
	nvic_enable_irq(Serial_IRQn);


}


#ifdef SERIAL_USE_TX
uint16_t serial_write(uint8_t *data, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		uint16_t next_idx = (SerialBuffer.tx.wridx + 1) & (SERIAL_BUF_SIZE-1);

		// buffer full
		if (next_idx == SerialBuffer.tx.rdidx)
		{
			return i; // failure
		}

		SerialBuffer.tx.buffer[SerialBuffer.tx.wridx++] = data[i];
		SerialBuffer.tx.wridx &= SERIAL_BUF_SIZE-1;

		// habilitar interrupción TX
		Serial->CR1 |= USART_CR1_TXEIE;
	}

	return len;
}
#endif


#ifdef SERIAL_USE_RX
uint16_t serial_available(void)
{
	return (uint16_t)(SerialBuffer.rx.wridx - SerialBuffer.rx.rdidx);
}

int16_t serial_read(void)
{
	if (SerialBuffer.rx.wridx == SerialBuffer.rx.rdidx)
	{
		return -1; // no data
	}

	uint8_t data = SerialBuffer.rx.buffer[SerialBuffer.rx.rdidx++];
	SerialBuffer.rx.rdidx &= SERIAL_BUF_SIZE-1;

	return data;
}

uint16_t serial_request(uint8_t *data, uint16_t len)
{
	uint16_t i = 0;

	while (i < len)
	{
		// Esperar a que haya datos disponibles
		if (SerialBuffer.rx.wridx == SerialBuffer.rx.rdidx)
		{
			// No hay más datos en buffer en este instante
			break;
		}

		data[i++] = SerialBuffer.rx.buffer[SerialBuffer.rx.rdidx++];
		SerialBuffer.rx.rdidx &= SERIAL_BUF_SIZE-1;
	}

	return i; // devuelve cuántos bytes se leyeron realmente
}
#endif

/* IRQ Handlers */
void Serial_IRQHandler(void)
{
#ifdef SERIAL_USE_RX
	/* Check if new data has been received */
	if(Serial->SR & USART_SR_RXNE)
	{
		// The RXNE flag is cleared by reading DR reg
		SerialBuffer.rx.buffer[SerialBuffer.rx.wridx++] = Serial->DR;
		SerialBuffer.rx.wridx &= SERIAL_BUF_SIZE-1;
		GPIOC->ODR ^= (1 << 13);
	}
#endif

#ifdef SERIAL_USE_TX
	/* Check if we need to write data */
	if(USART1->SR & USART_SR_TXE)
	{
		if(SerialBuffer.tx.wridx != SerialBuffer.tx.rdidx)
		{
			Serial->DR = SerialBuffer.tx.buffer[SerialBuffer.tx.rdidx++];
			SerialBuffer.tx.rdidx &= SERIAL_BUF_SIZE-1;
		}
		else
		{
			// Disable TX interrupt
			Serial->CR1 &= ~USART_CR1_TXEIE;
		}
	}
#endif
}

