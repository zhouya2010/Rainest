/*
 * mqtt_packet.c
 *
 * Created: 2016/6/15 9:56:05
 *  Author: wjy
 */
#include <string.h>    //16.7.7 缺少头文件
#include "mqtt-packet.h"
#include "mqtt-data.h"



int connect_packet(unsigned char * buff,int buflen)
{
	int len = 0;
	connect_data_init();
		
	len = MQTTSerialize_connect(buff,buflen,&mq_connect_data);
	return len;
}

int subscribe_packet(unsigned char * buff,int buflen)
{
	unsigned short msgid = 1;
	int req_qos = 0;
	int len = 0;
	sub_topic_init();
	
	len = MQTTSerialize_subscribe(buff, buflen, 0, msgid, 1, &topicString, &req_qos);
	return len;
}

int ping_packet(unsigned char *buff)
{
	unsigned char buf[50] = {0};
	int buflen = sizeof(buf);
	int len = 0;
	
	len = MQTTSerialize_pingreq(buf,buflen);
	memcpy(buff,buf,len);
	return len;
}

int publish_packet(unsigned char * buff,unsigned char *payload)
{	
	unsigned char buf[400] = {0};
	int buflen = sizeof(buf);
	int len = 0;
	int payloadlen = strlen((const char *)payload); //16.7.7 strlen(payload)
	pub_topic_init();
	
	len = MQTTSerialize_publish(buf, buflen, 0, 1, 0, mq_packetid, topicString, (unsigned char*)payload, payloadlen);
	memcpy(buff,buf,len);
	return len;
}

int app_publish_packet(unsigned char * buff,unsigned char * payload)
{	
	unsigned char buf[400] = {0};
	int buflen = sizeof(buf);
	int len = 0;
	int payloadlen = strlen((const char *)payload); //16.7.7 strlen(payload)
	apppub_topic_init();
	
	len= MQTTSerialize_publish(buf, buflen, 0, 1, 0, mq_packetid, topicString, (unsigned char*)payload, payloadlen);
	memcpy(buff,buf,len);
	return len;
}