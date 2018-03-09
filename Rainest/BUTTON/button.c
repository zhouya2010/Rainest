/*
 * button.c
 *
 * Created: 2014/6/3 16:17:24
 *  Author: ZLF
 */ 
#include "button.h"
#include "data_rw.h"
#include "mqtt-client.h"
#include "UI.h"
#include "event_handle.h"
#include <avr/io.h>
#include <avr/wdt.h>

#ifndef F_CPU
#define F_CPU              12000000UL
#endif
#include <util/delay.h>

extern volatile unsigned char button_flag;

extern volatile unsigned char work_value;		//8工作模式
extern volatile unsigned char operate_value;	//4操作按键
extern volatile unsigned char spray_value;

extern volatile unsigned char wifi_send_over;

void button_init(void)
{
	SREG |= 0x80;
	DDRA &= ~(1 << PA1)|(1 << PA2)|(1 << PA3);
	DDRB &= ~(1 << PB2);
	DDRC &= ~(1 << PC2)|(1 << PC3)|(1 << PC4);
	
	
	DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6)|(1 << PD7);
	PORTD |= (1 << PD5) | (1 << PD6)|(1 << PD7);
	
	PCICR |= (1 << PCIE0)|(1 << PCIE1)|(1 << PCIE2)|(1 << PCIE3);
	PCMSK0 |= (1 << PCINT1)|(1 << PCINT2)|(1 << PCINT3);
	PCMSK1 |= (1 << PCINT10);
	PCMSK2 |= (1 << PCINT18)|(1 << PCINT19)|(1 << PCINT20);
	PCMSK3 |= (1 << PCINT30);
}


unsigned int button_detection(void)
{
	int res = 0;
	
	switch(button_flag)
	{
		case 1:
		{
			button_flag = 0; //16.9.27 
			
			static unsigned char previous = 0;
			previous = work_value;
			unsigned char i = 0;
			unsigned char count[8] = {0};
			while(i < 250)
			{
				switch(PINA & 0x0E)
				{
					case 0x00:count[0]++;break;
					case 0x02:count[1]++;break;
					case 0x04:count[2]++;break;
					case 0x06:count[3]++;break;
					case 0x08:count[4]++;break;
					case 0x0A:count[5]++;break;
					case 0x0C:count[6]++;break;
					case 0x0E:count[7]++;break;
					default:break;
				}
				i++;
			}
			for(i = 0;i < 8;i++)
			{
				if(count[i] > 200)
				{
					one_sec_count = 0; 
					MCDwarning_flag = 1; // 16.9.27 当检测到旋钮档变化时，打开警告
					
					if(i == 7)
					{
						if((previous == 7)|(previous == 1))
						{
							work_value = i + 1;
						}
					}
					else
					{
						work_value = i + 1;
					}
				}
			}
			res = 1;
			break;
		}
		case 2:
		{
			button_flag = 0;
			switch((PINC & 0x1C) >> 2)
			{
				case 0x00:_delay_ms(30);if((PINC & 0x1C) >> 2 == 0x00){while(0x00 == ((PINC & 0x1C) >> 2)){wdt_reset();}operate_value = 4;}break;
				case 0x01:_delay_ms(30);if((PINC & 0x1C) >> 2 == 0x01){while(0x01 == ((PINC & 0x1C) >> 2)){wdt_reset();}operate_value = 3;}break;
				case 0x02:_delay_ms(30);if((PINC & 0x1C) >> 2 == 0x02){while(0x02 == ((PINC & 0x1C) >> 2)){wdt_reset();}operate_value = 2;}break;
				case 0x03:_delay_ms(30);if((PINC & 0x1C) >> 2 == 0x03){while(0x03 == ((PINC & 0x1C) >> 2)){wdt_reset();}operate_value = 1;}break;
				case 0x04:_delay_ms(30);if((PINC & 0x1C) >> 2 == 0x04){while(0x04 == ((PINC & 0x1C) >> 2)){wdt_reset();}spray_value = 3;}break;
				case 0x05:_delay_ms(30);if((PINC & 0x1C) >> 2 == 0x05){while(0x05 == ((PINC & 0x1C) >> 2)){wdt_reset();}spray_value = 2;}break;
				case 0x06:_delay_ms(30);if((PINC & 0x1C) >> 2 == 0x06){while(0x06 == ((PINC & 0x1C) >> 2)){wdt_reset();}spray_value = 1;}break;
				
				default:break;
			}
			res = 2;
			break;
		}
		
		case 3:
		{
			button_flag = 0;
			switch(PIND & 0x44)
			{
				case 0x04:
				{
					unsigned int temp = 0;
					while((PIND & 0x40) == 0)
					{
						wdt_reset();
						temp++;
						_delay_ms(1);
						switch(temp)
						{
							case 1000:PORTD &= ~(1 << PD7);break;
							case 1200:PORTD |= (1 << PD7);set_wifi_stalink();break; //16.8.3
							default:break;
						}
						
					}
					
				}
				default: break;
			}
			res = 3;
			break;
		}
		
		case 4:
		{
			button_flag = 0;
			
			if ((PINB & (1<<PINB2)) == 0)
			{
				unsigned int temp = 0;
				while((PINB & (1<<PINB2)) == 0)
				{
					wdt_reset();
					temp++;
					_delay_ms(1);
					if(temp > 4000)
					{
						recovery_system();
					}
				}

			}
			res = 4;
			break;			
		}
		default:
		res = 0;
		break;
	}
	return res;
}


void sensor_init(void)
{
	DDRC |= (1 << PC7);
	PORTC |= (1 << PC7);
}

unsigned char get_sensor_state(void)
{
	DDRC &= ~(1 << PC7);
	if(PINC & 0x80)
	{
		return SensorOn;
	}
	else
	{
		return SensorOff;
	}
}