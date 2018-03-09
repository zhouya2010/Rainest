/*
 * mqtt_readpacket.h
 *
 * Created: 2016/6/15 9:59:41
 *  Author: wjy
 */ 
int unpack_publish(unsigned char * payload_in,int * payloadlen_in,unsigned char * buf,int buflen);
int unpack_suback(unsigned char * buf,int buflen);
int unpack_connack(unsigned char * buf,int buflen);
int unpack_puback(unsigned char *buf,int buflen);
