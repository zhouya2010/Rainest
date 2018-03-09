/*
 * data_rw.c
 *
 * Created: 2014/7/9 20:26:38
 *  Author: ZY
 */ 
#include "data_rw.h"
#include "btn_ctr.h"
#include "RTC.h"
#include "schedule_execute.h"
#include "wifi_receive.h"
#include "LCD.h"
#include <avr/wdt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

DevParam devparam;//设备参数
RainDelay rain_delay;
extern uchar user_band_flag;
extern uchar sentwater_flag;
extern SLink * water_used_link;
extern SLink *spray_link;

typedef struct epschedule_struct   //16.7.6 新设结构体，用于eeprom喷淋计划链表节点格式
{
	Schedule sch;                //喷淋计划数据
	unsigned char nextzone;     //指向下一路数
	} EpSchedule,*pEpSchedule;

void set_mac_flag(unsigned char flag)
{
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)DevcodeFlagAddr, flag);
}

void read_spray_time_limit(void)
{
	eeprom_busy_wait();
	spray_time_limit = eeprom_read_word((uint16_t *)SprayLimitTimeAddr);
	eeprom_busy_wait();
	if (MaxManuTime < spray_time_limit)
	{
		spray_time_limit = MaxManuTime;
	}
}

void save_spray_time_limit(uint16_t howlong)
{
	eeprom_busy_wait();
	eeprom_write_word((uint16_t *)SprayLimitTimeAddr, howlong);
	eeprom_busy_wait();
}
 /************************************************************************/
 /* 函数名：get_devcode													 */
 /* 功能：		获取设备号												 */
 /*参数：	无															 */
 /*返回值：设备号														 */
 /************************************************************************/
 void get_devcode(char * code)
 {
	eeprom_busy_wait();
	eeprom_read_block(code, (uint8_t *)DevcodeAddr, LenOfDevcodes);
	eeprom_busy_wait();
 }

 /************************************************************************/
 /* 函数名：get_serialcode*/																		
 /* 功能：		获取随机码*/															
 /*参数：	无		*/																						
 /*返回值：设备号	*/																			
 /************************************************************************/
void get_serialcode( char * code ) 
{
	int i,j;
	char s[12];
	char p[37];
	unsigned long ultime = 0;	//云端时间

	strcpy_P(p, PSTR("1234567890ABCDEFGHIJKLMNPQRSTUVWXYZ"));
	RTC_time_get();
	sprintf_P(s,PSTR("%d%d%d%d%d"),\
								current_time.mon,\
								current_time.mday,\
								current_time.hour,\
								current_time.min,\
								current_time.sec);
	sscanf_P(s,PSTR("%ld"),&ultime);
	
	srandom(ultime);
	
	for(i = 0; i < LenOfSerialCode; i++)
	{
		j = random()%35;
		code[i] = p[j];
	}
	eeprom_busy_wait();
	eeprom_write_block(serialcode, (uint8_t *)SerialCodeAddr, LenOfSerialCode);
}


/************************************************************************/
/* 函数名：read_code													*/
/* 功能：		读取设备号和随机码										*/
/*参数：	无															*/
/*返回值：无															*/
/************************************************************************/
void read_code( void ) 
{
	get_devcode(devcode);
	get_serialcode(serialcode);
	if (!check_mac(devcode))
	{
		set_mac_flag(0);
	}
}

/************************************************************************/
/* 函数名：read_schedule												*/
/* 功能：		获取喷淋计划											*/
/*参数：	无															*/
/*返回值：无															*/
/************************************************************************/
// void read_schedule( void )    //16.7.6 不再使用，影响编译故屏蔽
// {
// 	uchar i = 0;
// 	uchar num = 0;
// 	uint addr = 0;
// 	Schedule sch;
// 	
// 	addr = ScheduleNumAddr;
// 	eeprom_busy_wait();
// 	num = eeprom_read_byte((uint8_t *)addr);
// 	eeprom_busy_wait();
// 
// 	if( num > MaxScheduleNum)
// 	{
// 		num = 0;
// 	}
// 	
// 	if (NULL != sch_link)
// 	{
// 		for(i = 0; i < num; i++)
// 		{
// 			addr = Schedule1Addr + i * sizeof(Schedule);
// 			eeprom_read_block(&sch, (pSchedule)addr, sizeof(Schedule));
// 			eeprom_busy_wait();
// 			if (sch.zones > valves)
// 			{
// 				continue;
// 			}
// 			else
// 			{
// 				insert_sch(sch_link, sch);
// 			}
// 		}
// 	}
// }

void devparam_init(void)
{
// 	devparam.hassensor = 0;
// 	devparam.sensorinterv = 6;
// 	devparam.minadjust = 100;
	devparam.mwspray = 1;
// 	devparam.zonecount = 8;
	devparam.timezone = 0;
// 	devparam.maxelectric = 1000;
	devparam.mastertag = 0;
	devparam.tem_unit_flag = 0;
// 	strcpy_P(devparam.fwver,PSTR("1.6"));
}
/************************************************************************/
/* 函数名：read_devparam												*/
/* 功能：		读取设备参数										    */
/*参数：	无															*/
/*返回值：无															*/
/************************************************************************/
void read_devparam( void ) 
{
	eeprom_busy_wait();
	eeprom_read_block(&devparam, (pDevParam)DevParamAddr, sizeof(DevParam));
	eeprom_busy_wait();
	if (0xff == devparam.mwspray)
	{
		devparam_init();
		save_devparam();
	}
}

/************************************************************************/
/* 函数名：save_devparam												*/
/* 功能：		读取设备参数											*/
/*参数：	无															*/
/*返回值：无															*/
/************************************************************************/
void save_devparam( void )
{
	eeprom_busy_wait();
	eeprom_write_block(&devparam, (pDevParam)DevParamAddr, sizeof(DevParam));
}

/************************************************************************/
/* 函数名：get_water_used												*/
/* 功能：获取用水量														*/
/*参数：WaterUsedStruct 用水时间和路数									*/
/*返回值：无															*/
/************************************************************************/
pWaterUsedStruct get_water_used(SLink *L)
{
	int i = 0;
	SLink *p = NULL;
	pWaterUsedStruct pwus = NULL;
	
	i = get_length(L);
	
	if (i)
	{
		p = get_addr(L,i);
		if (NULL != p)
		{
			pwus = (pWaterUsedStruct)(p->data);
		}
	}
	
	return pwus;
}


/************************************************************************/
/* 函数名：read_water_used			*/													
/* 功能：		读取用水量		*/																			
/*参数：	无					*/																			
/*返回值：无				*/																			
/************************************************************************/
void save_raindelay( void ) 
{
	eeprom_busy_wait();
	eeprom_write_block(&rain_delay, (pRainDelay)RainDelayAddr, sizeof(RainDelay));
}

void read_raindelay( void ) 
{
	int min_dif = 0;

	eeprom_busy_wait();
	eeprom_read_block(&rain_delay, (pRainDelay)RainDelayAddr, sizeof(RainDelay));
	eeprom_busy_wait();
	if ( rain_delay.delay_time > MaxRainDelayTime )
	{
		rain_delay.delay_time = 0;
		return;
	}
	
	min_dif = MinDif(current_time, rain_delay.begin_time);
	if( min_dif < 0 )
	{
		rain_delay.delay_time = 0;
	}
	else
	{
		rain_delay.delay_time = rain_delay.delay_time - min_dif;
		if (rain_delay.delay_time < 0)
		{
			rain_delay.delay_time = 0;
		}
	}
}

void read_manuwater_set( void ) 
{
	eeprom_busy_wait();
	eeprom_read_block(ManuValveTime, (uint16_t *)ManuWaterSetAddr, MaxValveManu*2);
	if (ManuValveTime[0] > MaxManuTime)
	{
		memset(ManuValveTime, 0,sizeof(int)*MaxValveManu);
		save_manuwater_set();
	}
}


// void save_schedule( SLink *L ) //16.7.6 不再使用
// {
// 	uint addr = 0;
// 	SLink *p = L->next;
// 	Schedule sch;
// 	uchar i = 0;
// 	uchar j = 0;
// 	
// 	i = get_length(L);
// 	addr = ScheduleNumAddr;
// 	eeprom_busy_wait();
// 	eeprom_write_byte((uint8_t *)addr,i);
// 	
// 	while(NULL != p)
// 	{
// 		sch = *(pSchedule)(p->data);
// 		p = p->next;
// 		addr = Schedule1Addr + j * sizeof(Schedule);
// 		eeprom_busy_wait();
// 		eeprom_write_block(&sch, (pSchedule)addr, sizeof(Schedule));
// 		j++;
// 	}
// }

/************************************************************************/
/* 函数名：add_epschedule												*/
/* 功能：将新喷淋计划保存在eeprom										*/
/*参数：L： 喷淋计划链表地址											*/
/*返回值：1成功  0：失败												*/
/************************************************************************/
int add_epschedule(SLink *L,Schedule sch_new)           //16.7.6        
{
	uint addr = 0;
	unsigned int i = 0;
	SLink *p;
	EpSchedule epsch;
	memset(&epsch,0,sizeof(EpSchedule));
	
	i = get_length(L);                   //RAM中链表长度为0,则新的数据路数作为首节点
	if(i == 0)
	{
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t *)EpScheduleHeadAddr,sch_new.zones);
		eeprom_busy_wait();
	}else                               //如果链表长度不为0，则需更改尾节点的nextzone指向新的节点
	{
		p = get_addr(L,i);
		epsch.sch = *(pSchedule)(p->data);
		epsch.nextzone = sch_new.zones;
		addr = EpSchedule1Addr + sizeof(EpSchedule) * (epsch.sch.zones - 1);
		eeprom_busy_wait();
		eeprom_write_block(&epsch,(pEpSchedule)addr,sizeof(EpSchedule));
		eeprom_busy_wait();
	}
	
	epsch.sch = sch_new;
	epsch.nextzone = 0;
	
	addr = EpSchedule1Addr + sizeof(EpSchedule) * (epsch.sch.zones - 1);
	eeprom_busy_wait();
	eeprom_write_block(&epsch,(pEpSchedule)addr,sizeof(EpSchedule));
	eeprom_busy_wait();

	return 1;
}

/************************************************************************/
/* 函数名：updata_epschedule											*/
/* 功能：更新保存在eeprom中指定路数的喷淋计划数据						*/
/*参数：L： 喷淋计划链表地址	zone： 指定路数							*/
/*返回值：1成功  0：失败												*/
/************************************************************************/
int updata_epschedule(SLink *L,Schedule sch_new)    //16.7.6  
{
	uint addr = 0;
	unsigned int j = 0;
	SLink *p;
	EpSchedule epsch_new;
	Schedule sch;
	memset(&epsch_new,0,sizeof(EpSchedule));
	memset(&sch,0,sizeof(Schedule));
												
	j = locate_zone(L,sch_new.zones);
	if(j == 0) return 0;
	
	epsch_new.sch = sch_new;
	
	j = j+1;
	p = get_addr(L,j);
	if(p != NULL)     //如果需更新的节点不是尾节点，则还需填写nextzone指向路数             
	{
		sch = *(pSchedule)(p->data);
		epsch_new.nextzone = sch.zones;
	}	
	
	addr = EpSchedule1Addr + sizeof(EpSchedule)*(epsch_new.sch.zones - 1);
	eeprom_busy_wait();
	eeprom_write_block(&epsch_new,(pEpSchedule)addr,sizeof(EpSchedule));
	eeprom_busy_wait();

	return 1;                           
}

/************************************************************************/
/* 函数名：insert_epschedule											*/
/* 功能：将新喷淋计划保存在eeprom										*/
/*参数：L： 喷淋计划链表地址											*/
/*返回值：1成功  0：失败												*/
/************************************************************************/
void insert_epschedule(SLink *L, Schedule sch_new)  //16.7.6 
{
	int i = 0;

	i = locate_zone(L, sch_new.zones);
	if (i)
	{
		updata_epschedule(L,sch_new);  //16.7.6
	}
	else
	{
		add_epschedule(L,sch_new);
	}
}


/************************************************************************/
/* 函数名：delete_epschedule											*/
/* 功能：删除eeprom中指定路数的喷淋计划数据								*/
/*参数：L： 喷淋计划链表地址	zone： 指定路数							*/
/*返回值：1成功  0：失败												*/
/************************************************************************/
int delete_epschedule(SLink * L,unsigned char zone)    //16.7.6  
{
	uint addr = 0;
	SLink *p;
	unsigned int i;
	unsigned int j;
	EpSchedule epsch;
	Schedule sch = {0};
	
	i = locate_zone(L,zone);
	if(i == 0) return 0;
	
	j = i+1;              //取删除节点的下一节点
	i = i-1;			  //取删除节点的上一节点
	
	if(i == 0)           //如果上一节点为0 ，说明删除的是头节点
	{
		p = get_addr(L,j);
		if(p == NULL)          //下一节点为空，说明删除了最后一个节点，头路数应保存为0
		{
			eeprom_busy_wait();
			eeprom_write_byte((uint8_t *)EpScheduleHeadAddr,0);
			eeprom_busy_wait();
			return 1;
		}
		sch = *(pSchedule)(p->data);  //不为空，则头路数保存为下一节点路数
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t *)EpScheduleHeadAddr,sch.zones);
		eeprom_busy_wait();
		return 1;		
	}
	
	p = get_addr(L,i);
	epsch.sch = *(pSchedule)(p->data);
	
	p = get_addr(L,j);        
	if(p == NULL)				//如果要下一节点为空，则删除的是尾节点，上一节点指向0
	{		
		epsch.nextzone = 0;
		addr = EpSchedule1Addr + sizeof(EpSchedule) * (epsch.sch.zones - 1);
		eeprom_busy_wait();
		eeprom_write_block(&epsch,(pEpSchedule)addr,sizeof(EpSchedule));
		eeprom_busy_wait();
		return 1;
	}else                          //如果不为空，则上一节点的next指向下一节点的路数
	{
		sch = *(pSchedule)(p->data);
		epsch.nextzone = sch.zones;
		addr = EpSchedule1Addr + sizeof(EpSchedule) * (epsch.sch.zones - 1);
		eeprom_busy_wait();
		eeprom_write_block(&epsch,(pEpSchedule)addr,sizeof(EpSchedule));
		eeprom_busy_wait();
		return 1;
	}
}

/************************************************************************/
/* 函数名：updata_epschedule											*/
/* 功能：读取eeprom中的喷淋计划数据，并插入到RAM中的sch_link			*/
/*参数：L： 空															*/
/*返回值：空															*/
/************************************************************************/
void read_epschedule(void)               //16.7.6  
{
	uint addr = 0;
	unsigned char head = 0;
	Schedule sch = {0};
	EpSchedule epsch;
			
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)EpScheduleHeadAddr);
	eeprom_busy_wait();
	if(head > MaxValve)
	{
		head = 0;
	}
	if(head == 0) return;
	
	
	addr = EpSchedule1Addr + sizeof(EpSchedule) * (head - 1);
	eeprom_busy_wait();
	eeprom_read_block(&epsch,(pEpSchedule)addr,sizeof(EpSchedule));
	eeprom_busy_wait();
	while(1)
	{
		sch = epsch.sch;	
		insert_sch(sch_link, sch);	
		
		if(epsch.nextzone == 0) break;
		addr = EpSchedule1Addr + sizeof(EpSchedule) * (epsch.nextzone - 1);
		eeprom_busy_wait();
		eeprom_read_block(&epsch,(pEpSchedule)addr,sizeof(EpSchedule));	
		eeprom_busy_wait();
	}
}


/************************************************************************/
/* 函数名：reset_epschedule												*/
/* 功能：删除eeprom中所有喷淋计划数据									*/
/*参数：L： 喷淋计划链表地址	zone： 指定路数							*/
/*返回值：1成功  0：失败												*/
/************************************************************************/
void reset_epschedule(void)             //16.7.6  
{
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)EpScheduleHeadAddr,0);
	eeprom_busy_wait();
}

// void reset_sch_num(void)          //16.7.6  不再使用
// {
// 	eeprom_busy_wait();
// 	eeprom_write_byte((uint8_t *)ScheduleNumAddr,0);
// 	eeprom_busy_wait();
// }

void save_band_status( void )
{
	uchar i = 0xaa;
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)BandStatusAddr,i);
}

void reset_band_status( void )
{
	uchar i = 0xff;
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)BandStatusAddr,i);
}


void read_band_status(void)
{
	uchar i = 0;
	eeprom_busy_wait();
	i = eeprom_read_byte((uint8_t *)BandStatusAddr);
	eeprom_busy_wait();
	if (0xaa == i)
	{
		user_band_flag = UserBandOk;
	}
}


void save_manuwater_set(void)
{
	eeprom_busy_wait();
	eeprom_write_block(ManuValveTime, (uint16_t *)ManuWaterSetAddr, MaxValveManu*2);
}

void save_finetune( FineTune ft )
{
	eeprom_busy_wait();
	eeprom_write_block(&ft,(pFineTune)FineTuneAddr, sizeof(FineTune));
	eeprom_busy_wait();
}

void finetune_init(void)
{
	memset(weather_adjust.minadjust, 100, sizeof(weather_adjust.minadjust));
	weather_adjust.adjust_mode = 0;
	save_finetune(weather_adjust);
}

void read_finetune( void )
{
	eeprom_busy_wait();
	eeprom_read_block(&weather_adjust,(pFineTune)FineTuneAddr, sizeof(FineTune));
	eeprom_busy_wait();
	if (weather_adjust.minadjust[0] > MaxtAdjustValue)
	{
		finetune_init();
	}
}

// void update_schedule( uchar i,Schedule NewSch )          //16.7.6  不再使用
// {
// 	uint addr = 0;	
// 	
// 	addr = Schedule1Addr + (i-1) * sizeof(Schedule);
// 	eeprom_busy_wait();
// 	eeprom_write_block(&NewSch, (pSchedule)addr, sizeof(Schedule));
// }

void set_del_zone(uchar zone)
{
	int i = 1;
	SLink *p = NULL;
	pSchedule psch = NULL;
	uchar del_valves[6] = {0};
	
	if (zone > MaxValve)
	{
		return;
	}
	
	i = locate_zone(sch_link,zone);
	p = get_addr(sch_link,i);
	if (NULL != p)
	{
		psch = (pSchedule)p->data;
		if(Uploaded == psch->upload_status)				
		{
			eeprom_busy_wait();
			eeprom_read_block(del_valves,(uint8_t *)DeleteSchAddr, sizeof(del_valves));
			eeprom_busy_wait();
			del_valves[(zone-1)/8] &= ~(1<<((zone-1)%8));
			eeprom_write_block(del_valves,(uint8_t *)DeleteSchAddr,sizeof(del_valves));
			eeprom_busy_wait();
		}
	}
}

void reset_del_zone(uchar zone)
{
	uchar del_valves[6] = {0};
	
	if(zone > MaxValve)
	{
		return;
	}
	eeprom_busy_wait();
	eeprom_read_block(del_valves,(uint8_t *)DeleteSchAddr, sizeof(del_valves));
	eeprom_busy_wait();
	del_valves[(zone-1)/8] |= 1<<((zone-1)%8);
	eeprom_write_block(del_valves,(uint8_t *)DeleteSchAddr,sizeof(del_valves));
	eeprom_busy_wait();
}

uchar get_del_zone(void)
{
	int i = 0;
	int j = 0;
	uchar k = 0;
	uchar del_valves[6] = {0};
	eeprom_busy_wait();
	eeprom_read_block(del_valves,(uint8_t *)DeleteSchAddr, sizeof(del_valves));
	eeprom_busy_wait();
	
	for (i=0; i<6;i++)
	{
		for (j=0;j<8;j++)
		{
			k++;
			if (k > valves)
			{
				return 0;
			}
			if (!(del_valves[i] & (1<<j)))
			{
				return k;
			}
			
			if (48 == k)
			{
				return 0;
			}
		}
	}
	
	return k;
}

void recovery_system(void)
{
	int i = 0;
	LCD_clear();
	LCD_show_strP(3,3,PSTR("System Recovery"));
	
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)BandStatusAddr, 0);
	
	for (i = DevcodeFlagAddr; i < (EpSchedule1Addr+sizeof(EpSchedule)); i++)    //16.7.6  修改为“EpSchedule1Addr”
	{
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t *)i, 0);
	}
	
	devparam_init();
	eeprom_busy_wait();
	eeprom_write_block(&devparam,(pDevParam)DevParamAddr, sizeof(DevParam));
	eeprom_busy_wait();
	
	uchar del_valves[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	eeprom_write_block(del_valves,(uint8_t *)DeleteSchAddr,sizeof(del_valves));
	eeprom_busy_wait();
	
	eeprom_write_byte((uint8_t *)WaterUsedHead, 0);
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)WaterUsedTag, 0);
	eeprom_busy_wait();

	eeprom_write_word((uint16_t *)SprayLimitTimeAddr, 0xffff);
	eeprom_busy_wait();
	
	finetune_init();
	
	while(1);
}

void eeprom_write_water_used(SLink *L, WaterUsedStruct wus)
{
	uchar head = 0;
	uchar tag = 0;
	
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)WaterUsedHead);
	eeprom_busy_wait();
	tag = eeprom_read_byte((uint8_t *)WaterUsedTag);
	eeprom_busy_wait();
	if ((head > WaterUsedNum) || (tag > WaterUsedNum))
	{
		head = 0;
		tag = 0;
	}
	
	eeprom_write_block(&wus, (pWaterUsedStruct)(WaterUsed1Addr + (tag * sizeof(WaterUsedStruct))), sizeof(WaterUsedStruct));
	eeprom_busy_wait();
	
	if(head == tag)
	{
		tag++;
		if (tag > WaterUsedNum)
		{
			tag = 0;
		}
	}
	else if (head > tag)
	{
		tag++;
		if (tag == head)
		{
			head++;
		}
		if (head > WaterUsedNum)
		{
			head = 0;
		}
	}
	else if (head < tag)
	{
		tag++;
		
		if (tag > WaterUsedNum)
		{
			tag = 0;
		}
		if (tag == head)
		{
			head++;
		}
	}
	
	eeprom_write_byte((uint8_t *)WaterUsedHead, head);
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)WaterUsedTag, tag);
	eeprom_busy_wait();
}

void eeprom_read_water_used(SLink *L)
{
	uchar head = 0;
	uchar tag = 0;
	int i = 0;
	WaterUsedStruct wus;
	pWaterUsedStruct pwus = NULL;
	
	i = get_length(L);
	if (i)
	{
		return;
	}
	
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)WaterUsedHead);
	eeprom_busy_wait();
	tag = eeprom_read_byte((uint8_t *)WaterUsedTag);
	eeprom_busy_wait();
	if ((head > WaterUsedNum) || (tag > WaterUsedNum))
	{
		head = 0;
		tag = 0;
	}
	
	if (head == tag)
	{
		return;
	}
	else
	{
		if (tag == 0)
		{
			tag = WaterUsedNum;
		}
		else
		{
			tag--;
		}
		
		eeprom_read_block(&wus, (pWaterUsedStruct)(WaterUsed1Addr + tag * sizeof(WaterUsedStruct)), sizeof(WaterUsedStruct));
		eeprom_busy_wait();

		pwus = (pWaterUsedStruct)malloc(sizeof(WaterUsedStruct));
		if (pwus != NULL)
		{
			*pwus = wus;
			ins_elem(L,pwus,1);
		}
	}
	
	eeprom_write_byte((uint8_t *)WaterUsedHead, head);
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)WaterUsedTag, tag);
	eeprom_busy_wait();
}

/************************************************************************/
/* 函数名：save_water_used*/																			 
/* 功能：保存用水量*/																					
/*参数：无*/																							
/*返回值：无*/																						
/************************************************************************/
void save_all_water_used( SLink *L )
{
	SLink *p = L->next;
	pWaterUsedStruct pwus = NULL;

	while(NULL != p)
	{
		pwus = (pWaterUsedStruct)(p->data);
		eeprom_write_water_used(L, *pwus);
		p = p->next;
	}
}

void save_mac( char *mac )
{
	eeprom_busy_wait();
	eeprom_write_block(mac, (uint8_t *)DevcodeAddr, LenOfDevcodes);
	set_mac_flag(1);
}

unsigned char mac_justice(unsigned char temp)
{
	switch(temp)
	{
		case '0':return 0x00;
		case '1':return 0x01;
		case '2':return 0x02;
		case '3':return 0x03;
		case '4':return 0x04;
		case '5':return 0x05;
		case '6':return 0x06;
		case '7':return 0x07;
		case '8':return 0x08;
		case '9':return 0x09;
		case 'a':return 10;
		case 'b':return 11;
		case 'c':return 12;
		case 'd':return 13;
		case 'e':return 14;
		case 'f':return 15;
		case 'A':return 10;
		case 'B':return 11;
		case 'C':return 12;
		case 'D':return 13;
		case 'E':return 14;
		case 'F':return 15;
		default:return 0x00;
	}
}

unsigned char get_mac_flag(void)
{
	unsigned char flag = 0;
	eeprom_busy_wait();
	flag = eeprom_read_byte((uint8_t *)DevcodeFlagAddr);
	eeprom_busy_wait();
	return flag;
}



void get_mac(unsigned char *mac)
{
	unsigned char i = 0;
	unsigned char j = 0;
	char mac_temp[12] = {0};
	int flag = 0;
	flag = get_mac_flag();
	if (1 == flag)
	{
		eeprom_read_block(mac_temp, (uint8_t *)DevcodeAddr, LenOfDevcodes);
		eeprom_busy_wait();
		for(i = 0;i < 6;i++)
		{
			mac[i] = (mac_justice(mac_temp[j]) << 4) + mac_justice(mac_temp[j + 1]);
			j = j + 2;
		}
	}
}

void set_backlight_flag(unsigned char data)
{
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)BackLightAddr, data);
}

int get_backlight_flag(void)
{
	int flag = 0;
	eeprom_busy_wait();
	flag = eeprom_read_byte((uint8_t *)BackLightAddr);
	eeprom_busy_wait();
	return flag;
}

void set_timemode_flag(unsigned char timemode)
{
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)TimeModeAddr,timemode);
}

void get_timemode_flag(unsigned char * timemode)
{
	unsigned char timemode_temp = 0;
	eeprom_busy_wait();
	timemode_temp = eeprom_read_byte((uint8_t *)TimeModeAddr);
	if(timemode_temp != TimeMode_12 && timemode_temp != TimeMode_24){*timemode = TimeMode_24;}
	else{*timemode = timemode_temp;}	
}

int get_version_id(void)
{
	return version;	
}

void get_vesion_str(char *res)    //167=>1.6.7
{
	int version_id = 0;
	
	version_id = get_version_id();
	
	int unit = version_id%10;
	int ten = version_id/10%10;
	int hundred = version_id/100%10;
	
	sprintf_P(res, PSTR("%d.%d.%d"), hundred, ten, unit);
}

void save_domain(char *serve)
{
	eeprom_busy_wait();
	eeprom_write_block(serve, (uint8_t *)ServeAddr, strlen(serve)+1);
	eeprom_busy_wait();
}

void read_daomain(char *dest)
{
	eeprom_busy_wait();
	eeprom_read_block(dest, (uint8_t *)ServeAddr, LenOfServe);
	eeprom_busy_wait();
}


void eep_save_spray(Spray spray)
{
	uchar head = 0;
	uchar tag = 0;
	
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)SparyHeadAddr);
	eeprom_busy_wait();
	tag = eeprom_read_byte((uint8_t *)SparyTagAddr);
	eeprom_busy_wait();
	if ((head > SprayEepromNum) || (tag > SprayEepromNum))
	{
		head = 0;
		tag = 0;
	}
	
	eeprom_write_block(&spray, (pSpray)(SparyFirstAddr + (tag * sizeof(Spray))), sizeof(Spray));
	eeprom_busy_wait();
	
	if(head == tag)
	{
		tag++;
		if (tag > SprayEepromNum)
		{
			tag = 0;
		}
	}
	else if (head > tag)
	{
		tag++;
		if (tag == head)
		{
			head++;
		}
		if (head > SprayEepromNum)
		{
			head = 0;
		}
	}
	else if (head < tag)
	{
		tag++;
		
		if (tag > SprayEepromNum)
		{
			tag = 0;
		}
		if (tag == head)
		{
			head++;
		}
	}
	
	eeprom_write_byte((uint8_t *)SparyHeadAddr, head);
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)SparyTagAddr, tag);
	eeprom_busy_wait();
}

void eep_read_spray(SLink *L)
{
	uchar head = 0;
	uchar tag = 0;
	int i = 0;
	Spray spray;
	pSpray pspray = NULL;
	
	i = get_length(L);
	if (NULL == L)
	{
		init_list(&spray_link);
	}
	
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)SparyHeadAddr);
	eeprom_busy_wait();
	tag = eeprom_read_byte((uint8_t *)SparyTagAddr);
	eeprom_busy_wait();
	if ((head > SprayEepromNum) || (tag > SprayEepromNum))
	{
		head = 0;
		tag = 0;
	}
	
	if (head == tag)
	{
		return;
	}
	else
	{
		while(head != tag)
		{
			eeprom_read_block(&spray, (pSpray)(SparyFirstAddr + head * sizeof(Spray)), sizeof(Spray));
			eeprom_busy_wait();

			pspray = (pSpray)malloc(sizeof(Spray));
			if (pspray != NULL)
			{
				*pspray = spray;
				i =get_length(L)+1;
				ins_elem(L,pspray,i);
			}
			else
			{
				// 				USART0_send("\r\n malloc error \r\n");
			}
			
			head++;
			if (head > SprayEepromNum)
			{
				head = 0;
			}
		}
	}
}

void save_spray_link( SLink *L )
{
	SLink *p = L->next;
	Spray spr;
	uchar head = 0;
	uchar tag = 0;
	
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)SparyHeadAddr);
	eeprom_busy_wait();
	tag = eeprom_read_byte((uint8_t *)SparyTagAddr);
	eeprom_busy_wait();
	if ((head > SprayEepromNum) || (tag > SprayEepromNum))
	{
		head = 0;
		tag = 0;
	}
	
	head = tag;
	
	while(NULL != p)
	{
		spr = *(pSpray)(p->data);
		p = p->next;
		
		eeprom_busy_wait();
		eeprom_write_block(&spr, (pSpray)(SparyFirstAddr + (tag * sizeof(Spray))), sizeof(Spray));
		eeprom_busy_wait();
		
		if(head == tag)
		{
			tag++;
			if (tag > SprayEepromNum)
			{
				tag = 0;
			}
		}
		else if (head > tag)
		{
			tag++;
			if (tag == head)
			{
				head++;
			}
			if (head > SprayEepromNum)
			{
				head = 0;
			}
		}
		else if (head < tag)
		{
			tag++;
			
			if (tag > SprayEepromNum)
			{
				tag = 0;
			}
			if (tag == head)
			{
				head++;
			}
		}
	}
	
	eeprom_write_byte((uint8_t *)SparyHeadAddr, head);
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)SparyTagAddr, tag);
	eeprom_busy_wait();
}

void eep_update_spray(Spray spr)
{
	uchar head = 0;
	uchar tag = 0;
	Spray spray;
	
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)SparyHeadAddr);
	eeprom_busy_wait();
	tag = eeprom_read_byte((uint8_t *)SparyTagAddr);
	eeprom_busy_wait();
	if ((head > SprayEepromNum) || (tag > SprayEepromNum))
	{
		head = 0;
		tag = 0;
	}
	
	if (head == tag)
	{
		return;
	}
	else
	{
		
		while(head != tag)
		{
			eeprom_busy_wait();
			eeprom_read_block(&spray, (pSpray)(SparyFirstAddr + head * sizeof(Spray)), sizeof(Spray));
			eeprom_busy_wait();
			
			if (spray.zone == spr.zone)
			{
				eeprom_write_block(&spr, (pSpray)(SparyFirstAddr + (head * sizeof(Spray))), sizeof(Spray));
				eeprom_busy_wait();
				return;
			}
			
			head++;
			if (head > SprayEepromNum)
			{
				head = 0;
			}
		}
	}
}

void eep_spray_delete_first(void)
{
	uchar head = 0;
	uchar tag = 0;
	
	eeprom_busy_wait();
	head = eeprom_read_byte((uint8_t *)SparyHeadAddr);
	eeprom_busy_wait();
	tag = eeprom_read_byte((uint8_t *)SparyTagAddr);
	eeprom_busy_wait();
	if ((head > SprayEepromNum) || (tag > SprayEepromNum))
	{
		head = 0;
		tag = 0;
		eeprom_write_byte((uint8_t *)SparyHeadAddr, head);
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t *)SparyTagAddr, tag);
		eeprom_busy_wait();
		return;
	}
	
	head++;
	if (head > SprayEepromNum)
	{
		head = 0;
	}
	eeprom_write_byte((uint8_t *)SparyHeadAddr, head);
	eeprom_busy_wait();
}

void read_param( void )
{
	read_band_status();//读取用户绑定状态
	read_code();//读取设备号和序列码
	read_epschedule();//读取喷淋计划
	read_devparam();//读取设备参数
	read_raindelay();//读取延迟喷淋时间
	read_manuwater_set();//读取手动喷淋设置
	read_finetune();//读取微调参数
	read_spray_time_limit();
	eep_read_spray(spray_link);
}
