/*
 * usart1.c
 *
 * Created: 2016/5/24 10:38:12
 *  Author: Administrator
 */ 
#include "usart1.h"
#include "ring_buffer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU              12000000UL
#endif
#include <util/delay.h>

#define USATR1_BUFFER_SIZE 512
static uint8_t usart1_buffer[USATR1_BUFFER_SIZE];
static uint8_t *in_buffer_ptr = &usart1_buffer[0];
static struct ring_buffer usart1_ring_buffer_in;



void usart1_init(void)
{
	UBRR1 = 0x0026;	//38400
	UCSR1A |= (1 << U2X1);
	UCSR1A &= ~(1 << FE1);
	UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10) | (0 << USBS1) |
	(0 << UPM11) | (0 << UPM10) | (0 << UMSEL11) |
	(0 << UMSEL10);
	SREG |= 0x80;
	usart1_ring_buffer_in = ring_buffer_init(in_buffer_ptr, USATR1_BUFFER_SIZE);

}


void usart1_send(uint8_t *data)
{
	while(*data != '\0')
	{
		while ( !( UCSR1A & (1<<UDRE1)) );
		UDR1 = *data;
		data++;
	}
}

int usart1_send_n(uint8_t *data, uint16_t len) //16.7.6_2  "control reaches end of non-void function"
{
	int i = 0;
	
	while(len)
	{
		while ( !( UCSR1A & (1<<UDRE1)) );
		UDR1 = *data;
		data++;
		len--;
		i++;
	}
	return i;
}

ISR(USART1_RX_vect)
{
	ring_buffer_put(&usart1_ring_buffer_in, UDR1);
}

/**
 * \brief Function for getting a char from the UART receive buffer
 *
 * \retval Next data byte in receive buffer
 */
uint8_t usart1_getchar(void)
{
	return ring_buffer_get(&usart1_ring_buffer_in);
}

/**
 * \brief Function to check if we have a char waiting in the UART receive buffer
 *
 * \retval true if data is waiting
 * \retval false if no data is waiting
 */
int usart1_char_waiting(void)
{
	return !ring_buffer_is_empty(&usart1_ring_buffer_in);
}

uint16_t usart1_get_available(void)
{
	if (usart1_ring_buffer_in.write_offset < usart1_ring_buffer_in.read_offset)
	{
		return (usart1_ring_buffer_in.write_offset + usart1_ring_buffer_in.size - usart1_ring_buffer_in.read_offset);
	}
	else
	{
		return (usart1_ring_buffer_in.write_offset - usart1_ring_buffer_in.read_offset);
	}
}


/*
*	读取len个字节长度，返回读取到的长度
*
*/
int usart1_read(uint8_t *buffer, uint16_t len)
{
	uint16_t i = 0;
	uint8_t data;
	
	while(i < len)
	{
		if (!usart1_char_waiting())
		{
			_delay_ms(1);
			if(!usart1_char_waiting())
			break;
		}
		
		data = usart1_getchar();
		
		*buffer = data;
		buffer++;
		i++;
	}
	return i;
}