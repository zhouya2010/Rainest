/*
 * rw_buffer.h
 *
 * Created: 2016/4/29 9:41:52
 *  Author: Administrator
 */ 


#ifndef RW_BUFFER_H_
#define RW_BUFFER_H_

#include <stdint.h>

void buffer_init(void);

int16_t buffer_out_write(uint8_t * src , uint16_t len);
int16_t buffer_out_read(uint8_t * buf, uint16_t len);
int16_t buffer_out_available(void);

int16_t buffer_in_write(uint8_t * src , uint16_t len);
int16_t buffer_in_read(uint8_t * buf, uint16_t len);
int16_t buffer_in_available(void);
uint8_t buffer_in_read_byte(void);
void buffer_in_write_byte(uint8_t data);

void buffer_out_fresh(void);
#endif /* RW_BUFFER_H_ */