/*
 * USART0.h
 *
 * Created: 2014/5/29 10:28:18
 *  Author: ZLF
 */ 


#ifndef USART0_H_
#define USART0_H_
#include <stdint.h>

#define BAUD_9600	0
#define BAUD_19200	1
#define BAUD_38400	2

extern uint8_t *in_buffer;

#define BUFFER_SIZE UIP_CONF_BUFFER_SIZE

void set_usart_baud(uint16_t baud);
void USART0_init(void);
void USART0_send(char *data);
int USART0_send_n(char *data, int len);
uint8_t readchar(void);
int usart_char_waiting(void);
uint8_t usart_getchar(void);
int usart_read(uint8_t *buffer, uint16_t len);
uint16_t get_read_offset(void);
uint8_t * get_start_ptr(uint16_t offset);
uint16_t usart_get_available(void);
void ring_buffer_copy(char *dest, uint16_t offset, uint16_t len );
void usart_buffer_reset(void);

#endif /* USART0_H_ */