/*
 * mqtt_client.c
 *
 * Created: 2016/4/28 15:34:43
 *  Author: Administrator
 */ 
#include <string.h>  //16.7.7
				     
#include "uip.h"
#include "uiplib.h"
#include "rw_buffer.h"
#include "uip-client.h"
#include "USART0.h"

#define CLIENT_DISCONNECT		0
#define CLIENT_CONNECTED		1
#define CLIENT_TIMEOUT			3



#define MQTT_TIMEOUT 1
static struct mqtt_state s = {0};

unsigned char uip_state = CLIENT_DISCONNECT;
static unsigned char buff_temp[300] = {0};
static unsigned int len_temp = 0;
static unsigned char uip_ack_flag = 1;
static unsigned char uip_send_flag = 0;
static unsigned char uip_socket_flag = CLIENT_DISCONNECT;


int uip_resolv_connect(char * host,uint16_t port)
{
	struct uip_conn *conn;
	uip_ipaddr_t *ipaddr;
	static uip_ipaddr_t addr;
	
	/* First check if the host is an IP address. */
	ipaddr = &addr;
	if(uiplib_ipaddrconv(host, (unsigned char *)addr) == 0)
	{
		ipaddr = (uip_ipaddr_t *)resolv_lookup(host);
		
		if(ipaddr == NULL)
		{
			return 0;
		}
	}
	
	conn = uip_connect(ipaddr, htons(port));
	if(conn == NULL)
	{
		return 0;
	}
	
	return 1;
}

int uip_socket_connect(char * host)
{	
	resolv_query(host);
	
	s.timer++;
	if(s.timer >= 20)
	{
		s.timer = 0;
		uip_socket_flag = CLIENT_TIMEOUT;
	}
	
	if(uip_socket_flag == CLIENT_CONNECTED) return 1;
	if(uip_socket_flag == CLIENT_DISCONNECT) return 0;
	if(uip_socket_flag == CLIENT_TIMEOUT)  return -1;
	
	return 0;
}

void uip_send_data(unsigned char * buf,int len)
{
	if(uip_ack_flag)
	{
		uip_send_flag = 1;
		uip_ack_flag = 0;
		len_temp = len;
		memcpy(buff_temp,buf,len_temp);
	}
}

void uip_client_restart(void)
{
	uip_abort();
	s.timer = 0;
	uip_ack_flag = 1;
	uip_socket_flag = CLIENT_DISCONNECT;
}


void mqtt_uip_newdata(uint8_t *data, uint16_t len)
{
	buffer_in_write((uint8_t *)uip_appdata, uip_len);
}


void mqtt_appcall(void)
{	
	if(uip_connected())
	{
		s.timer = 0;
		uip_socket_flag = CLIENT_CONNECTED;
		return;
	}
	
	if(uip_aborted())
	{
		uip_abort();
		uip_socket_flag = CLIENT_DISCONNECT;
		return;
	}
	
	if(uip_timedout())
	{	
		uip_abort();
		uip_socket_flag = CLIENT_TIMEOUT;
		return;
	}
	
	
	if(uip_acked())
	{
		uip_ack_flag = 1;
	}
	
	if(uip_newdata())
	{
		mqtt_uip_newdata((uint8_t *)uip_appdata, uip_len);
	}
	
	if(uip_rexmit())
	{
		uip_send(buff_temp, len_temp);
		return;
	}
	else if(uip_poll())
	{
		if(uip_socket_flag != CLIENT_CONNECTED)
		{
			uip_abort();
		}
		
		if(uip_send_flag)
		{
			uip_send_flag = 0;
			uip_send(buff_temp,len_temp);
		}
	
 		return;
	}
	
	if(uip_closed())
	{	
		uip_abort();
		uip_socket_flag = 0;

		return;
	}
}