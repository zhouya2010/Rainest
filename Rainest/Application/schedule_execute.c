/*
 * schedule_execute.c
 *
 * Created: 2014/7/11 15:41:57
 *  Author: ZY
 */ 
#include "data_rw.h"
#include "schedule_execute.h"
#include "HC595N.h"
#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

extern SLink *sch_link ;
extern SLink * spray_link;
extern FineTune weather_adjust;

/************************************************************************/
/* 函数名：start_spray													*/
/* 功能：执行喷淋开														*/
/*参数：zones: 路数(位表示) howlong：时间								*/
/*返回值：无															*/
/************************************************************************/
void start_spray( uchar zones, uint howlong ) 
{
//	uchar i = 0;   //16.7.7 unused
	Spray spr;
	
	if(OffMode == work_value) 
		return;//OFF模式下，不启用
		
	spr.add_time = current_time;
	spr.adjust = 100;
	spr.type = SparyByApp;
	
	if (zones <= MaxValve)
	{
		spr.zone = zones;
		spr.howlong = howlong;
		spr.src_time = howlong;
		spr.work_status = NoExecute;
		add_spray(spray_link, spr);
	}
}


/************************************************************************/
/* 函数名：schedule_exe													*/
/* 功能：执行单个喷淋计划												*/
/*参数：sch: 喷淋计划  tm：时间											*/
/*返回值：无															*/
/************************************************************************/
void ScheduleExe( Schedule sch, Time tm)
{		
	uchar i = 0;
	float howlong = 0;
	float adjust = 0;
	Spray spr;
	
// 	if(rain_delay.delay_time) return;//正在執行延迟喷淋，退出查询喷淋计划
	if ((((sch.weeks >> (tm.wday-1)) &1) && (sch.mode == BY_WEEKS)) || \
	    (((sch.days >> (tm.mday-1)) &1) && (sch.mode == BY_DAYS)))		//星期匹配或者天数匹配
	{
		for (i = 0; i < TimesOfSchedule; i++)	//匹配四个时段
		{
			if (sch.times[2*i] == tm.hour)	//小时匹配
			{
				if (sch.times[2*i + 1] == tm.min)	//分钟匹配
				{
					if ((sch.howlong[i] > 0) && (sch.howlong[i] <= spray_time_limit))
					{
						adjust = weather_adjust.minadjust[0];
						howlong = sch.howlong[i];
						howlong = howlong * 60;//分钟转换成秒
						
						if((sensor_value == SensorOn) || (adjust == 0) || (rain_delay.delay_time))
						{
							spr.howlong = 0;
							spr.src_time = (uint)howlong;
							if (sensor_value == SensorOn)
							{
								spr.type = SparyBySensor;
							}
							else if(adjust == 0)
							{
								spr.type = SparyBySch;
							}
							else
							{
								spr.type = SparyByDelay;
							}
						}
						else
						{
							if (!sch.is_weather)
							{
								adjust = adjust / 100;
// 								howlong *= adjust;	//喷淋计划需要根据微调调整喷淋时间
								howlong = howlong * adjust * sch.xishu;	//喷淋计划需要根据微调调整喷淋时间
							}
							spr.howlong = (uint)howlong;
							spr.src_time = spr.howlong;
							spr.type = SparyBySch;
						}
						
						spr.zone =sch.zones;
						spr.add_time = tm;
						spr.adjust = weather_adjust.minadjust[0];
						spr.work_status = NoExecute;
						
						if((0 != devparam.mastertag) && (spr.zone == MasterZone)) //master模式下，12路不执行
						{
							continue;
						}
						add_spray(spray_link, spr);
					}
					else
					{
						//时间不合法不执行操作
// 						if (sch.howlong[i] > spray_time_limit)
// 						{
// 							LCD_show_number(0,0,spray_time_limit);
// 						}
					}
				}
			}
		}	
	}			
}

/************************************************************************/
/* 函数名：InitList														*/
/* 功能：链表初始化														*/
/*参数：无																*/
/*返回值：链表指针地址													*/
/************************************************************************/
void init_list( SLink **L)
{
	*L = (SLink *)malloc(sizeof(SLink));
	if (NULL != *L)
	{
		(*L)->data = NULL;
		(*L)->next = NULL;
	}
}

/************************************************************************/
/* 函数名：GetLength													*/
/* 功能：获取链表长度													*/
/*参数：L： 链表地址													*/
/*返回值：																*/
/************************************************************************/
int get_length( SLink *L )
{
	int i = 0;
	SLink *p = L->next;
	while(p != NULL)
	{
		p = p->next;
		i++;
	}
	return i;
}

/************************************************************************/
/* 函数名：LocateZone													*/
/* 功能：获取喷淋计划ID为x的位置										*/
/*参数：L： 链表地址	x：Zone值										*/
/*返回值：i：所在链表位置  0：失败，没有该元素							*/
/************************************************************************/
int locate_zone( SLink *L,uchar x )
{
	int i = 1;
	SLink *p = L->next;
	pSchedule psch;

	while (p != NULL /*&& (x != psch->zones)*/)
	{
		psch = (pSchedule)(p->data);
		if (x == psch->zones)
		{
			break;
		}
		p = p->next;
		i++;
	}

	if (p == NULL)
	{
		return 0;
	}
	else return i;
}


/************************************************************************/
/* 函数名：InsElem														*/
/* 功能：插入一个元素													*/
/*参数：L： 链表地址	x：喷淋计划 i：插入位置							*/
/*返回值：1：成功 0：失败												*/
/************************************************************************/
int ins_elem( SLink *L, void * data, int i )
{
	int j = 1;
	SLink *p = L;
	SLink *s;
	
	if (i<1 || i>(get_length(L)+1))
	{
		return 0;
	}
	else
	{
		s = (SLink *)malloc(sizeof(SLink));
		if (s != NULL)
		{
			s->data = data;
			s->next = NULL;
			
			while(j<i)
			{
				p = p->next;
				j++;
			}
			s->next = p->next;
			p->next = s;
			return 1;
		}
		else
		{
			return 0;
		}
	}
}

/************************************************************************/
/* 函数名：DelElem														*/
/* 功能：删除一个元素													*/
/*参数：L： 链表地址	i：删除位置										*/
/*返回值：1：成功 0：失败												*/
/************************************************************************/
int DelElem( SLink *L, int i )
{
	int j = 1;
	SLink *p = L;
	SLink *q ;
	if ( i<1 || i>get_length(L) )
	{
		return 0;
	}
	else
	{
		while( j<i)
		{
			p = p->next;
			j++;
		}
		q = p->next;
		p->next = q->next;

		free(q->data);
		q->data = NULL;
		free(q);
		q = NULL;
		return 1;
	}
}

/************************************************************************/
/* 函数名：GetAdrr														*/
/* 功能：获取位置为i的元素指针											*/
/*参数：L： 链表地址	i：元素位置										*/
/*返回值：p：成功，返回该元素地址 0：失败								*/
/************************************************************************/
SLink* get_addr( SLink *L, int i )
{
	int j = 1;
	SLink *p = L->next;
	while( j<i )
	{
		if(NULL == p)
			break;
		p = p->next;
		j++;
	}
	return p;
}

/************************************************************************/
/* 函数名：UpdataSch													*/
/* 功能：更新喷淋计划													*/
/*参数：NewSch：新的喷淋计划											*/
/*返回值：无															*/
/************************************************************************/
void updata_sch( SLink *L, Schedule NewSch )
{
	int i = 0;
	SLink *p = NULL;
	i = locate_zone(L, NewSch.zones);
	if (i)
	{
		p = get_addr(L,i);
		*(pSchedule)(p->data) = NewSch;
//		update_schedule(i, NewSch);
	}
}

/************************************************************************/
/* 函数名：InsertSch													*/
/* 功能：添加一个新的喷淋计划											*/
/*参数：sch：喷淋计划,L: 链表首地址									    */
/*返回值：无															*/
/************************************************************************/
void insert_sch(SLink *L, Schedule sch)  
{                                                             
	int i = 0;
	SLink *p = NULL;
	pSchedule psch = NULL;

	i = locate_zone(L, sch.zones);
	if (i)
	{
		p = get_addr(L,i);
		*(pSchedule)(p->data) = sch;
	}
	else
	{
		psch = (pSchedule)malloc(sizeof(Schedule));
		if (psch != NULL)
		{
			*psch = sch;
			i =get_length(L)+1;
			if(!ins_elem(L,psch,i))
			{
				free(psch);
				psch = NULL;
			}
		}
	}
}


/************************************************************************/
/* 函数名：DelSch														*/
/* 功能：删除喷淋计划													*/
/*参数：zone：喷淋计划的zone											*/
/*返回值：无															*/
/************************************************************************/
void del_sch( SLink *L, uchar zone )
{
	int i = 0;
	i = locate_zone(L,zone);
	if (i > 0)
	{
		delete_epschedule(L,zone); //16.7.6 需注意，删除喷淋计划时，为了能同步eeprom中的数据，必须将delete_epschedule放置于DelElem之前
		DelElem(L,i);
//		save_schedule(L);
	}
}

void del_all_sch(SLink *L)
{
	clear_link(L);
//	reset_sch_num();
	reset_epschedule();  //16.7.6
}

/************************************************************************/
/* 函数名：SchInquire													*/
/* 功能：查看喷淋计划是否需要执行										*/
/*参数：L：喷淋计划链表		tm：当前时间								*/
/*返回值：无															*/
/************************************************************************/
void sch_inquire( SLink *L,Time tm )
{
	SLink *p = L->next;
	Schedule sch;
	
	while(NULL != p)
	{
		sch = *(pSchedule)(p->data);
		ScheduleExe(sch, tm);
		p = p->next;
	}
}

/************************************************************************/
/* 函数名：GetUploadSchAddr												*/
/* 功能：获取未上传的喷淋计划											*/
/*参数：L：喷淋计划链表													*/
/*返回值：喷淋计划指针													*/
/************************************************************************/
pSchedule get_upload_sch_addr( SLink *L )
{
	SLink *p = L->next;
	pSchedule psch = NULL;

	while(p != NULL)
	{
		psch = (pSchedule)(p->data);
		
		if(Uploaded != psch->upload_status)
		{
			return	psch;
		}
		else
		{
			p = p->next;
		}
	}
	return psch;
}

/************************************************************************/
/* 函数名：LocateSpray													*/
/* 功能：根据路数获取所在位置											*/
/*参数：L：喷淋计划链表													*/
/*返回值：该路数位置													*/
/************************************************************************/
int locate_spray( SLink *L,uchar zone )
{
	int i = 1;
	SLink *p = L->next;
	pSpray psch;

	while (p != NULL)
	{
		psch = (pSpray)(p->data);
		if (zone == psch->zone)
		{
			break;
		}
		p = p->next;
		i++;
	}

	if (p == NULL)
	{
		return 0;
	}
	else return i;
}

/************************************************************************/
/* 函数名：AddSpray														*/
/* 功能：添加及时喷淋													*/
/*参数：L：及时喷淋链表 spr：及时喷淋结构体								*/
/*返回值：无															*/
/************************************************************************/
void add_spray( SLink *L, Spray spr )
{
	int i = 0;
	SLink *p = NULL;
	pSpray pspr = NULL;
			
	i = locate_spray(L, spr.zone);
	if (i)
	{
		p = get_addr(L,i);
		pspr = (pSpray)(p->data);
		pspr->work_status = ExeCompleted;
		
		add_spray_to_waterused(pspr);//添加到用水队列
		DelElem(L, i);//删除该喷淋，节省内存
		
		if ((spr.zone > 0) && (spr.zone <= MaxValve))
		{
			pspr = (pSpray)malloc(sizeof(Spray));
			if (pspr != NULL)
			{
				*pspr = spr;
				if(!ins_elem(L,pspr,i))
				{
					free(pspr);
					pspr = NULL;
				}
			}
			eep_update_spray(spr);
		}
	}
	else
	{
		if ((spr.zone > 0) && (spr.zone <= MaxValve))
		{
			pspr = (pSpray)malloc(sizeof(Spray));
			if (pspr != NULL)
			{
				*pspr = spr;
				i =get_length(L)+1;
				if(!ins_elem(L,pspr,i))
				{
				
					free(pspr);
					pspr = NULL;
				}
			}
			eep_save_spray(spr);
			spray_open();
		}
	}	
}

/************************************************************************/
/* 函数名：DelSpray														*/
/* 功能：删除及时喷淋													*/
/*参数：L：及时喷淋链表 zone：及时喷淋的路数							*/
/*返回值：无															*/
/************************************************************************/
void del_spray( SLink *L, uchar zone )
{
	int i = 0;
	i = locate_spray(L,zone);
	if (i)
	{
		DelElem(L,i);
	}
}

/************************************************************************/
/* 函数名：CleanSpray													*/
/* 功能：清除喷淋时间为0的路数											*/
/*参数：L：及时喷淋链表													*/
/*返回值：无															*/
/************************************************************************/
void clean_spray( SLink *L )
{
	SLink *p = L->next;
	pSpray pspr = NULL;

	while(NULL != p)
	{
		pspr = (pSpray)(p->data);
		if (((MaxManuTime * 60ul) < pspr->howlong) || (pspr->zone > MaxValve) || (pspr->zone < 1)) //16.7.8   "integer overflow in expression"  
		{
			del_spray(L, pspr->zone);
			p = L->next;
			continue;
		}
		p = p->next;
	}
}

/************************************************************************/
/* 函数名：ClearLink													*/
/* 功能：清空链表														*/
/*参数：L：及时喷淋链表													*/
/*返回值：无															*/
/************************************************************************/
void clear_link( SLink *L )
{
	int i = 0;
	SLink *p;
	i = get_length(L);
	while(i)
	{
		p = get_addr(L,i);
		if (NULL != p)
		{
			DelElem(L, i);
		}
		i--;
	}
}

static void AddWaterUsedToLink(SLink *L, WaterUsedStruct wus)
{
	int i = 0;
	pWaterUsedStruct pwus = NULL;

	i = get_length(L) + 1;
	
	pwus = (pWaterUsedStruct)malloc(sizeof(WaterUsedStruct));
	if (pwus != NULL)
	{
		*pwus = wus;
		if(!(ins_elem(L,pwus,i)))
		{
			free(pwus);
			pwus = NULL;
		}
	}
}

void add_water_used( SLink *L, WaterUsedStruct wus )
{
	int i = 0;
	SLink *p = NULL;
	pWaterUsedStruct pwus = NULL;
	
	i = get_length(L);
	
	if (i)
	{
		p = get_addr(L,i);
		if(NULL != p)
		{
			pwus = (pWaterUsedStruct)(p->data);
			if((pwus->zone == wus.zone) && (!memcmp(&(pwus->add_time), &wus.add_time, sizeof(Time))))
			{
				pwus->howlong = wus.howlong;
				pwus->end_time = wus.end_time;
			}
			else 
			{
				 AddWaterUsedToLink(L, wus);
				 i = get_length(L);
				 if(i > 1) //队列限制3个，超过时把第1个写入EEPROM
				 {
					 p = get_addr(L,1);
					 if(NULL != p)
					 {
						 pwus = (pWaterUsedStruct)(p->data);
						 eeprom_write_water_used(L, *pwus);
						 del_first_water_used(L);
					 }
				 }
			}
		}
	}
	else
	{
		AddWaterUsedToLink(L, wus);
	}
}

void del_first_water_used( SLink *L )
{
	int i = 0;
		
	i = get_length(L);
	if (i)
	{
		DelElem(L,1);
	}
}

void del_last_water_used( SLink *L )
{
	int i = 0;
	
	i = get_length(L);
	if (i)
	{
		DelElem(L,i);
	}
}

int schedule_is_legal(Schedule sch)
{
	if ((sch.zones < 1) || (sch.zones > MaxValve))
	{
		return 0;
	}
	
	
	for (int i = 0; i < TimesOfSchedule; i++)
	{
		if (sch.howlong[i] > MaxManuTime)
		{
			return 0;
		}
	}
	
	for (int i = 0; i < (2 * TimesOfSchedule); i++)
	{
		if (i%2 == 0)
		{
			if (sch.times[i] > 23)
			{
				return 0;
			}
			
		}
		else
		{
			if (sch.times[i] > 59)
			{
				return 0;
			}
		}
	}
	
	if (sch.mode > BY_DAYS)
	{
		return 0;
	}
	
	if (sch.xishu > 5)
	{
		return 0;
	}
	
	return 1;
}
