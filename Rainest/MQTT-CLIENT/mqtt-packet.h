/*
 * mqtt_send.h
 *
 * Created: 2016/6/15 9:55:24
 *  Author: wjy
 */ 



extern unsigned int mq_packetid;

int connect_packet(unsigned char * buff,int buflen);
int subscribe_packet(unsigned char * buff,int buflen);
int ping_packet(unsigned char *buff);
int publish_packet(unsigned char * buff,unsigned char *payload);
int app_publish_packet(unsigned char * buff,unsigned char * payload);




