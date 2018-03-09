/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/

#include <stdio.h>                  //16.7.7
#include "mqtt-data.h"
#include "transport.h"
#include "rw_buffer.h"
#include "wifi.h"
#include "uip-client.h"
#include "USART0.h"    //16.7.7



#ifndef F_CPU
#define F_CPU   12000000UL
#endif
#include <util/delay.h>


/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTSNPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/


//int Socket_error(char* aString, int sock)
//{
//
//}


void wifi_transport_sendPacketBuffer(unsigned char* buf,int len)
{	
	USART0_send_n((char *)buf,len);  //16.7.8
}

void eth_transport_sendPacketBuffer(unsigned char* buf,int len)
{
	uip_send_data(buf,len);
}

int transport_getdata(unsigned char* buf, int count)
{
	int res = 0;

	while(count != 0)
	{
		if (!buffer_in_available())
		{
			_delay_ms(1);
			if (!buffer_in_available())
			{
				break;
			}
		}
	
		*buf = buffer_in_read_byte();
		buf++;
		count--;
		res++;
	}
	return res;
}


int wifi_transport_open(void)
{
	send_wifi_socket(mainserver,mq_port);	
	return 1;
}

int eth_transport_open(void)
{
	return uip_socket_connect(mainserver);
}

void wifi_transport_close() //16.7.6_2  "control reaches end of non-void function"
{
	unsigned char temp[50] = {0};
	sprintf((char *)temp,"at+tcpdis=off\r\n");     //16.7.7
	_delay_ms(1000);
	usart_buffer_reset();
}

