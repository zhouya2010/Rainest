/*
 * mqtt_data.c
 *
 * Created: 2016/6/15 11:27:50
 *  Author: wjy
 */ 
#include "mqtt-data.h"
#include "MQTTPacket.h"
#include "typedef.h"
#include <stdio.h>

const char mainserver[] = "www.rainmq.com";
uint16_t mq_port = 1883;

MQTTPacket_connectData mq_connect_data = MQTTPacket_connectData_initializer;
MQTTString topicString = MQTTString_initializer;

char topic[25] = {0};

unsigned int mq_packetid = 0;


void connect_data_init(void)
{
	mq_connect_data.clientID.cstring = devcode;
	mq_connect_data.keepAliveInterval = 200;
	mq_connect_data.cleansession = 1;
	mq_connect_data.username.cstring = "NxEco@2016";
	mq_connect_data.password.cstring = "#@iHyX4y+GPQ";
}

void sub_topic_init(void)
{
	sprintf(topic,"device_%s",devcode);
	topicString.cstring = topic;
}

void pub_topic_init(void)
{
	sprintf(topic,"device_pubtopic");
	topicString.cstring = topic;
}

void apppub_topic_init(void)
{	
	sprintf(topic,"app_%s",devcode);
	topicString.cstring = topic;
}