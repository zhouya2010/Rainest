/*
 * exboard.c
 *
 * Created: 2016/5/26 8:57:17
 *  Author: Administrator
 */ 
#include "usart1.h"  //16.7.7
#include "exboard.h"
#include "typedef.h"
#include "USART0.h"
#include "event_handle.h"
#include "LCD.h"               //16.7.7 implicit declaration of function

#ifndef F_CPU
#define F_CPU              12000000UL
#endif
#include <util/delay.h>

uint16_t packt_id = 0;
uint8_t exboard_detect_count;

//计算CRC校验：1021
uint16_t crc16(uint8_t *buf, int len)
{
  uint8_t j;
  uint8_t i;
  uint16_t t;
  uint16_t crc = 0;
  uint16_t res = 0;

  for(j = len; j > 0; j--)
  {
    //标准CRC校验
    crc = (crc ^ (((uint16_t) *buf) << 8));
    for(i = 8; i > 0; i--)
    {
      t = crc << 1;
      if(crc & 0x8000)
        t = t ^ 0x1021;
      crc = t;
    }
	//累加和校验
	crc += (unsigned int)(*buf);
    buf++;
  }
    
  res = (uint16_t)((crc / 256) << 8);
  res |= (uint8_t)(crc % 256);
  
  return res; 
}

uint16_t calculate_crc(ExboardState ex)
{
	return crc16(&ex.addr, 5 + ex.data_len);
}

static void exboard_send(ExboardState ex)
{
	usart1_send_n((uint8_t *)&ex, 7);					//16.7.8
	usart1_send_n((uint8_t *)&ex.data, ex.data_len);	//
	usart1_send_n((uint8_t *)&ex.crc, sizeof(ex.crc));	//

// 	USART0_send_n(&ex, 7);
// 	USART0_send_n(&ex.data, ex.data_len);
// 	USART0_send_n(&ex.crc, sizeof(ex.crc));
}

void exboard_open(uint8_t *valves, int len)
{
	ExboardState ex;
	
	packt_id++;
	
	ex.head = HEAD;
	ex.addr = 0x01;
	ex.packet_id = packt_id;
	ex.cmd = CTR_OPEN;
	ex.data_len = len;
	
	memcpy(&ex.data[0], valves, len);
// 	ex.data[0] = valve;
	
	ex.crc = calculate_crc(ex);
	
	exboard_send(ex);
}

void exboard_close(uint8_t valve)
{
	ExboardState ex;
	
	packt_id++;
	
	ex.head = HEAD;
	ex.addr = 0x01;
	ex.packet_id = packt_id;
	ex.cmd = CTR_CLOSE;
	ex.data_len = 1;
	ex.data[0] = valve;
	
	ex.crc = calculate_crc(ex);
	
	exboard_send(ex);
}

void exboard_detection(void)
{
	ExboardState ex;
	
	packt_id++;
	
	ex.head = HEAD;
	ex.addr = 0x01;
	ex.packet_id = packt_id;
	ex.cmd = CTR_DETECT;
	ex.data_len = 0;
	
	ex.crc = calculate_crc(ex);
	
	exboard_send(ex);
}

enum ReceivePart {
	HTTP_OTHER,
	HTTP_AT,
	HTTP_JSON
};


void detect_handle(ExboardState ex)
{
	exboard_detect_count = 0;
	valves = 36;
	
	if (valves != valves_temp)
	{
		valves_temp = valves;
		valves_changed();
	}
	if (ex.data_len > 0)
	{
		
	}
}

void exboard_receive(void)
{
	ExboardState ex;
	uint8_t temp;
	
	while (usart1_char_waiting())
	{	
head1:
		if (!usart1_char_waiting()) break;
		temp = usart1_getchar();
		if (0xA5 != temp) continue;
head2:
		_delay_ms(5);
		if (!usart1_char_waiting()) break;
		temp = usart1_getchar();
		if (0x5A == temp)
		{
			ex.head = HEAD;
			if (sizeof(ex.addr) != usart1_read((uint8_t *)&ex.addr, sizeof(ex.addr))) break; //16.7.8
			if (sizeof(ex.packet_id) != usart1_read((uint8_t *)&ex.packet_id, sizeof(ex.packet_id))) break;
			if (sizeof(ex.cmd) != usart1_read((uint8_t *)&ex.cmd, sizeof(ex.cmd))) break;
			if (sizeof(ex.data_len) != usart1_read((uint8_t *)&ex.data_len, sizeof(ex.data_len))) break;
			if (ex.data_len != usart1_read((uint8_t *)&ex.data, ex.data_len)) break;
			if (sizeof(ex.crc) != usart1_read((uint8_t *)&ex.crc, sizeof(ex.crc))) break; 
			if (ex.crc != calculate_crc(ex)) break;
									
			if (ex.cmd == CTR_OPEN)
			{
// 				LCD_show_string(1,0,"open");
			}
			else if (ex.cmd == CTR_CLOSE)
			{
// 				LCD_show_string(1,0,"close");
			}
			else if (ex.cmd == CTR_DETECT)
			{
				detect_handle(ex);
			}
		}
		else if (0xA5 == temp)
		{
			goto head2;
		}
		else
		{
			goto head1;
		}	
	}
}


