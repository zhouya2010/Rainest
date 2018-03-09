/*
 * exboard.h
 *
 * Created: 2016/5/26 8:57:38
 *  Author: Administrator
 */ 


#ifndef EXBOARD_H_
#define EXBOARD_H_

#include <stdint.h>

#define CMD_WIRTE	0x02
#define CMD_READ	0x01

#define CTR_OPEN  0x01
#define CTR_CLOSE 0x02
#define CTR_DETECT 0x03

#define HEAD 0x5AA5

typedef  struct exboard_state {
	uint16_t head;
	uint8_t addr;
	uint16_t packet_id;
	uint8_t cmd;
	uint8_t data_len;
	uint8_t data[20];
	uint16_t crc;
	}ExboardState;

extern uint8_t exboard_detect_count;

void exboard_open(uint8_t *valves, int len);
void exboard_close(uint8_t valve);
void exboard_detection(void);
void exboard_receive(void);
void exboard_receive2(void);
#endif /* EXBOARD_H_ */