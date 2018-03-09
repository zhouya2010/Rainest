/*
 * wiif_receive.c
 *
 * Created: 2015/3/20 10:53:38
 *  Author: zy
 */ 


#include "USART0.h"
#include "wifi.h"
#include "wifi_receive.h"
#include "data_rw.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "rw_buffer.h"
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <avr/pgmspace.h>

unsigned char wifi_get_data_flag = 0;



// static uint8_t data_temp[20] = {0};  //16.7.6
// static uint8_t *ptr1 = NULL;
// static uint8_t *ptr2 = NULL;
// 
// static uint8_t http_flag = 0;




void set_mac(char *src)
{
	strncpy(devcode, src, LenOfDevcodes);
	save_mac(devcode);
}

int check_mac(char *src)
{
	int i = 0;
	for(i = 0; i < LenOfDevcodes; i++)
	{
		if (!(isdigit(*src) || isupper(*src)))
		{
			return 0;
		}
		src++;
	}
	return 1;
}


int parse_mac( void )
{
	uint8_t data = 0;
	uint8_t temp[13] = {0};
	uint16_t len ;
	static uint8_t receive_state = AT_HEAD;
	int res = 0;
		
	while (buffer_in_available())
	{	
		switch(receive_state)
		{
			case AT_HEAD:
			{
				data = buffer_in_read_byte();
				if ('+' == data)
				{
					buffer_in_read(temp, 3);
					if (!strncmp_P((const char *)temp,PSTR("ok="), 3)) //16.7.7  strncmp_P(temp,PSTR("ok="), 3)
					{		
						receive_state = AT_DATA;
					}
				}
				break;
			}
			
			case AT_DATA:
			{
				len = buffer_in_read(temp, 12);
				if (12 == len)
				{
					if (check_mac((char *)temp)) //16.7.7
					{
						set_mac((char *)temp);
						receive_state = AT_END;
						res = 1;
					}
					else
					{
						receive_state = AT_HEAD;
						res = 0;
					}
					
				}
				else
				{
					receive_state = AT_HEAD;
				}
				break;
			}
			
			case AT_END:
			{
				buffer_in_read(temp, 2);
				if ((0x0d == temp[0]) && (0x0a == temp[1]))
				{
					receive_state = AT_HEAD;
					return res;
				}
				break;
			}
			
			default:
				{
					receive_state = AT_HEAD;
					break;
				}
		}
	}	
	return res;
}


int parse_wifi_baudrate( char * baud )
{
	uint8_t data = 0;
	uint8_t temp[13] = {0};
	uint16_t len ;
	static uint8_t receive_state = AT_HEAD;
	int res = 0;
	int read_len = strlen(baud);
	
	while (buffer_in_available())
	{
		switch(receive_state)
		{
			case AT_HEAD:
			{
				data = buffer_in_read_byte();
				if ('+' == data)
				{
					buffer_in_read(temp,3);
					if (!strncmp_P((char*)temp,PSTR("ok="), 3))
					{
						receive_state = AT_DATA;
					}
				}
				break;
			}
			
			case AT_DATA:
			{
				len = buffer_in_read(temp, read_len);
				if (read_len == len)
				{
					if (!strncmp((const char *)temp, baud, read_len)) //16.7.7strncmp(temp, baud, read_len)
					{
						return 1;
					}
					else
					{
						receive_state = AT_HEAD;
						res = 0;
					}
					
				}
				else
				{
					receive_state = AT_HEAD;
				}
				break;
			}
						
			default:
			{
				receive_state = AT_HEAD;
				break;
			}
		}
	}
	return res;
}



