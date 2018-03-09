/*
 * usart1.h
 *
 * Created: 2016/5/24 10:38:25
 *  Author: Administrator
 */ 


#ifndef USART1_H_
#define USART1_H_

#include <stdint.h>

void usart1_init(void);
void usart1_send(uint8_t *data);
int usart1_send_n(uint8_t *data, uint16_t len);
uint8_t usart1_getchar(void);
int usart1_char_waiting(void);
uint16_t usart1_get_available(void);
int usart1_read(uint8_t *buffer, uint16_t len);

#endif /* USART1_H_ */
