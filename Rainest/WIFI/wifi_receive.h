/*
 * wifi_receive.h
 *
 * Created: 2015/3/20 11:06:11
 *  Author: zy
 */ 


#ifndef WIFI_RECEIVE_H_
#define WIFI_RECEIVE_H_

#include "typedef.h"

enum AtReceiveCase {
	AT_HEAD,
	AT_DATA,
	AT_END
};

enum HttpReceiveCase {
	HTTP_HEAD,
	HTTP_DATA,
	HTTP_END
};

enum ReceivePart {
	HTTP_OTHER,
	HTTP_AT,
	HTTP_JSON
};

extern unsigned char wifi_get_data_flag;	//wifi是否收到数据标志，为1表示收到数据
extern unsigned char receive_part;

int parse_mac( void );
int check_mac(char *src);
int parse_wifi_baudrate( char * baud );

#endif /* WIFI_RECEIVE_H_ */