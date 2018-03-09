/*
 * mqtt_operation.h
 *
 * Created: 2016/6/14 13:54:32
 *  Author: wjy 
 */ 
#define NET_LED_ON()	do{PORTA &= ~(1 << PA0);}while(0)
#define NET_LED_OFF()	do{PORTA |= (1 << PA0);}while(0)

extern unsigned char mqtt_send_flag;

void mqclient_datahandler(char *data, unsigned int len);
void send_mq_app_publish(unsigned char * payload);
void send_mq_publish(unsigned char * payload);
void mqtt_read_packet(void);
void mqtt_send_data(void);
void wifimac_reload(void);
void set_wifi_restart(void);
void set_uip_restart(void);
void mqclient_timedout(void);
void mqclient_connected(void);
int mq_send_ping(void);
void set_wifi_stalink(void);


