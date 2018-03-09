/*
 * event_handle.c
 *
 * Created: 2014/7/31 15:06:54
 *  Author: ZY
 */ 

#include "json.h"
#include "CMD_Ctr.h"
#include "data_rw.h"
#include "btn_ctr.h"
#include "HC595N.h"
#include "event_handle.h"
#include "timer.h"
#include "RTC.h"
#include "LCD.h"
#include "UI.h"
#include "webclient.h"
#include "wifi_receive.h"
#include "wifi.h"
#include "RTC.h"
#include "USART0.h"
#include "mqtt-client.h"
#include "exboard.h"
#include <stdint.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#ifndef F_CPU
#define F_CPU  12000000UL
#endif
#include <util/delay.h>

static uchar work_value_temp;
static uchar sensor_value_temp;
static struct timer sec_timer;
static struct timer min_timer;
static struct timer half_min;
static struct timer half_sec_timer;
static struct timer reconnect_cloud_timer;
static struct timer half_hour;
static struct timer two_hour_timer ;
static struct timer wifi_timer ;
struct timer one_hour_timer ;


uchar restart_sys_count = 0;
uchar half_sec_count = 0;
uchar one_sec_count = 0;
uint valves_temp;

extern  unsigned char updata_display_flag;
extern SLink * water_used_link;
extern SLink *sch_link;

// void set_wifi_baudrate(char * baud)
// {
// 	char temp[35] = {0};
// 	sprintf_P(temp, PSTR("at+uart=%s,8,1,none,nfc\r\n"), baud);
// 	do
// 	{
// 		wdt_reset();
// 		USART0_send(temp);
// 		_delay_ms(1000);
// 		USART0_send("at+uart\r\n");
// 		_delay_ms(200);
// 	}while(!parse_wifi_baudrate(baud));
// }

void upgrade_firmware(void)
{
	LCD_clear();
	LCD_show_string(4,1,"Upgrade firmware...");
		
	_delay_ms(250);
	USART0_send("+++");
	_delay_ms(20);
	USART0_send("a\r\n");
	_delay_ms(100);
	
	set_wifi_baudrate("9600");
	
	wifi_restart();
	
	
	wdt_reset();
	eeprom_write_byte((uint8_t *) BootFlag, 0x01);
	eeprom_busy_wait();

	while(1);
}


void valves_changed(void)
{
	if ((ManuWaterPage == CurrentPage) || (SchSelValvePage == CurrentPage))
	{
		LCD_clear();
	}
	manu_sel = 0;
	plan_sel = 1;
	sent_zone_num_flag = SENT_FAILED;
	clear_link(sch_link);
//	read_schedule(); 
	read_epschedule();        //16.7.6 
}

/************************************************************************/
/* 函数名：send_water_data*/
/* 功能：发送用水量*/
/*参数：无*/
/*返回值：无*/
/************************************************************************/
void send_water_data(void)
{
	if (NetConnected == net_connect_flag)
	{
		sentwater_flag = ReadyToSend;
	}
	else
	{
		save_all_water_used(water_used_link);
		clear_link(water_used_link);
	}
}


/************************************************************************/
/* 函数名：PushEvent*/
/* 功能：消息推送*/
/*参数：无*/
/*返回值：无*/
/************************************************************************/
void PushEvent(void)
{
	if (work_value != work_value_temp)
	{
		send_work_value_flag = SentWorkMode;
		work_value_temp = work_value;
	}
	
	if ((spray_array_state != spray_array_state_temp))
	{
		spray_array_state_temp = spray_array_state;
		send_spary_status = SentSparyStatus;
		sent_zone_time_flag = NeedSentZoneTime;
		if (!spray_state)
		{
			watering_flag = WateringOFF;
			send_water_data();
		}
		else
		{
			watering_flag = WateringStrat;
		}
	}
	
	if (sensor_value != sensor_value_temp)
	{
		sent_sensor_state = NeedSentSensor;
		sensor_value_temp = sensor_value;
	}
}

/************************************************************************/
/* 函数名：fine_tune_timeout*/
/* 功能：查看微调设置是否超时*/
/*参数：无*/
/*返回值：无*/
/************************************************************************/
void fine_tune_timeout( void ) 
{	
	unsigned int i = 0;        //16.7.6_2  数组下标应为int型
	if (DateDif(current_time, weather_adjust.set_t))
	{
		for (i = 0; i < NumOfAdjustDays; i++)
		{
			weather_adjust.minadjust[i] = weather_adjust.minadjust[i+1];
			if ((NumOfAdjustDays -1) == i)
			{
				weather_adjust.minadjust[i] = 100;
			}
		}
		weather_adjust.set_t = current_time;
		save_finetune(weather_adjust);
		weather_adjust_init();
		get_adjust_flag = DisGetAdjust;
	}
}

/************************************************************************/
/* 函数名：upload_sch_detection*/
/* 功能：查看是否需要上传喷淋计划*/
/*参数：无*/
/*返回值：无*/
/************************************************************************/
void upload_sch_detection(void)
{
		sch_upload = get_upload_sch_addr(sch_link);
		
		if (NULL != sch_upload)
		{
			if (schedule_is_legal(*sch_upload))
			{
				if (Uploaded != sch_upload->upload_status)//16.7.26 原来的状态判断没有uploading，而在进入上传接口get_cmd后会将状态置为uploading
				{										  //如果上传后的业务返回解析失败，则导致当前阀门一直处于无法上传的状态,故将uploading改为uploaded
					set_sch_flag = NeedSetSch;
				}
			}
			else
			{
				del_sch(sch_link, sch_upload->zones);
			}
		}
}


void delete_sch_detection(void)
{
	delete_zone = get_del_zone();
	if (delete_zone)
	{
		set_del_zone_flag = NeedSetSch;
	}
}

void restart_system_detect() 
{
	if (restart_sys_count >= 12)
	{
		restart_sys_count = 0;
		if ((WateringOFF == watering_flag))
		{
			set_backlight_flag(1);
 			while(1);
		}
	}
}

void sent_water_detection(SLink * L)
{
	if ((DisConnCloud != net_connect_flag) && (WateringOFF == watering_flag))
	{
		eeprom_read_water_used(L);
		if (get_length(water_used_link))
		{
			sentwater_flag = ReadyToSend;
		}
	}
}
/************************************************************************/
/* 函数名：TimeCtr_int*/
/* 功能：时间事件处理初始化*/
/*参数：无*/
/*返回值：无*/
/************************************************************************/
void TimeCtr_init(void)
{
	timer_set(&half_sec_timer, CLOCK_SECOND/2);		//0.5秒定时
	timer_set(&sec_timer, CLOCK_SECOND);		//1秒定时
	timer_set(&half_min, CLOCK_SECOND * 30);	//30秒
	timer_set(&min_timer, CLOCK_MINUTE);		//1分钟定时
	timer_set(&reconnect_cloud_timer, CLOCK_MINUTE * 4);
	timer_set(&half_hour, CLOCK_MINUTE * 30);
	timer_set(&one_hour_timer, CLOCK_HOUR);
	timer_set(&two_hour_timer, CLOCK_HOUR * 2);
	timer_set(&wifi_timer, CLOCK_SECOND * 6);
}
/************************************************************************/
/* 函数名：TimeCtr*/
/* 功能：时间事件处理*/
/*参数：无*/
/*返回值：无*/
/************************************************************************/
void TimeCtr(void)
{
	if (timer_expired(&half_sec_timer))			//half second reached
	{
		timer_reset(&half_sec_timer);
		updata_display_flag = UpdataDisplay;
		half_sec_count++;		//用于"Saved Successfully"2秒定时
	}
	
	if (timer_expired(&sec_timer))			//one second reached
	{
		timer_reset(&sec_timer);
		
		one_sec_count++;
		
		delete_sch_detection();
		upload_sch_detection();
		fine_tune_timeout();
		
		if (ManuRemaningTime)
		{
			ManuRemaningTime--;
		}

		if (DisConnCloud == net_connect_flag)		//网络符号显示
		{
			LCD_show_char(0, 20, '*');
		} 
		else
		{
			LCD_show_char(0, 20, ' ');
		}
		
		exboard_detection();
		exboard_detect_count++;
		if (exboard_detect_count > 2)
		{
			valves = 12;
			if (valves != valves_temp)
			{
				valves_temp = valves;
				valves_changed();
			}
		}
	}
	
	if (timer_expired(&wifi_timer))
	{
		timer_reset(&wifi_timer);
			
		wifi_start_flag = 1;
	}
	
	if (timer_expired(&half_min))			//half min reached
	{
		timer_reset(&half_min);
	}
	
	if (timer_expired(&min_timer))			//one min reached
	{
		timer_reset(&min_timer);
		
		mq_send_ping();
		
		if (rain_delay.delay_time > 0)
		{
			rain_delay.delay_time--;
		}
		
		if (watering_flag == WateringStrat)
		{
			sent_zone_time_flag = NeedSentZoneTime;
		}
		
		restart_system_detect();
		
		sent_water_detection(water_used_link);
	}
	
	if (timer_expired(&reconnect_cloud_timer))
	{
		timer_reset(&reconnect_cloud_timer);
 		conn_cloud_flag = DisConnCloud;
	}

	if (timer_expired(&half_hour))		 //half an hour reached
	{
		timer_reset(&half_hour);
		
		
		if (rain_delay.delay_time)
		{
			set_raindelay_flag = NeedSetRainDelay;
		}
		
		if (WateringOFF == watering_flag)
		{
			get_weather_flag = NoGetWeather;
			get_adjust_flag = DisGetAdjust;
			get_time_flag =GetTimeFailed;
			visit_cloud_time = Slow;	
		}
	}
	
	if (timer_expired(&one_hour_timer))		//one hour reached
	{
		timer_reset(&one_hour_timer);
		
		restart_sys_count++;
		
		if (WateringOFF == watering_flag)
		{
			get_schedule_flag = UpdataAllSch;
		}
	}
	
	if (timer_expired(&two_hour_timer))
	{
		timer_reset(&two_hour_timer);
		if (WateringOFF == watering_flag)
		{
			visit_cloud_time = VerySlow;
		}
	}	
}
