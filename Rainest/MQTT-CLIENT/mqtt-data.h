/*
 * mqtt_client_data.h
 *
 * Created: 2016/6/15 11:27:19
 *  Author: WJY
 */ 

// #ifndef MQTT-CLIENT_DATA_H_
// #define MQTT-CLIENT_DATA_H_

#include "MQTTPacket.h"

/************************************************************************/
/*                                    MQTT-PING						*/
/************************************************************************/
#define PINGINIT        1           //心跳计数初始化
#define PINGSEND        0			//心跳发送

#define WIFI_MODE		0
#define ETH_MODE        1

#define MQ_START		0
#define WIFI_AT			1
#define	WIFI_WSMAC		2
#define WIFI_RESTART	3
#define WIFI_WMODE		4
#define SOCKET			5
#define MQ_CONNECT		6
#define MQ_SUB			7
#define MQ_READY		8
#define MQ_PUB			9
#define MQ_PING			10
#define UIP_RESTART		11
#define WIFI_LINKWAIT	12 //16.8.3

extern MQTTPacket_connectData mq_connect_data;
extern MQTTString topicString;
extern unsigned int mq_packetid;
extern const char mainserver[];
extern uint16_t mq_port;

void connect_data_init(void);
void sub_topic_init(void);
void pub_topic_init(void);
void apppub_topic_init(void);



/*#endif / * MQTT-CLIENT_DATA_H_ * /*/