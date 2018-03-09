/*
 * mqtt_readpacket.c
 *
 * Created: 2016/6/15 9:59:28
 *  Author: wjy
 */ 
#include "MQTTPacket.h"
#include "mqtt-unpack.h"
#include "mqtt-data.h"

int unpack_publish(unsigned char * payload_in,int * payloadlen_in,unsigned char * buf,int buflen)
{	
	unsigned char dup;
	int qos;
	unsigned char retained;
	unsigned short msgid;
	MQTTString receivedTopic;
	
	if(MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
	&payload_in, payloadlen_in, buf, buflen) == 1)
	{
		return 1;
	}
	return 0;
}

int unpack_suback(unsigned char * buf,int buflen)
{
	unsigned short submsgid;
	int subcount;
	int granted_qos;
	
	if(MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen) == 1)
	{
		if (granted_qos == 0)
		{
			return 1;
		}
	}
	return 0;
}

int unpack_connack(unsigned char * buf,int buflen)
{
	unsigned char sessionPresent, connack_rc;
	
	if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
	{	
	}
	else
	{
		return 1;
	}
	return 0;
}

int unpack_puback(unsigned char *buf,int buflen)
{
	unsigned char packettype = 0;
	unsigned char dup_ack = 0;
	unsigned short packetid_ack = 0;
	
	if(MQTTDeserialize_ack(&packettype,&dup_ack,&packetid_ack,buf,buflen) == 1)
	{
		if(packetid_ack == mq_packetid)
		{
			return 1;
		}
	}
	return 0;
}