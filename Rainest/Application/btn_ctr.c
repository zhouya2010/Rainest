/*
 * btn_ctr.c
 *
 * Created: 2014/7/9 13:56:03
 *  Author: ZY
 */ 
#include "btn_ctr.h"
#include "CMD_Ctr.h"
#include "HC595N.h"
#include "RTC.h"
#include "data_rw.h"
#include "schedule_execute.h"
#include "timer.h"
#include "typedef.h"
#include <string.h>
#include <stdlib.h>
#include <avr/wdt.h>

uchar copy_fun_flag = 0;

uchar manu_sel = 0;
uint ManuValveTime[MaxValveManu] = {0};
uint ManuRemaningTime = 0;
extern SLink *sch_link;

extern RainDelay rain_delay;
extern uchar CurrentPage;//当前显示页面ID
extern uchar NextPage;//下个显示页面ID

int rain_delay_temp = 0;
static uchar reset_raindelay_falg = 0;
uchar setting_delaytime_flag = 0;	
uchar show_success_flag = 0;
extern uchar half_sec_count;

static uchar  ManuWaterPageTemp = ManuWaterPage;
uchar watering_flag = WateringOFF;//是否正在喷淋表示

// SchReTime SchRmT;

Time tm_setup;
uchar timemode_setup; //16.9.22
static uchar  ConfigurePageTemp = ConfigPage;
uchar time_option = 0;//选择设置时间项
uchar config_select = 0;

uchar adjust_value_temp[5] = {0};
uchar adjust_days[5] = {0};
uchar adjust_sel = 0;
FineTune weather_adjust;
uchar adjust_mode_temp = 0;	//位表示某一天

static uchar SchPageTemp = SchSelValvePage;
uchar plan_sel = 1;
Schedule new_sch;
uchar set_timer_option = 0;
uchar weekday_sel = 0;
uchar days_sel = 0;
uchar sch_id = 1;
uchar sch_mod_sel = 1;

extern SLink *spray_link;


/************************************************************************/
/* 函数名：AutoModeCtr													*/
/* 功能：自动模式档显示控制												*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
void auto_mode_ctr(void)
{	
// 	 if (WateringOFF == watering_flag)
// 	 {
// 		 NextPage = AutoModePage;
// 	 }
// 	 if (WateringStrat == watering_flag)
// 	 {	
// 		  NextPage = WaterAutoPage;
// 	 }

	if (get_length(spray_link))
	{
		NextPage = WaterAutoPage;
	}
	else
	{
		NextPage = AutoModePage;
	}
}	

/************************************************************************/
/* 函数名：get_max_time													*/
/* 功能：获取手动设置的最大时间											*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
uint get_max_time(uint *pTime)
{
	uint max_time = 0;
	uint i = 0;
	for (i= 0; i<MaxValveManu; i++)
	{
		if(max_time < pTime[i] )
		max_time = pTime[i];
	}
	return max_time;
}

/************************************************************************/
/* 函数名：ManuWaterCtr													*/
/* 功能：手动喷淋界面显示控制											*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
void manu_water_ctr(void)
{
	int i = 0;
	Spray spr;
	int per_valve = valves/3;
	
	switch(CurrentPage)
	{
		case ManuWaterPage:		
			{
				ManuWaterPageTemp = CurrentPage;
				switch(operate_value)
				{
					case  Operate1:	// 按下"RESET"号键，全部清零
					{
						for ( i = 0; i<MaxValveManu; i++)
						{
							ManuValveTime[i] = 0;
						}
						break;
					}
					case  Operate2:	// 按下"-"号键，时间减1
					{
						if(ManuValveTime[manu_sel])
						{
							ManuValveTime[manu_sel] --;
						}
						break;
					}
					case  Operate3:	//按下“+”号键，时间加1
					{
						if(ManuValveTime[manu_sel] < MaxManuTime)
						{
							ManuValveTime[manu_sel] ++;
						}
						break;
					}
					case  Operate4:	//按下START键，切换到正在喷淋界面
					{
						ManuRemaningTime = get_max_time(ManuValveTime);
						if (ManuRemaningTime > spray_time_limit)
						{
							NextPage = ManuWaterLimitPage;
						}
						else
						{
							NextPage = ManuWaterStartPage;
							for ( i = 0; i<MaxValveManu; i++)
							{
								wdt_reset();           //16.7.26  36路阀门数据EEPROM保存处理时间过长，如果快速操作manul water
								if (ManuValveTime[i])  //的start - stop命令则会导致看门狗重启，数据保存出现异常，重启后start命令
								{                      //会导致设备异常重启，添加喂狗命令后恢复正常
									spr.zone = i + 1;
									
									if ((0 != devparam.mastertag) && (MasterZone == spr.zone))
									{
										continue;
									}
									
									spr.howlong = ManuValveTime[i] * 60;
									spr.src_time = spr.howlong;
									spr.add_time = current_time;
									spr.adjust = 100;
									spr.type = SparyByManu;
									spr.work_status = NoExecute;
									add_spray(spray_link, spr);
								}
							}
						}
						save_manuwater_set();
						break;
					}
					default: break;
				}
				
				switch(spray_value)
				{
					case KEY1:
					{
						manu_sel++;
						if(manu_sel > (per_valve -1)) manu_sel = 0;
						break;
					}
					
					case KEY2:
					{
						manu_sel++;
						if((manu_sel > (per_valve * 2 - 1)) || (manu_sel < per_valve)) manu_sel = per_valve;
						break;
					}
					
					case KEY3:
					{
						manu_sel++;
						if((manu_sel > (per_valve * 3 - 1)) || (manu_sel < (per_valve * 2))) manu_sel = per_valve * 2;
						break;
					}
					
					default:	break;
				}
				break;
			}
		case ManuWaterStartPage:
			{
				ManuWaterPageTemp = CurrentPage;
				if(!get_length(spray_link))
				{
					NextPage = ManuWaterPage;
				}
				if (Operate4 == operate_value)	//按下STOP键，返回主界面
				{
					NextPage = ManuWaterPage;
					spray_close_all(spray_link);
				}
				break;
			}
			
		case ManuWaterLimitPage:
			{
				ManuWaterPageTemp = CurrentPage;
				switch(operate_value)
				{
					case Operate1:
					case Operate4:
						NextPage = ManuWaterPage;
						break;
					default:break;
				}
				break;
			}
		default: 
				NextPage = ManuWaterPageTemp;	
				break;
	}
}


void new_sch_init(void)
{
	memset(&new_sch,0,sizeof(Schedule));
	new_sch.times[0] = 6;
	new_sch.times[2] = 10;
	new_sch.times[4] = 14;
	new_sch.times[6] = 18;
	new_sch.xishu = 1.0;
	set_timer_option = 0;
	weekday_sel = 0;
	days_sel = 0;
	sch_mod_sel = 1;
}

void SchMainCtr(void)			//喷淋计划总控制函数
{
	switch(operate_value)
	{
		case Operate4:
		{
			NextPage = SchSelValvePage;
			break;
		}
		default: break;
	}
}

void SchSelValveCtr(void)		//
{
	uchar i = 0;
	SLink *p = NULL;
	int page_num = valves/12 * 4;
	
	switch(operate_value)
	{
		case Operate1:
			i = locate_zone(sch_link, plan_sel);
			if (i)
			{
				NextPage = SchDelPage;
			}	
			else
			{
				NextPage = SchNoPlanPage;
			}
			break;
			
		case Operate2:
			if(get_weather_flag == GetWeatherOk)
			get_schedule_flag = UpdataAllSch;
			break;
			
		case Operate3:
			i = locate_zone(sch_link, plan_sel);
			if(copy_fun_flag)		//粘贴操作
			{
				copy_fun_flag = 0;
				if(i)
				{
					NextPage = SchConfirmCoverPage;
				}
				else
				{
					new_sch.upload_status = NoUpload;					//更新标志，zones
					new_sch.zones = plan_sel;
					insert_epschedule(sch_link,new_sch);        //16.7.6
					insert_sch(sch_link, new_sch);    
//					save_schedule(sch_link);      
				}
			}
			else                     //复制操作
			{
				if(i)
				{
					copy_fun_flag = 1;
					p = get_addr(sch_link,i);	//获取复制的数据					
					new_sch = *(pSchedule)(p->data);
				}
				else
				{
					NextPage = SchNoPlanPage;
				}
			}
			break;
			
		case Operate4:
			i = locate_zone(sch_link, plan_sel);
			if (i)
			{
				p = get_addr(sch_link, i);
				new_sch = *(pSchedule)(p->data);
				
				set_timer_option = 0;
				weekday_sel = 0;
				days_sel = 0;
				sch_mod_sel = 1;
			}
			else
			{
				new_sch_init();
				new_sch.zones = plan_sel;
			}
			NextPage = SchSetTimePage;
			break;
		
		default: 
			break;
	}
	
	switch(spray_value)
	{
		case KEY1:
		{
			plan_sel++;
			if(plan_sel > page_num) plan_sel = 1;
			break;
		}
		
		case KEY2:
		{
			plan_sel++;
			if((plan_sel > page_num * 2) || (plan_sel < (page_num + 1))) 
				plan_sel = page_num + 1;
			break;
		}
		
		case KEY3:
		{
			plan_sel++;
			if((plan_sel > page_num * 3) || (plan_sel < (page_num * 2 + 1))) 
				plan_sel = page_num * 2 + 1;
			break;
		}
		
		default:	
			break;
	}
}

void sch_to_tmr(uint *tmr)
{
	uchar i = 0;
	uchar j = 0;
	uchar k = 0;
	
	for(i = 0; i < 12; i++)
	{
		if(!((i+1)%3))
		{
			tmr[i] = new_sch.howlong[k];
			k++;
		}
		else
		{
			tmr[i] = new_sch.times[j];
			j++;
		}
	}
}

void tmr_to_sch(uint *tmr)
{
	uchar i = 0;
	uchar j = 0;
	uchar k = 0;
	
	for(i = 0; i < 12; i++)
	{
		if(!((i+1)%3))
		{
			new_sch.howlong[k] = tmr[i];
			k++;
		}
		else
		{
			new_sch.times[j] = tmr[i];
			j++;
		}
	}
}

void sch_timer_plus(uchar option)
{
	uint tmr[12] = {0};
	sch_to_tmr(tmr);
	if(0 == option%3)
	{
		tmr[option]++;
		if(tmr[option] > 23)
		tmr[option] = 0;
	}
	if(1 == option%3)
	{
		tmr[option]++;
		if(tmr[option] > 59)
			tmr[option] = 0;
	}
	if(2 == option%3)
	{
		if(tmr[option] < MaxManuTime)
			tmr[option]++;
	}
	tmr_to_sch(tmr);
}

void sch_timer_sub(uchar option)
{
	uint tmr[12] = {0};
	sch_to_tmr(tmr);
	if(0 == option%3)
	{
		if(tmr[option] == 0)
			tmr[option] = 24;
		tmr[option]--;
	}
	if(1 == option%3)
	{
		if(tmr[option] == 0)
			tmr[option] = 60;
		tmr[option]--;
	}
	if(2 == option%3)
	{
		if(tmr[option])
			tmr[option]--;
	}
	tmr_to_sch(tmr);
}

void SchSetTimerCtr(void)
{
	uchar i = 0;
	uchar j = 0;
	switch(operate_value)
	{
		case Operate1:
			set_timer_option++;
			if (set_timer_option > 11)
			{
				set_timer_option = 0;
			}
			break;
		
		case Operate2:
			sch_timer_sub(set_timer_option);
			break;
		
		case Operate3:
			sch_timer_plus(set_timer_option);
			break;
		
		case Operate4:
		{
			for (i = 0, j = 0; i < TimesOfSchedule; i++)
			{
				if (new_sch.howlong[i] > spray_time_limit)
				{
					NextPage = SchSprayLimitPage;
					return;
				}
				if (new_sch.howlong[i])
				{
					j = 1;
				}
			}
			if (j)
			{
				if (new_sch.weeks > 0)
				{
					sch_mod_sel = 1;
				}
				else if (new_sch.days > 0)
				{
					sch_mod_sel = 3;
				}
				
				NextPage = SchSelModePage;
			}
			else
			{
				NextPage = SchPlsSetTimePage;
			}
			break;
		}
		default: break;
	}
}


void SchSetWeeksCtr(void)
{
	switch(operate_value)
	{
		case Operate1:		//NEXT
			weekday_sel++;
			if(weekday_sel > 6)
			{
				weekday_sel = 0;
			}
			break;
		
		case Operate2:		//CANCEL
			new_sch.weeks &= ~(1<<weekday_sel);
			break;
		
		case Operate3:		//ADD
			new_sch.weeks |= 1<<weekday_sel;
			break;
		
		case Operate4:		//OK
			if (new_sch.weeks)
			{
				new_sch.upload_status = NoUpload;
				insert_epschedule(sch_link,new_sch);  //16.7.6 
				insert_sch(sch_link, new_sch);
//				save_schedule(sch_link);  
				NextPage = SchSelValvePage;
			}
			else
			{
				NextPage = SchPlsSetWeekPage;
			}
			break;
		
		default: break;
	}
}


void SchSetDaysCtr(void)
{
	long i = 1;
	
	switch(operate_value)
	{
		case Operate1:		//NEXT
		days_sel++;
		if(days_sel > 30)
		{
			days_sel = 0;
		}
		break;
		
		case Operate2:		//CANCEL
		new_sch.days &= ~(i<<days_sel);
		break;
		
		case Operate3:		//ADD
		new_sch.days |= i<<days_sel;
		break;
		
		case Operate4:		//OK
		if (new_sch.days)
		{
			new_sch.upload_status = NoUpload;
			insert_epschedule(sch_link,new_sch);  //16.7.6
			insert_sch(sch_link, new_sch);
//			save_schedule(sch_link);    
			NextPage = SchSelValvePage;
		}
		else
		{
			NextPage = SchPlsSetWeekPage;
		}
		break;
		
		default: break;
	}
}

void SchSelModeCtr()
{
	switch(operate_value)
	{
		case Operate1:
			sch_mod_sel += 2;
			if (sch_mod_sel > 3)
			{
				sch_mod_sel = 1;
			}
			break;
		case Operate4:
			if (1 == sch_mod_sel)
			{
				new_sch.mode = BY_WEEKS;
				new_sch.days = 0;
				NextPage = SchSetWeeksPage;
			} 
			else if(3 == sch_mod_sel)
			{
				new_sch.mode = BY_DAYS;
				new_sch.weeks = 0;
				NextPage = SchSetDaysPage;
			}
			break;
		default:
			break;
	}
}

void  SchDelCtr(void)
{
	switch(operate_value)
	{
		case Operate1:	//CANCEL
			NextPage = SchSelValvePage;
			break;
			
		case Operate4://DEL
			set_del_zone(plan_sel);
			del_sch(sch_link, plan_sel);
			NextPage = SchSelValvePage;
			break;
		default: break;
	}
}

void SchPlsSetTimeCtr(void)
{
	switch(operate_value)
	{		
		case Operate1:
		NextPage = SchSelValvePage;
		break;
		case Operate4://OK
		NextPage = SchSetTimePage;
		break;
		default: break;
	}
}

void SchPlsSetWeekCtr(void)
{
	switch(operate_value)
	{
		case Operate1:
		NextPage = SchSelValvePage;
		break;
		case Operate4:
		if (BY_WEEKS == new_sch.mode)
		{
			NextPage = SchSetWeeksPage;
		}
		else if (BY_DAYS == new_sch.mode)
		{
			NextPage = SchSetDaysPage;
		}
		break;
		default: break;
	}
}

void SchNoPlanCtr(void)
{
	switch(operate_value)
	{
		case Operate4://OK
		NextPage = SchSelValvePage;
		break;
		default: break;
	}
}

void SchConfirmCover(void)
{
	switch(operate_value)
	{
		case Operate1:	//CANCEL
		NextPage = SchSelValvePage;
		break;
		
		case Operate4://Cover
		new_sch.upload_status = NoUpload;					//更新标志，zones
		new_sch.zones = plan_sel;
		insert_epschedule(sch_link,new_sch); //16.7.6
		insert_sch(sch_link, new_sch);
//		save_schedule(sch_link);   
		NextPage = SchSelValvePage;
		break;
		
		default: break;
	}
}

void SchSprayLimitCtr(void)
{
	switch(operate_value)
	{
		case Operate1:
		NextPage = SchSelValvePage;
		break;
		case Operate4://OK
		NextPage = SchSetTimePage;
		break;
		default: break;
	}
	
}

/************************************************************************/
/* 函数名：ScheduleCtr													*/
/* 功能：查看喷淋计划LCD显示控制										*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
void schedule_ctr(void)
{
	switch(CurrentPage)
	{
		case ScheduleMainPage:
			SchMainCtr();
			break;
		
		case SchSelValvePage:
			SchSelValveCtr();
			break;
			
		case SchSetTimePage:
			SchSetTimerCtr();
			break;
		
		case SchSetWeeksPage:
			SchSetWeeksCtr();
			break;
			
		case SchDelPage:
			 SchDelCtr();
			break;
			
		case SchPlsSetTimePage:
			SchPlsSetTimeCtr();
			break;
			
		case SchPlsSetWeekPage:
			SchPlsSetWeekCtr();
			break;
		case SchNoPlanPage:
			SchNoPlanCtr();
			break;
			
		case SchConfirmCoverPage:
			SchConfirmCover();		
			break;
			
		case SchSetDaysPage:
			SchSetDaysCtr();
			break;
			
		case SchSelModePage:
			SchSelModeCtr();
			break;
			
		case SchSprayLimitPage:
			SchSprayLimitCtr();
			break;
			
		default:
			NextPage = SchPageTemp;	
			break;
	}
}

void show_success_init(void)
{
	half_sec_count = 0;
	show_success_flag = 1;
}

void set_rain_delay_temp(void)
{
	rain_delay_temp = rain_delay.delay_time; //转换成小时
}

void set_rain_delay(void)
{
	rain_delay.delay_time = rain_delay_temp;//转换成分钟
	rain_delay.begin_time = current_time;
}

void rain_delay_sub(void)
{
	if (rain_delay_temp == 0)
	{
		return;
	}
	else if (rain_delay_temp <= 10)
	{
		rain_delay_temp--;
	}
	else if (rain_delay_temp <= 60)
	{
		if (rain_delay_temp%10)
		{
			rain_delay_temp -= rain_delay_temp%10;
		}
		else
		{
			rain_delay_temp -= 10;
		}
	}
	else if (rain_delay_temp <= 360)
	{
		if (rain_delay_temp%60)
		{
			rain_delay_temp -= rain_delay_temp%60;
		}
		else
		{
			rain_delay_temp -= 60;
		}
	}
	else if (rain_delay_temp <= MaxRainDelayTime)
	{
		if (rain_delay_temp%360)
		{
			rain_delay_temp -= rain_delay_temp%360;
		}
		else
		{
			rain_delay_temp -= 360;
		}
	}
}

void rain_delay_plus(void)
{
	if (rain_delay_temp > 60)
	{
		rain_delay_temp -= rain_delay_temp%60;
	}
	else if(rain_delay_temp > 0)
	{
		rain_delay_temp -= rain_delay_temp%10;
	}
	
// 	if (rain_delay_temp < 10)
// 	{
// 		rain_delay_temp++;
// 	}
	if (rain_delay_temp < 60)
	{
		rain_delay_temp += 10;
	}
	else if (rain_delay_temp < 360)
	{
		rain_delay_temp += 60;
	}
	else if (rain_delay_temp < MaxRainDelayTime)
	{
		rain_delay_temp += 360;
		if (rain_delay_temp > MaxRainDelayTime)
		{
			rain_delay_temp = MaxRainDelayTime;
		}
	}
}
/************************************************************************/
/* 函数名：RainDelayCtr													*/
/* 功能：查看喷淋计划LCD显示控制										*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
void rain_delay_ctr( void )
{
	if (!(reset_raindelay_falg && setting_delaytime_flag))
	{
		set_rain_delay_temp();
		reset_raindelay_falg = 1;
	}
	
	if(RainDelayPage == CurrentPage)
	{
		switch(operate_value)
		{
			case Operate1:
			{
				setting_delaytime_flag = 1;
				rain_delay_temp = 0;
				break;
			}
			case Operate2:								//按下“-”号键
			{
				rain_delay_sub();
				setting_delaytime_flag = 1;
				break;
			}
			
			case Operate3:								//按下“+”号键
			{
				rain_delay_plus();
				setting_delaytime_flag = 1;
				break;
			}
			
			case Operate4:					//SAVE
			{
				setting_delaytime_flag = 0;
				reset_raindelay_falg = 0;
				set_rain_delay();
				save_raindelay();
				show_success_init();
				set_raindelay_flag = NeedSetRainDelay;
				break;
			}
			default: break;
		}
	}
	else
	{
		reset_raindelay_falg = 0;
		set_rain_delay_temp();
		NextPage = RainDelayPage;
	}
}

void SetupDateTimePlus(pTime tm, uchar option)
{
	switch(option)
	{
		case SetHour:
			tm->hour++;
			if(tm->hour > 23) tm->hour = 0;
			break;
		case SetMin:
			tm->min++;
			if(tm->min > 59) tm->min = 0;
			break;
		case SetTimeMode:
			if(timemode_setup == TimeMode_12){timemode_setup = TimeMode_24;}
			else{timemode_setup = TimeMode_12;}
			break;
		case SetWday:
			tm->wday++;
			if(tm->wday > 7)  tm->wday = 1;
			break;
		case SetMday:
			tm->mday++;
			if(tm->mday > get_last_mday(tm->year, tm->mon)) 
			{
				tm->mday = 1;
			}
			break;
		case SetMon :
			tm->mon++;
			if(tm->mon > 12) tm->mon = 1;
			break;
		case SetYear:
			tm->year++;
			if(tm->year>50) tm->year = 12;
			break;
		default:break;
	}
}

void SetupDateTimeSub(pTime tm, uchar option)
{
	switch(option)
	{
		case SetHour:
			if(tm->hour == 0) tm->hour = 24;
			tm->hour--;
			break;
		case SetMin:
			if(tm->min == 0) tm->min = 60;
			tm->min--;
			break;
		case SetTimeMode:
			if(timemode_setup == TimeMode_24){timemode_setup = TimeMode_12;}
			else{timemode_setup = TimeMode_24;}
			break;
		case SetWday:
			tm->wday--;
			if(tm->wday == 0)  tm->wday = 7;
			break;
		case SetMday:
			tm->mday--;
			if(tm->mday == 0)
			{
				tm->mday = get_last_mday(tm->year, tm->mon);
			}
			break;
		case SetMon :
			tm->mon--;
			if(tm->mon == 0) tm->mon = 12;
			break;
		case SetYear:
			tm->year--;
			if(tm->year == 11) tm->year = 50;
			break;
		default:break;
	}
}

void SetupDateTimeCtr(void)
{
	switch(operate_value)
	{
		case Operate1:		//按下"NEXT"键
		{
			time_option++;
			if(time_option > SetYear) time_option = SetHour;
			break;
		}
		
		case Operate2:			//按下“-”号键
		{
			SetupDateTimeSub(&tm_setup, time_option);
			break;
		}
		
		case Operate3:			//按下“+”号键
		{
			SetupDateTimePlus(&tm_setup, time_option);
			break;
		}
		
		case Operate4:		//OK键
		{
			tm_setup.sec = 0;
			RTC_time_set(tm_setup);
			rain_delay.begin_time = tm_setup;
			set_timemode_flag(timemode_setup);
			timemode_flag = timemode_setup;
			save_raindelay();		//更新延迟喷淋设置时间
			weather_adjust.set_t = tm_setup;
			save_finetune(weather_adjust);		//更新微调设置时间
			NextPage = ConfigPage;
			break;
		}
		default: break;
	}
}

void ConfigSelectCtr(void)
{
	switch(operate_value)
	{
		case Operate1:		//"NEXT"键
		{
			config_select++;
			if (config_select > 3)  config_select = 0;
			break;
		}
		
		case Operate4:		//确认键
		{
			switch(config_select)
			{
				case 0:
					NextPage = SetupDateTimePage;
					tm_setup = current_time;
					timemode_setup = timemode_flag;
					break;
					
				case 1:
					NextPage = WifiAccessPage;
					break;
					
				case 2:
					NextPage = RegSmartDevPage;
					break;
					
				case 3:
					NextPage = MasterConfigPage;
					break;
					
				default:
					break;
			}
		}
		default: break;
	}
}

void WifiAccesCtr(void)
{
	switch(operate_value)
	{
		case Operate4:	
			NextPage = ConfigPage;
			break;
			
		default: break;
	}
}

void RegSmartDeveCtr()
{
	switch(operate_value)
	{
		case Operate4:	
		{
			NextPage = ConfigPage;
			break;
		}
		default: break;
	}
}

void MasterConfigCtr(void)
{
	switch(operate_value)
	{
		case Operate1:
		{
			NextPage = ConfigPage;
			sent_master_state = SENT_FAILED;
			devparam.mastertag = 0;
			save_devparam();
			break;
		}
		
		case Operate4:
		{
			NextPage = ConfigPage;
			sent_master_state = SENT_FAILED;
			devparam.mastertag = 1;
			save_devparam();
			break;
		}
		default: break;
	}
}
/************************************************************************/
/* 函数名：ConfigCtr													*/
/* 功能：查看当前日期时间												*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
void config_ctr(void)
{
	switch(CurrentPage)
	{
		case ConfigPage:
			ConfigSelectCtr();
			break;
			
		case SetupDateTimePage:
			SetupDateTimeCtr();
			break;
			
		case WifiAccessPage:
			WifiAccesCtr();
			break;
			
		case RegSmartDevPage:
			RegSmartDeveCtr();
			break;
			
		case MasterConfigPage:
			MasterConfigCtr();
			break;
		
		default:
			NextPage = ConfigurePageTemp;
			break;
	}
}

/************************************************************************/
/* 函数名：OffModeCtr													*/
/* 功能：关闭阀门控制													*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
void off_mode_ctr(void)
{
	spray_close_all(spray_link);
	ManuWaterPageTemp = ManuWaterPage;
	watering_flag = WateringOFF;
	NextPage = OffModePage;
}

/************************************************************************/
/* 函数名：get_adjust_days												*/
/* 功能：获取今后五天的星期												*/
/*参数：days：星期值													*/
/*返回值：无															*/
/************************************************************************/
void get_adjust_days(uchar * days)
{
	uchar i = 0;
	uchar day_temp = current_time.wday;
	
	for (i = 0; i < 5; i++)
	{
		days[i] = day_temp;
		day_temp++;
		if (day_temp > 7)
		{
			day_temp = 1;
		}
	}
}

void weather_adjust_init(void)
{
	adjust_sel = 0;
	adjust_mode_temp = 0;
	get_adjust_days(adjust_days);
	memcpy(adjust_value_temp, weather_adjust.minadjust, sizeof(weather_adjust.minadjust));
}

void set_adjust_mode(uchar sel)
{
	adjust_mode_temp |= ByManual << (4 - sel);
}

/************************************************************************/
/* 函数名：WeatherAdjustCtr												*/
/* 功能：微调控制														*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
 void weather_adjust_ctr(void)
{	
	if (FineTunePage == CurrentPage)
	{
		switch(operate_value)
		{
			case  Operate1:	//NEXT
			{
				adjust_sel++;
				if (adjust_sel > 4)
				{
					adjust_sel = 0;
				}
				break;
			}
			
			case  Operate2:	// 按下"-"号键，时间减1
			{
				adjust_value_temp[adjust_sel] -= 1;
				if (adjust_value_temp[adjust_sel] > MaxtAdjustValue)
				{
					adjust_value_temp[adjust_sel] = MaxtAdjustValue;
				}
				set_adjust_mode(adjust_sel);
				break;
			}
			
			case  Operate3:	//按下“+”号键，时间加1
			{
				adjust_value_temp[adjust_sel] += 1;
				if (adjust_value_temp[adjust_sel] > MaxtAdjustValue)
				{
					adjust_value_temp[adjust_sel] = 0;
				}
				set_adjust_mode(adjust_sel);
				break;
			}
			
			case Operate4:		//OK	
			{
				memcpy(weather_adjust.minadjust, adjust_value_temp, sizeof(adjust_value_temp));
				weather_adjust.set_t = current_time;
				weather_adjust.adjust_mode |= adjust_mode_temp; 
 				save_finetune(weather_adjust);
				show_success_init();
				set_adjust_flag = NeedSetAdjust;
				break;
			}
			default:  	break;
		}
	} 
	else
	{
		weather_adjust_init();
		NextPage = FineTunePage;
	}
 }

void special_features_ctr(void)
{
	NextPage = SpecialFeatPage;
}

void btn_ctr( void )
{
	switch(work_value)
	{
		case AutoMode:	auto_mode_ctr();		break;
		case  ManuWaterMode:	manu_water_ctr(); break;
		case ScheduleView:	 schedule_ctr();	break;
		case RainDelayView: rain_delay_ctr(); break;
		case Configration: config_ctr(); break;
		case  OffMode:	off_mode_ctr();break;
		case  WeatherAdjust: weather_adjust_ctr();break;
		case  SpecialFeatures: special_features_ctr(); break;
		default: break;
	}
	operate_value = 0;
	spray_value = 0;
}
