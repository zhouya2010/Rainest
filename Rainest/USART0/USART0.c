/*
 * USART0.c
 *
 * Created: 2014/5/29 10:28:05
 *  Author: ZLF
 */ 
#include "USART0.h"
#include "ring_buffer.h"
#include "uip.h"
#include "rw_buffer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "typedef.h"



uint8_t *in_buffer = &uip_buf[0];
struct ring_buffer ring_buffer_in;


void set_usart_baud(uint16_t baud)
{
	UBRR0H = 0x00;
	switch(baud)
	{
		case BAUD_9600:
		UBRR0L = 0x9b;
		break;
		
		case BAUD_19200:
		UBRR0L = 0x4D;
		break;
		
		case BAUD_38400:
		UBRR0L = 0x26;
		break;
		
		default:
		UBRR0L = 0x26;
		break;
	}
}



void USART0_init(void)
{
// 	UBRR0 = 0x009b; //9600
// 	UBRR0 = 0x004D;	//19200
// 	UBRR0 = 0x000C;	//115200
	UBRR0 = 0x0026;	//38400
	UCSR0A |= (1 << U2X0);
	UCSR0A &= ~(1 << FE0);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (0 << USBS0) |
					(0 << UPM01) | (0 << UPM00) | (0 << UMSEL01) |
					(0 << UMSEL00);
	SREG |= 0x80;
	
//	ring_buffer_in = ring_buffer_init(in_buffer, BUFFER_SIZE);
}

/**
 * \brief Function to check if we have a char waiting in the UART receive buffer
 *
 * \retval true if data is waiting
 * \retval false if no data is waiting
 */
int usart_char_waiting(void)
{
	return !ring_buffer_is_empty(&ring_buffer_in);
}

uint16_t usart_get_available(void)
{
	if (ring_buffer_in.write_offset < ring_buffer_in.read_offset)
	{
		return (ring_buffer_in.write_offset + ring_buffer_in.size - ring_buffer_in.read_offset);
	}
	else
	{
		return (ring_buffer_in.write_offset - ring_buffer_in.read_offset);
	}
}


void USART0_send(char *data)
{
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B |= (1 << TXEN0);
	while(*data != '\0')
	{
		while ( !( UCSR0A & (1<<UDRE0)) );
		UDR0 = *data;
		data++;
	}
	UCSR0B |= (1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
}


int USART0_send_n(char *data, int len)
{
	int count = 0;
	UCSR0B &= ~(1 << RXEN0);
	UCSR0B |= (1 << TXEN0);
	while(len)
	{
		while ( !( UCSR0A & (1<<UDRE0)) );
		UDR0 = *data;
		data++;
		len--;
		count++;
	}
	UCSR0B |= (1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
	return count;
}

ISR(USART0_RX_vect)		//串口接收中断处理函数
{
	buffer_in_write_byte(UDR0);
}


/**
 * \brief Function for getting a char from the UART receive buffer
 *
 * \retval Next data byte in receive buffer
 */
uint8_t usart_getchar(void)
{
	return ring_buffer_get(&ring_buffer_in);
}

/*
*	读取len个字节长度，返回读取到的长度
*
*/
int usart_read(uint8_t *buffer, uint16_t len)
{
	uint16_t i = 0;
	uint8_t data;
	
	while(i < len)
	{
		if (!usart_char_waiting())
		{
			break;
		}
		
		data = usart_getchar();
		
		*buffer = data;
		buffer++;
		i++;
	}
	return i;
}


uint16_t get_read_offset(void)
{
	return ring_buffer_in.read_offset;
}

uint8_t * get_start_ptr(uint16_t offset)
{
	if (offset < ring_buffer_in.size)
	{
		return &ring_buffer_in.buffer[offset];
	}
	else
	{
		return NULL;
	}
}


void ring_buffer_copy(char *dest, uint16_t offset, uint16_t len )
{
	while(len)
	{
		*dest = in_buffer[offset];
		dest++;
		offset++;
		if (offset > BUFFER_SIZE)
		{
			offset = 0;
		}
		len--;
	}
}

void usart_buffer_reset(void)
{
	ring_buffer_in.read_offset = 0;
	ring_buffer_in.write_offset = 0;
}