/*
 * wifi.h
 *
 * Created: 2014/6/13 16:01:01
 *  Author: ZLF
 */ 


#ifndef WIFI_H_
#define WIFI_H_

extern unsigned char  wifi_start_flag;

#define WIFI_RST_ALT()  PORTD ^= (1 << PD5)

void wifi_init(void); //wifi模块初始化
void wifi_restart(void); //wifi模块重启
void sent_wifi_at(void); //开启wifi at指令
int sent_wifi_wsmac(void); //发送wifi code获取指令
void wifi_wmode(void); // 设定wifi网络参数
void send_wifi_socket(char * host,uint16_t port); //发送wifi socket指令
int get_wifi_baudrate(char * baud); //获取wifi波特率
void set_wifi_baudrate(char * baud); //设置wifi波特率
int check_baud(void);   //wifi波特率检测


#endif /* WIFI_H_ */