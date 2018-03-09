#include "USART0.h"
#include "wifi.h"
#include "wifi_receive.h"
#include "data_rw.h"
#include <string.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/wdt.h>

#ifndef F_CPU
#define F_CPU              12000000UL
#endif
#include <util/delay.h>

unsigned char wifi_start_flag = 0;

void sent_at_cmd(void)
{
	char temp[10] = {0};
	strcpy_P(temp, PSTR("+++"));
	USART0_send(temp);
	_delay_ms(50);
	strcpy_P(temp, PSTR("a\r\n"));
	USART0_send(temp);
	_delay_ms(100);
}


void wifi_init(void)
{
	wifi_restart();
}

void sent_wifi_at( void ) 
{
	char temp[20] = {0};
		
	wdt_reset();
	strcpy_P(temp, PSTR("+++"));
	USART0_send(temp);
	_delay_ms(200);
	strcpy_P(temp, PSTR("a"));
	USART0_send(temp);
	_delay_ms(100);
}

int sent_wifi_wsmac(void) 
{
	char temp[20] = {0};
	unsigned char flag = 0;
	
	flag = get_mac_flag();
	if (1 != flag)
	{
		sprintf_P(temp, PSTR("at+wsmac\r\n\r\n"));
		USART0_send(temp);
		wdt_reset();
		_delay_ms(1000);
		if(parse_mac())
		{
			return 1;
		}
	}
	else
	{
		wdt_reset();
		return 1;
	}
	
	return 0;
}


void wifi_wmode(void)
{
	char temp[40] = {0};
	char user_name[11];
	char password[13];
	
	sprintf_P(temp, PSTR("at+wmode=sta\r\n\r\n"));
	USART0_send(temp);
	wdt_reset();
	_delay_ms(1500);
		
	sprintf_P(temp, PSTR("at+htpen=off\r\n\r\n"));
	USART0_send(temp);
	wdt_reset();
	_delay_ms(500);	
		
	sprintf_P(user_name, PSTR("username"));
	sprintf_P(password, PSTR("password"));
	sprintf_P(temp, PSTR("at+webu=%s,%s\r\n\r\n"), user_name, password);
		
	USART0_send(temp);
	wdt_reset();
	_delay_ms(2000);	
}




void wifi_restart(void)
{
	wdt_reset();
	_delay_ms(10);
	WIFI_RST_ALT();
	_delay_ms(100);
	WIFI_RST_ALT();
}


void send_wifi_socket(char * host,uint16_t port)
{
	char temp[50] = {0};  //16.7.8
	
	sprintf(temp,"at+netp=TCP,client,%d,%s\r\n",port,host);
	USART0_send(temp);
	_delay_ms(500);
	
	sprintf(temp,"at+tcpdis=on\r\n");
	USART0_send(temp);
	_delay_ms(500);
}

void set_wifi_baudrate(char * baud)
{
	char temp[35] = {0};
	char temp2[15] = {0};
	sprintf_P(temp, PSTR("at+uart=%s,8,1,none,nfc\r\n"), baud);
	strcpy_P(temp2, PSTR("at+uart\r\n"));
	do
	{
		wdt_reset();
		USART0_send(temp);
		_delay_ms(1000);
		USART0_send(temp2);
		_delay_ms(400);
	}while(!parse_wifi_baudrate(baud));
}

int get_wifi_baudrate(char * baud)
{
	char temp[20];
	for (int i=0; i<3; i++)
	{
		wdt_reset();
		strcpy_P(temp,PSTR("at+uart\r\n"));
		USART0_send(temp);
		_delay_ms(200);
		if(parse_wifi_baudrate(baud))
		{
			return 1;
		}
	}
	
	return 0;
}

int check_baud(void)
{
	char baud[10] = {0};
	
	set_usart_baud(BAUD_38400);
	strcpy_P(baud,PSTR("38400"));
	sent_at_cmd();
	if(get_wifi_baudrate(baud))
	{
		return 1;
	}
	
	set_usart_baud(BAUD_19200);
	strcpy_P(baud,PSTR("19200"));
	sent_at_cmd();
	if(get_wifi_baudrate(baud))
	{
		strcpy_P(baud,PSTR("38400"));
		set_wifi_baudrate(baud);
		wifi_restart();
		wifi_start_flag = 0;
		set_usart_baud(BAUD_38400);
		return 1;
	}
	
	set_usart_baud(BAUD_9600);
	strcpy_P(baud,PSTR("9600"));
	sent_at_cmd();
	if (get_wifi_baudrate(baud))
	{
		strcpy_P(baud,PSTR("38400"));
		set_wifi_baudrate(baud);
		wifi_restart();
		wifi_start_flag = 0;
		set_usart_baud(BAUD_38400);
		return 1;
	}
	
	wifi_restart();
	wifi_start_flag = 0;
	return 0;
}
