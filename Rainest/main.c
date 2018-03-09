#include "UI.h"
#include "data_rw.h"
#include "timer.h"
#include "json.h"
#include "CMD_Ctr.h"
#include "tapdev.h"
#include "HC595N.h"
#include "uip.h"
#include "uip_arp.h"
#include "LCD.h"
#include "event_handle.h"
#include "btn_ctr.h"
#include "USART0.h"
#include "usart1.h"  //16.7.7
#include "RTC.h"
#include "button.h"
#include "data_rw.h"
#include "resolv.h"
#include "rw_buffer.h"
#include "mqtt-client.h"
#include "mqtt-data.h"
#include "exboard.h"
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#ifndef F_CPU
#define F_CPU              12000000UL
#endif
#include <util/delay.h>

	
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#define BackLightCloseTime  (300)	//N秒无操作，关背光 300*2

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */


#define BACK_LIGHT_OFF()  do{PORTD |= (1 << PD4);}while(0)	//开背光
#define BACK_LIGHT_ON()   do{PORTD &= ~(1 << PD4);}while(0)		//关背光


volatile unsigned int arp_count;			//10s定时器计数
volatile unsigned int periodic_count;		//0.5s定时器计数
volatile unsigned char arp_flag = 0;		//10s定时标志位
volatile unsigned char periodic_flag = 0;	//0.5s定时标志位
volatile unsigned int tm_count = 0;		//时间累加器(每0,5秒加1)

volatile unsigned int spray_count;			//喷淋计数
volatile unsigned char spray_flag;			//喷淋标志
volatile unsigned char sch_time_flag = 0;		//喷淋计划时间到达标志

volatile unsigned int webclient_count;		//网络访问计数器
volatile unsigned char webclient_flag;		//网络访问标志位
volatile unsigned int wifi_path_count;
volatile unsigned char wifi_path_flag;

volatile unsigned int back_light_count;		//背光计数

volatile unsigned char button_flag = 1;		//按键标识

unsigned char net_mode = WIFI_MODE;	//网络模式
volatile unsigned int visit_cloud_time = Fast;//访问接口时间

volatile unsigned char work_value = 1;		//8工作模式
volatile unsigned char operate_value;	//4操作按键
volatile unsigned char spray_value = 1;	//八个按键
volatile unsigned char sensor_value = 0;	//传感器值

unsigned char timemode_flag; //时间模式选择 16.9.22

static unsigned int net_fail_count = 0; //联网失败次数
//static unsigned char eth_state = ETH_DISCONN;  //16.7.6_2


/*---------------------------------------------------------------------------*/
// char serve[LenOfServe];
 char content[LenOfPath];

char devcode[LenOfDevcodes+1];	//设备序列号
char serialcode[LenOfSerialCode+1]; //设备随机码
char connuid[LenOfConnuid+1];//链接码
// char is_have_sensor = 0;
unsigned char mac[6] = {0};
volatile Time current_time;
unsigned int spray_time_limit = 0;

int valves = 12; //阀门路数

SLink *sch_link = NULL;
SLink *spray_link = NULL;
SLink *water_used_link = NULL;

	
//设置版本信息	
int version = 217;

void set_backlight(void)
{
	int flag = 0;
	flag = get_backlight_flag();
	if (1 == flag)
	{
		BACK_LIGHT_OFF(); 
		set_backlight_flag(0);
	}
	else
	{
		BACK_LIGHT_ON();
	}
}

void ethernet_receive(void)
{
	int i;
	if(uip_len > 0)
	{
		//eth_try = 0;
		if(BUF->type == htons(UIP_ETHTYPE_IP))		//为IP包  //16.7.7  dereferencing type-punned pointer will break strict-aliasing rules  
		{
			uip_arp_ipin();		//IP数据包处理函数
			uip_input();		//实际为uip_process，IP包核心处理函数
			if(uip_len > 0)
			{
				uip_arp_out();	//发出ARP包
				tapdev_send();	//数据送出
			}
		}
		else if(BUF->type == htons(UIP_ETHTYPE_ARP))		//为ARP包
		{
			uip_arp_arpin();	//ARP数据包处理函数
			if(uip_len > 0)
			{
				tapdev_send();	//ARP响应
			}
		}
	}
	else if(periodic_flag == 1) //0.5秒定时
	{
		periodic_flag = 0;
		for(i = 0; i < UIP_CONNS; i++)
		{
			uip_periodic(i);	//uip定期处理函数
			if(uip_len > 0)		//有数据需要发送
			{
				uip_arp_out();
				tapdev_send();
			}
		}
		#if UIP_UDP		//如果支持UDP
		for(i = 0; i < UIP_UDP_CONNS; i++)
		{
			
			uip_udp_periodic(i);
			if(uip_len > 0)
			{
				uip_arp_out();
				tapdev_send();
			}
		}
		#endif /* UIP_UDP */
		if(arp_flag == 1)	//10s定时
		{
			arp_flag = 0;
			uip_arp_timer();	//Call the ARP timer function every 10 seconds.
		}
	}
}

void move_interrupts(int mode)
{
	unsigned char temp;
	
	if (0 == mode)
	{
		/* GET MCUCR*/
		temp = MCUCR;
		/* Enable change of Interrupt Vectors */
		MCUCR = temp|(1<<IVCE);
		/* Move interrupts to Start Flash section */
		MCUCR = temp & (~(1<<IVSEL));
	}
	else if(1 == mode)
	{
		/* GET MCUCR*/
		temp = MCUCR;
		/* Enable change of Interrupt Vectors */
		MCUCR = temp|(1<<IVCE);
		/* Move interrupts to Boot Flash section */
		MCUCR = temp|(1<<IVSEL);
	}
}


int main(void) 
{
	wdt_disable();
	wdt_enable(WDTO_4S);
	wdt_reset();
	
	move_interrupts(0);
 	_delay_ms(50);
	
	DDRA |= (1 << PA0);		//LED
	PORTA |= (1 << PA0);
	DDRD |= (1 << PD4);		//背光 
	
	LCD_init();	//lcd
	get_timemode_flag(&timemode_flag); // 获取时间模式
	UI_Display();
	USART0_init();	//串口
	usart1_init();
	buffer_init();
	get_mac(mac);	
	timer_init();	//修改的初始化
	button_init();	//按键
	
	RTC_init();
	HC595N_init();	//阀门
	
	uip_setethaddr(mac);		//SetMacAddress
	tapdev_init();	//以太网初始化
	 
	uip_init();	//uip初始化
	uip_arp_init();	//缓存初始化
	
 	dhcpc_init(&mac, 6);	//DHCP
	
	resolv_init();		//DNS解析初始化 //16.9.6
	resolv_query(mainserver);	//网页服务器
	
	init_list(&sch_link);
	init_list(&spray_link);
	init_list(&water_used_link);
	read_param();
//	set_mainserve();
	TimeCtr_init();
	sensor_init();
	 while(1)
	 {		
		wdt_reset();
		
		spray_detection(spray_link);		//喷淋检测
		if (sch_time_flag)
		{
			sch_time_flag = 0;
			sch_inquire(sch_link, current_time);
		}
		if(button_detection())			//按键检测
		{
			back_light_count = 0;
			set_backlight();
 			updata_display_flag = UpdataDisplay;
		}
		sensor_value = get_sensor_state();
		
		TimeCtr(); 
		PushEvent();
 		btn_ctr();	//按键操作控制
		UI_Display();	//LCD显示
		mqtt_read_packet(); //读取网络接收缓存里的mqtt包
		mqtt_send_data();   //检测并发送mqtt
		
 		if(net_mode == ETH_MODE)
 		{
			uip_len = tapdev_read();	//读以太网包，返回包长
			ethernet_receive();
		}
		exboard_receive();
	}
	return 0;
}


ISR(TIMER1_COMPA_vect)	//定时器1A中断处理函数
{
	arp_count++;
	periodic_count++;
	
	webclient_count++;
	wifi_path_count++;
	
	if(arp_count >= Sec_10)
	{
		arp_flag = 1;
		arp_count = 0;
	}
	if(periodic_count >= Sec_half)
	{
		tm_count++;
		periodic_flag = 1;
		periodic_count = 0;
		
		RTC_time_get();
	}
	if(webclient_count >= Sec_1)  
	{
		webclient_flag = 1;
		webclient_count = 0;
		back_light_count++;
		if(back_light_count > BackLightCloseTime)
		{
			BACK_LIGHT_OFF();
		}
	}
}

ISR(TIMER1_COMPB_vect)	//定时器1B中断处理函数，用于阀门
{
	spray_count++;
	
	if(spray_count >= Sec_1)
	{
		spray_count = 0;
		spray_flag = 1;
		if (0 == current_time.sec)
		{
			sch_time_flag = 1;
		}
	}
}

ISR(PCINT0_vect)	//按键中断处理函数
{
	button_flag = 1;
}

ISR(PCINT2_vect)	//按键中断处理函数
{
	button_flag = 2;
}


ISR(PCINT3_vect)
{
	button_flag = 3;
}

ISR(PCINT1_vect)
{
	button_flag = 4;
}

/************************************************************************/
/* 函数名：resolv_found													*/
/* 功能：网页服务器文件查找												*/
/* 参数：name为网址   ipaddr为IP										*/
/* 返回值：无															*/
/************************************************************************/
void resolv_found(char *name, u16_t *ipaddr)	//网页服务器文件查找
{
    if(ipaddr != NULL) 
	{
		uip_resolv_connect(mainserver,mq_port);
	}
}

/************************************************************************/
/* 函数名：mqclient_connected											*/
/* 功能：网络连接成功													*/
/* 参数：无																*/
/* 返回值：无															*/
/************************************************************************/
void mqclient_connected(void)
{
	NET_LED_ON();
	LCD_show_strP(0, 20, PSTR("@"));
	net_connect_flag = NetConnected;
	net_fail_count = 0;
}

/************************************************************************/
/* 函数名：mqclient_timedout											*/
/* 功能：网络连接超时													*/
/* 参数：无																*/
/* 返回值：无															*/
/************************************************************************/
void mqclient_timedout( void )
{
	NET_LED_OFF();
	net_connect_flag = NetDisConnected;
	net_fail_count++;
	
	if(WIFI_MODE == net_mode)
	{
		set_wifi_restart();
		
		if(net_fail_count > 2)
		{
			net_fail_count = 0;
			net_mode = ETH_MODE;
		}
	}
	else if(ETH_MODE == net_mode)
	{
		set_uip_restart();             
		
		if(net_fail_count > 2)
		{
			net_fail_count = 0;
			net_mode = WIFI_MODE;
		}
	}
}

/************************************************************************/
/* 函数名：mqclient_datahandler										*/
/* 功能：网络数据处理													*/
/* 参数：data：接收到的数据  len：数据长度								*/
/* 返回值：无															*/
/************************************************************************/
void mqclient_datahandler( char *data, u16_t len )
{
	u16_t i = 0;
	char *data_temp = data;
	
	if(len)
	{
		while(i < len)
		{
			i++;
			data_temp++;
		}
		
		*data_temp = 0;
		
		if (NULL != data)
		{
			parse_cloud_API(data, len);
		}
	}
}


void MY_UDP_APPCALL(void)
{
	switch (uip_udp_conn->rport)
	{
		case HTONS(67):
		dhcpc_appcall();
		break;
		case HTONS(68):
		dhcpc_appcall();
		break;
		case HTONS(53):
		resolv_appcall();
		break;
		default: break;
	}
}

