/*
 * UI.c
 *
 * Created: 2014/6/30 14:01:41
 *  Author: ZY
 */ 
#include "btn_ctr.h"
#include "CMD_Ctr.h"
#include "UI.h"
#include "LCD.h"
#include <stdio.h>
#include "schedule_execute.h"
#include "data_rw.h"
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

extern volatile uchar copy_fun_flag;

uchar updata_display_flag = 1; //更新显示 0-不更新 1-更新
uchar MCDwarning_flag = 1;
uchar CurrentPage = 0;
uchar NextPage = 0;


extern DevParam devparam;//设备参数
extern SLink *sch_link;
extern SLink *spray_link;
extern RainDelay rain_delay;
extern long long spray_state;        // 16.7.6
extern Weather weather;
extern uchar user_band_flag;
extern uchar half_sec_count;
extern uchar one_sec_count;

static uchar manu_sel_temp = 0;
static uchar config_select_temp = 1;
static uchar showed_success_flag = 0;
static uchar plan_sel_temp = 0;
static uchar adjust_sel_temp = 0;
static uchar days_sel_temp = 0;
static uchar sch_mod_sel_temp = 1;

/************************************************************************/
/* 函数名：maun_value_show												*/
/* 功能：手动模式下显示选中的路数，被选中后反显							*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/
void maun_zone_chage(uchar start_zone)
{
	char disp[12] = {0};
	sprintf_P(disp, PSTR("%2d: %-3d"), start_zone+1, ManuValveTime[start_zone]);
	LCD_show_string(3,3,disp);
	start_zone++;
	sprintf_P(disp, PSTR("%2d: %-3d"), start_zone+1, ManuValveTime[start_zone]);
	LCD_show_string(3,13,disp);
	start_zone++;
	sprintf_P(disp, PSTR("%2d: %-3d"), start_zone+1, ManuValveTime[start_zone]);
	LCD_show_string(5,3,disp);
	start_zone++;
	sprintf_P(disp, PSTR("%2d: %-3d"), start_zone+1, ManuValveTime[start_zone]);
	LCD_show_string(5,13,disp);
}

void RTC_show_time( unsigned char page, unsigned char col,Time time )
{
	char disp[20] = {0};
	char timearea[5] = {0};
	unsigned char timehour = current_time.hour;  //16.9.21 
	unsigned char timemin = current_time.min;	
	
	if(timemode_flag == TimeMode_24)
	{
		if (one_sec_count%2)
		{
			sprintf_P(disp,PSTR("%2d:%02d"), timehour, timemin);
		}
		else
		{
			sprintf_P(disp,PSTR("%2d %02d"), timehour, timemin);
		}
		LCD_show_str916(page,col,disp);
	}
	
	if(timemode_flag == TimeMode_12)
	{
		
		if(0 <= timehour && timehour <= 11)
		{
			LCD_show_string((page+1),(col+9),"AM");
		}
		else if(12 <= timehour && timehour <= 23)
		{
			LCD_show_string((page+1),(col+9),"PM");
			timehour = timehour - 12;
		}
		
		if (one_sec_count%2)
		{
			sprintf_P(disp,PSTR("%2d:%02d"), timehour, timemin);
		}
		else
		{
			sprintf_P(disp,PSTR("%2d %02d"), timehour, timemin);
		}
		LCD_show_str916(page,(col-1),disp);
	}
	
}


void to_asc_mon(unsigned char mon, char *strMon)
{
	switch(mon)
	{
		case 1:strcpy_P(strMon,PSTR("Jan")); break;
		case 2:strcpy_P(strMon,PSTR("Feb")); break;
		case 3:strcpy_P(strMon,PSTR("Mar")); break;
		case 4:strcpy_P(strMon,PSTR("Apr")); break;
		case 5:strcpy_P(strMon,PSTR("May")); break;
		case 6:strcpy_P(strMon,PSTR("June")); break;
		case 7:strcpy_P(strMon,PSTR("July")); break;
		case 8:strcpy_P(strMon,PSTR("Aug")); break;
		case 9:strcpy_P(strMon,PSTR("Sept")); break;
		case 10:strcpy_P(strMon,PSTR("Oct")); break;
		case 11:strcpy_P(strMon,PSTR("Nov")); break;
		case 12:strcpy_P(strMon,PSTR("Dec")); break;
		default: break;
	}
}


void to_asc_week(unsigned char wday, char *strWday)
{
	switch(wday)
	{
		case MON : strcpy_P(strWday,PSTR("Mon")); break;
		case TUES : strcpy_P(strWday,PSTR("Tues")); break;
		case WED : strcpy_P(strWday,PSTR("Wed")); break;
		case THURS : strcpy_P(strWday,PSTR("Thur")); break;
		case FRI : strcpy_P(strWday,PSTR("Fri")); break;
		case SAT : strcpy_P(strWday,PSTR("Sat")); break;
		case SUN : strcpy_P(strWday,PSTR("Sun")); break;
		default: break;
	}
}

void RTC_show_date( unsigned char page, unsigned char col,Time time )
{
	char disp[20] = {0};
	char strWday[5] = {0};
	char strMon[5] = {0};

	to_asc_week(time.wday, strWday);
	to_asc_mon(time.mon, strMon);
	
	sprintf_P(disp,PSTR("%s %s %02d 20%02d  "),strWday,strMon,time.mday,time.year);
	LCD_show_string(page,col,disp);
}


void show_manu_arrow(uchar select, char icon)
{
	select %= 4;
	
	if((0 == select) || (4 == select) || (8 == select)) LCD_show_icon(3,1,icon);
	if((1 == select) || (5 == select) || (9 == select)) LCD_show_icon(3,11,icon);
	if((2 == select) || (6 == select) || (10 == select)) LCD_show_icon(5,1,icon);
	if((3 == select) || (7 == select) || (11 == select)) LCD_show_icon(5,11,icon);
}


/************************************************************************/
/* 函数名：maun_value_show												*/
/* 功能：手动模式下显示选中的路数，被选中后反显							*/
/*参数：无																*/
/*返回值：无															*/
/************************************************************************/	
void maun_value_show(void)
{
	uchar start_zone = manu_sel/4 * 4;
	
	maun_zone_chage(start_zone);
	
	if (manu_sel != manu_sel_temp)
	{
		show_manu_arrow(manu_sel_temp,' ');
		manu_sel_temp = manu_sel;
	}
	show_manu_arrow(manu_sel,'>');
}

void AutoModePageDisplay(void)
{
	uchar col = 4;
	char disp[17] = {0};
		
	LCD_show_strP(0, 7, PSTR("Auto Run"));
	RTC_show_date(2,3,current_time);
	RTC_show_time(3,5,current_time);
	
	if (0 == devparam.tem_unit_flag)
	{
		sprintf_P(disp, PSTR("Temp: %d`-%d`  "),weather.tempminF,weather.tempmaxF);
	}
	else {
		sprintf_P(disp, PSTR("Temp: %d#-%d#  "),weather.tempminC,weather.tempmaxC);
	} 
	
	LCD_show_string(5,4,disp);
	sprintf_P(disp, PSTR("Raining: %d"), weather.raining);
	LCD_show_string(6,col,disp);
	col += strlen(disp);
	LCD_show_icon(6,col,'%');
	col++;
	LCD_show_icon(6,col,' ');
	col++;
	LCD_show_icon(6,col,' ');
}


/************************************************************************/
/* 函数名：invert_valve													*/
/* 功能：12路选中显示（上下各6路，被选中则反显）						*/
/*参数：zero_temp ：被选中路数，位表示									*/
/*返回值：无															*/
/************************************************************************/
void invert_valve( uint zero_temp ) 
{
	uchar i = 0;
	uchar page = 4;
	uchar col = 2;
	for (i = 0; i<12; i++)
	{
		if(6 == i)
		 {
			page = 5;
			col = 2;
		 }
		if (zero_temp & (1<<i))
		{
			LCD_show_numberR(page,col,i+1);
		}
		else
		{
			LCD_show_number(page,col,i+1);
		}
		col += 3;
	}
}

void show_warning(void)
{
	if(MCDwarning_flag)
	{
		LCD_show_strP(1,5,PSTR("  Warning:  "));
		LCD_show_strP(2,2,PSTR("Mobile Control Off"));
	}else
	{
		LCD_show_strP(1, 5,PSTR("(zone:min.)"));
	}
	
	if(one_sec_count >= 180) { one_sec_count = 0; MCDwarning_flag = 1;  return;}
	
	if(one_sec_count >= 6 && MCDwarning_flag == 1)
	{
		MCDwarning_flag = 0;
		LCD_show_strP(1,5,PSTR("                "));
		LCD_show_strP(2,2,PSTR("                   "));
		return;
	}
}

void SchMainDisplay(void)
{
	char str[6] = {0};
	LCD_show_strP(0, 6, PSTR("Schedule"));
	LCD_show_strP(2, 4,PSTR("Check & Setup"));
	strcpy_P(str, PSTR("Plans"));
	LCD_show_str916(4, 4, str);
	LCD_show_strP(7, 16, PSTR("ENTER"));
}

void ManuWaterPageDisplay(void)
{
	LCD_show_strP(0, 3, PSTR("Manual Watering"));
	show_warning(); // 16.9.27 单位显示被移入此函数
	maun_value_show();
	LCD_show_strP(7, 0, PSTR("RESET"));
	LCD_show_strP(7, 16, PSTR("START"));
	LCD_show_char(7,8,'-');
	LCD_show_icon(7,12,'+');
}

void show_watering(void)
{
	uchar i = 0;
	char temp[5] =  {0};
	char temp1[10] = {0};
	char temp2[10] = {0};
	char disp[15] = {0};
	char space[3] = {0};
	int min = 0;
	int sec = 0;
	SLink *p = spray_link->next;
	Spray spr;
	int version = get_version_id();
	strcat_P(space, PSTR("  "));
	
	LCD_show_strP(3,3,PSTR("Zone"));
	
	if ((1 > devparam.mwspray) || (MaxWMSpray < devparam.mwspray))
	{
		devparam.mwspray = 1;
	}
	
	if (1 == devparam.mwspray)
	{
		if (NULL != p)
		{
			spr = *(pSpray)(p->data);
			sprintf_P(disp,PSTR("%d "),spr.zone);
		
			p = p->next;
			while(NULL != p)
			{
				spr = *(pSpray)(p->data);
				strcat_P(temp1, PSTR("  "));
				sprintf_P(temp,PSTR("~%d"),spr.zone);
				strcat(temp2,temp);
				p = p->next;
				i++;
				if (i == 2)
				{
					break;
				}
			}
			LCD_show_str916(2,5,space);
			LCD_show_str916(2,7,disp);
			while(strlen(temp1) < 6)
			{
				strcat_P(temp1, PSTR("  "));
				strcat_P(temp2, PSTR("  "));
			}
			if (2 == strlen(disp))
			{
				LCD_show_string(2, 13, temp1);
				LCD_show_string(3, 13, temp2);
			}
			else
			{
				LCD_show_string(2, 14, temp1);
				LCD_show_string(3, 14, temp2);
			}
		}
	}
	else
	{
		while (NULL != p)
		{
			spr = *(pSpray)(p->data);
			sprintf_P(temp,PSTR("%d "),spr.zone);
			strcat(disp,temp);
			p = p->next;
			i++;
			if (i == devparam.mwspray)
			{
				break;
			}
		}
		if (strlen(disp) < 8)
		{
			while(strlen(disp) < 8)
			{
				strcat_P(disp, PSTR(" "));
			}
			LCD_show_str916(2,5,space);
			LCD_show_str916(2,7,disp);
		}
		else
		{
			LCD_show_str916(2,5,disp);
		}
	}
	
	if (0 != devparam.mastertag)
	{
		LCD_show_string(7,4,"M");
	}
	else 
	{
		LCD_show_string(7,4," ");
	}
	
	if ((version > 165) && (version != 0xffff))
	{
		if (sensor_value == SensorOn)
		{
			LCD_show_string(7,7,"S");
		}
		else
		{
			LCD_show_string(7,7," ");
		}
	}
	
	LCD_show_strP(5,1,PSTR("Remaining"));
	p = spray_link->next;
	if (NULL != p)
	{
		spr = *(pSpray)(p->data);
		min = spr.howlong/60;
		sec = spr.howlong%60;
		sprintf_P(disp,PSTR("%02d:%02d "), min, sec);
		LCD_show_str916(4,7,disp);
		
		if (spr.type == SparyBySch)
		{
			if (devparam.program_id >= PROMGRAM_A && devparam.program_id <= PROMGRAM_D)
			{
				char program = 'A'+ devparam.program_id - 1;
				LCD_show_char(7,16,program);
			}
			else
			{
				LCD_show_string(7,16," ");
			}
		}
		else
		{
			LCD_show_string(7,16," ");
		}
	}
}

void AutoStartPageDisplay(void)
{
	LCD_show_strP(0, 4, PSTR("WATERING AUTO"));
	show_watering();
}


void ManuWaterStartDisplay(void)
{
	LCD_show_strP(0, 3, PSTR("Manual Watering"));
	show_watering();
	LCD_show_strP(7,17,PSTR("STOP"))	;
}

void show_success(void)
{
	if (show_success_flag)
	{
		LCD_show_strP(6,2,PSTR("Saved successfully"));
		show_success_flag = 0;
		showed_success_flag = 1;
	}
	else
	{
		if (showed_success_flag &&(half_sec_count > 4 ))
		{
			LCD_show_strP(6,2,PSTR("                  "));
			showed_success_flag = 0;
		}
	}
}

void show_raindelay(int delay_time)
{
	char disp[10] = {0};
		
	if (delay_time < 60)
	{
		sprintf_P(disp,PSTR("%-03d"), delay_time);
		LCD_show_str916(3, 4, disp);
		LCD_show_strP(4, 11, PSTR("Min. "));
	}
	else
	{
		delay_time = (delay_time-1)/60 + 1;
		sprintf_P(disp,PSTR("%-03d"), delay_time);
		LCD_show_str916(3, 4, disp);
		LCD_show_strP(4, 11, PSTR("Hours"));
	}
}

void RainDelayDisplay(void)
{
	LCD_show_strP(0, 3, PSTR("Watering Delay"));
	
	if (setting_delaytime_flag)
	{
		show_raindelay(rain_delay_temp);
	} 
	else
	{
		show_raindelay(rain_delay.delay_time);
	}
	show_success();
	LCD_show_strP(7, 0, PSTR("RESET"));
	LCD_show_strP(7, 17, PSTR("SAVE"));
	LCD_show_char(7, 8, '-');
	LCD_show_icon(7, 12, '+');
}

void show_conf_sel(uchar select, char icon)
{
	if(0 == select) LCD_show_icon(2,0,icon);
	if(1 == select) LCD_show_icon(3,0,icon);
	if(2 == select) LCD_show_icon(4,0,icon);
	if(3 == select) LCD_show_icon(5,0,icon);
}

void SettingDisplay(void)
{
	LCD_show_strP(0, 7, PSTR("Setting"));
	LCD_show_strP(2,2,PSTR("Date & Clock"));
	LCD_show_strP(3,2,PSTR("WiFi Access"));
	LCD_show_strP(4,2,PSTR("Register Controller"));
	LCD_show_strP(5,2,PSTR("Master Setting"));
	
	show_conf_sel(config_select,'>');
	if (config_select != config_select_temp)
	{
		show_conf_sel(config_select_temp,' ');
		config_select_temp = config_select;
	}
	
	LCD_show_strP(7,0,PSTR("NEXT"));
	LCD_show_strP(7,16,PSTR("SETUP"));
}


void OffModeDisplay(void)
{
	LCD_show_strP(1, 5, PSTR("All Watering"));
	LCD_show_str916(2, 6, "OFF");
	RTC_show_date(4,3,current_time);
	RTC_show_time(5,5,current_time);
}

void show_adjust(uchar select, uchar *adjust, uchar *days)
{
	uchar i = 0;
	uchar page = 3;
	char disp[10] = {0};
	
	if (select < 3)
	{
		for (i = 0; i < 3; i++)
		{
			if( 0 == i)
			{
				strcpy_P(disp, PSTR("Today"));
			}
			else
			{
				to_asc_week(days[i], disp);
			}
			strcat_P(disp, PSTR(":"));
			sprintf_P(disp, PSTR("%-6s"), disp);
			LCD_show_string(page, 4, disp);
			
			sprintf_P(disp, PSTR("%3d"), adjust[i]);
			LCD_show_string(page, 11, disp);
			
			LCD_show_icon(page, 14,'%');
			page++;
		}
	}
	else
	{
		for (i = 3; i < 5; i++)
		{
			to_asc_week(days[i], disp);
			strcat_P(disp, PSTR(":"));
			sprintf_P(disp, PSTR("%-6s"), disp);
			LCD_show_string(page, 4, disp);
			
			sprintf_P(disp, PSTR("%3d"), adjust[i]);
			LCD_show_string(page, 11, disp);
			
			LCD_show_icon(page, 14,'%');
			page++;
		}
		LCD_show_strP(page, 4, PSTR("             "));
	}
}

void show_adjust_sel(uchar select, char icon)
{
	if((0 == select) || (3 == select)) LCD_show_icon(3,2,icon);
	if((1 == select) || (4 == select)) LCD_show_icon(4,2,icon);
	if(2 == select) LCD_show_icon(5,2,icon);
}


void WeatherAdjustDisplay(void)
{
	LCD_show_strP(0, 1, PSTR("Watering Adjustment"));
	
	show_adjust(adjust_sel, adjust_value_temp, adjust_days);
	
	show_adjust_sel(adjust_sel, '>');
	if (adjust_sel != adjust_sel_temp)
	{
		show_adjust_sel(adjust_sel_temp,' ');
		adjust_sel_temp = adjust_sel;
	}
	
	show_success();
	LCD_show_char(7,8,'-');
	LCD_show_icon(7,12,'+');
	LCD_show_strP(7, 17, PSTR("SAVE"));
	LCD_show_strP(7, 0, PSTR("NEXT"));
}


void setup_date_show(pTime tm, uchar option)
{
	char strWday[5] = {0};
	char strMon[5] = {0};
	char disp[16] = {0};
	uchar col = 0;
	
	to_asc_week(tm->wday, strWday);
	to_asc_mon(tm->mon, strMon);

	switch(option)
	{
		case SetWday:
		{
			LCD_show_stringR(2,3,strWday);
			LCD_show_strP(2,3+strlen(strWday),PSTR("|"));
			sprintf_P(disp,PSTR("%s %02d 20%02d "),strMon,tm->mday,tm->year);
			LCD_show_string(2,(3+strlen(strWday) +1),disp);
			break;
		}
		
		case SetMday:
		{
			sprintf_P(disp, PSTR("%s %s "), strWday, strMon);
			LCD_show_string(2,3,disp);
			col =3 + strlen(disp);
			sprintf_P(disp,PSTR("%02d"),tm->mday);
			LCD_show_stringR(2,col,disp);
			col += 2;
			LCD_show_strP(2,col,PSTR("|"));
			col++;
			sprintf_P(disp,PSTR("20%02d "),tm->year);
			LCD_show_string(2,col,disp);
			break;
		}
		
		case SetMon:
		{
			sprintf_P(disp,PSTR("%s "),strWday);
			LCD_show_string(2,3,disp);
			col = 3 + strlen(disp);
			LCD_show_stringR(2,col, strMon);
			col += strlen(strMon);
			LCD_show_strP(2,col,PSTR("|"));
			col++;
			sprintf_P(disp,PSTR("%02d 20%02d "),tm->mday,tm->year);
			LCD_show_string(2,col,disp);
			break;
		}
		
		case SetYear:
		{
			sprintf_P(disp,PSTR("%s %s %02d "),strWday,strMon,tm->mday);
			LCD_show_string(2,3,disp);
			col = 3 + strlen(disp);
			sprintf_P(disp,PSTR("20%02d"),tm->year);
			LCD_show_stringR(2,col,disp);
			break;
		}
		default:
			RTC_show_date(2,3,*tm);
			break;
	}
}

void setup_time_show(uchar page, uchar col, pTime tm, uchar option)
{
	char disp[10];
	uchar hourtemp = tm->hour;
	uchar mintemp = tm->min;
	
	if(timemode_setup == TimeMode_12)
	{
		 if(12 <= hourtemp && hourtemp <= 23)
		{
			hourtemp = hourtemp - 12;
		}
	}
	
	switch(option)
	{
		case SetHour:
			sprintf_P(disp, PSTR("%02d"),hourtemp);
			LCD_show_stringR(page, col, disp);
			col += strlen(disp);
// 			LCD_show_strP(page,col,PSTR("|"));
// 			col++;
			sprintf_P(disp,PSTR(":%02d"), mintemp);
			LCD_show_string(page, col, disp);
			col += (strlen(disp));		
			
			if(timemode_setup == TimeMode_12)
			{
				if(0 <= tm->hour && tm->hour <= 11)
				{
					sprintf_P(disp, PSTR(" AM"));
				}
				else if(12 <= tm->hour && tm->hour <= 23)
				{
					sprintf_P(disp, PSTR(" PM"));
				}
				LCD_show_string(page,col,disp);
			}else{LCD_show_string(page,col,"   ");}
				
			sprintf_P(disp,PSTR("%dHours"), timemode_setup);
			LCD_show_string(page,12, disp);
			LCD_show_string(page,19," ");			
			break;
			
		case SetMin:
			
			sprintf_P(disp, PSTR("%02d:"),hourtemp);
			LCD_show_string(page, col, disp);
			col += strlen(disp);
			sprintf_P(disp,PSTR("%02d"), mintemp);
			LCD_show_stringR(page, col, disp);
			col += (strlen(disp));
// 			LCD_show_strP(page,col,PSTR("|"));
// 			col++;
			
			if(timemode_setup == TimeMode_12)
			{
				if(0 <= tm->hour && tm->hour <= 11)
				{
					sprintf_P(disp, PSTR(" AM"));
				}
				else if(12 <= tm->hour && tm->hour <= 23)
				{
					sprintf_P(disp, PSTR(" PM"));
				}
				LCD_show_string(page,col,disp);
			}else{LCD_show_string(page,col,"   ");}
			
			sprintf_P(disp,PSTR("%dHours"), timemode_setup);
			LCD_show_string(page, 12, disp);
			LCD_show_string(page,19," ");
			break;
		
		case SetTimeMode:
			sprintf_P(disp, PSTR("%02d:%02d"),hourtemp, mintemp);
			LCD_show_string(page, col, disp);
			col += (strlen(disp));
			
			if(timemode_setup == TimeMode_12)
			{
				if(0 <= tm->hour && tm->hour <= 11)
				{
					sprintf_P(disp, PSTR(" AM"));
				}
				else if(12 <= tm->hour && tm->hour <= 23)
				{
					sprintf_P(disp, PSTR(" PM"));
				}
				LCD_show_string(page,col,disp);
			}else{LCD_show_string(page,col,"   ");col += 3;}
					
			sprintf_P(disp,PSTR("%dHours"), timemode_setup);
			LCD_show_stringR(page, 12, disp);
			LCD_show_string(page,19," ");

			break;			
			
		default: 
			sprintf_P(disp, PSTR("%02d:%02d"),hourtemp, mintemp);
			LCD_show_string(page, col, disp);
			col += (strlen(disp));
			
			if(timemode_setup == TimeMode_12)
			{
				if(0 <= tm->hour && tm->hour <= 11)
				{
					sprintf_P(disp, PSTR(" AM"));
				}
				else if(12 <= tm->hour && tm->hour <= 23)
				{
					sprintf_P(disp, PSTR(" PM"));
				}
				LCD_show_string(page,col,disp);
			}else{LCD_show_string(page,col,"   ");}
				
			sprintf_P(disp,PSTR("%dHours"), timemode_setup);
			LCD_show_string(page, 12, disp);
			LCD_show_string(page,29," ");
			break;
	}
}

void DataTimeSetDisplay( void )
{
	LCD_show_strP(0, 0, PSTR("Setup Date and Clock"));
	setup_date_show(&tm_setup, time_option);
	setup_time_show(4,3, &tm_setup, time_option);
	LCD_show_strP(7,0,PSTR("NEXT"));
	LCD_show_char(7,8,'-');
	LCD_show_icon(7,12,'+');
	LCD_show_strP(7,19,PSTR("OK"));
}

void WifiAccessDisplay(void)
{
	LCD_show_strP(0, 2, PSTR("Setup WiFi Access"));
	LCD_show_strP(3,1, PSTR("Pls download App,"));
	LCD_show_strP(5,1, PSTR("and run WiFi Access"));

	LCD_show_strP(7,17,PSTR("BACK"));
}

void RegSmartDevDisplay(void)
{
	char disp[10] = {0};
		
	LCD_show_strP(0, 1, PSTR("Register Controller"));
	if (DisBanded == user_band_flag)
	{
		LCD_show_strP(1,0,PSTR(" S/N:      "));
		LCD_show_string(2,5,devcode);
		LCD_show_strP(3,0,PSTR(" Password:        "));
		LCD_show_strP(4,0,PSTR("     "));
		LCD_show_string(4,5,serialcode);
		LCD_show_strP(4,11,PSTR("   "));
		LCD_show_strP(5, 0, PSTR(" Pls Use APP To "));
		LCD_show_strP(6, 0, PSTR(" Register Device"));
		LCD_show_strP(7,17,PSTR("BACK"));
	}
	else
	{
		strcpy_P(disp, PSTR("Success"));
		LCD_show_str916(3,4,disp);
		LCD_show_strP(7,17,PSTR("BACK"));
	}
}

/*
*@page_valves:每页面显示路数个数，分别为4,8,12个路数
*/
void show_valve(uint16_t page_valves, uint8_t start_valve)
{
	uint16_t i = 0;
	uint8_t col = 0;
	uint8_t page = 1;
	int8_t disp[10] = {0};
	
	if (!(4 == page_valves || 8 == page_valves || 12 == page_valves))
	{
		return;
	}
	
	for(i=0; i < page_valves; i++)        //16.7.7 statement with no effect
	{
		if ((start_valve+i) > (start_valve + page_valves))
		{
			sprintf_P((char *)disp, PSTR("   "));  //16.7.7 sprintf_P(disp, PSTR("   "))
		}
		else
		{
			sprintf_P((char *)disp, PSTR("%d"), start_valve+i); //16.7.7 sprintf_P(disp, PSTR("%d"), start_valve+i)
		}
		
		if (12 == page_valves)
		{
			if (0 == i%3)
			{
				col = 4;
				page++;
			}
			else if (1 == i%3)
			{
				col = 10;
			}
			else
			{
				col = 16;
			}
		}
		else if (8 == page_valves)
		{
			if (0 == i%2)
			{
				col = 5;
				page++;
			}
			else
			{
				col = 15;
			}
		}
		else if (4 == page_valves)
		{
			if (0 == i%2)
			{
				col = 5;
				page += 2;
			}
			else
			{
				col = 15;
			}
		}
		
		if (locate_zone(sch_link, start_valve+i))
		{
			LCD_show_stringR(page, col, (const char *)disp); //16.7.7 pointer targets in passing argument3 differ in signedness
			col += strlen((const char *)disp); //16.7.7 strlen(disp)
			LCD_show_strP(page, col, PSTR("|"));
			col++;
			LCD_show_strP(page,col,PSTR(" "));
		}
		else
		{
			LCD_show_string(page,col,(const char *)disp);
			col += strlen((const char *)disp);   //16.7.7 strlen(disp)
			LCD_show_strP(page,col,PSTR("  "));
		}
	}
}

void show_arrow(uint16_t page_num, uint8_t whichvalve, int8_t icon)
{
//	int16_t i = 0;    //16.7.7 unused
	uint8_t col = 0;
	uint8_t page = 1;
//	int8_t disp[10] = {0}; // 16.7.7 unused
	
	uint8_t select = whichvalve % page_num;
	
	if (0 == select)
	{
		select += page_num;
	}
	
	if (!(4 == page_num || 8 == page_num || 12 == page_num))
	{
		return;
	}
	
	if (4 == page_num)
	{
		page = (uint8_t)((select + 1)/2 *2 + 1);
		if (1 == select%2)
		{
			col = 3;
		}
		else
		{
			col = 13;
		}
	}
	else if (8 == page_num)
	{
		page = (uint8_t)((select + 1)/2 + 1);
		if (1 == select%2)
		{
			col = 3;
		}
		else
		{
			col = 13;
		}
	}
	else if (12 == page_num)
	{
		page = (uint8_t)((select + 2)/3 + 1);
		if (1 == select%3)
		{
			col = 2;
		}
		else if(2 == select%3)
		{
			col = 8;
		}
		else if(0 == select%3)
		{
			col = 14;
		}
	}
	LCD_show_icon(page,col,icon);
}


void SchSelValveDisplay(void)
{
	uchar start_valve;
	int page_num = valves/12 * 4;
	
	if (page_num > 12) //每页最多显示12路
	{
		page_num = 12;
	}
	
	start_valve = (plan_sel - 1)/page_num * page_num + 1;
	
	show_valve(page_num, start_valve);
	
	show_arrow(page_num, plan_sel,'>');
	if (plan_sel != plan_sel_temp)
	{
		show_arrow(page_num, plan_sel_temp,' ');
		plan_sel_temp = plan_sel;
	}
	
	LCD_show_strP(0, 4, PSTR("Schedule Zone"));
	LCD_show_strP(7, 0, PSTR("DEL"));
	
	if(get_weather_flag == GetWeatherOk) //16.4.1 添加 暂定获取天气完毕后更新喷淋
	LCD_show_strP(7, 5, PSTR("SYNC"));
	
	if(copy_fun_flag)
	{
		LCD_show_strP(7,11,PSTR("PASTE"));
	}
	else
	{
		LCD_show_strP(7,11,PSTR("COPY "));
	}
	LCD_show_strP(7,17,PSTR("EDIT"));
}

void timer_show(uchar page,uint* tm, uchar option)
{
	char disp[15];
	uchar col = 4;
	
	if(timemode_flag == TimeMode_12)
	{
		col = 3;
	}else if(timemode_flag == TimeMode_24) 
	{ 
		col = 4;
	}
	
	switch(option)
	{
		case 0:
			sprintf_P(disp, PSTR("%02d"),tm[0]);
			LCD_show_stringR(page, col, disp);
			col += strlen(disp);
			sprintf_P(disp,PSTR(":%02d"), tm[1]);
			LCD_show_string(page, col, disp);
			sprintf_P(disp,PSTR("%-3d"),tm[2]);
			LCD_show_string(page, 12, disp);
			break;
		
		case 1:
			sprintf_P(disp, PSTR("%02d:"),tm[0]);
			LCD_show_string(page, col, disp);
			col += strlen(disp);
			sprintf_P(disp,PSTR("%02d"), tm[1]);
			LCD_show_stringR(page, col, disp);
			sprintf_P(disp,PSTR("%-3d"),tm[2]);
			LCD_show_string(page, 12, disp);
			break;
		
		case 2:
			sprintf_P(disp, PSTR("%02d:%02d"),tm[0],tm[1]);
			LCD_show_string(page, col, disp);
			col += strlen(disp);
			sprintf_P(disp,PSTR("%-3d"),tm[2]);
			LCD_show_stringR(page, 12, disp);
//			col +=strlen(disp);
// 			LCD_show_strP(page,col,PSTR("|"));
// 			sprintf_P(disp,PSTR("%d"),tm[2]);
// 			if (strlen(disp) < 3)
// 			{
// 				col++;
// 				LCD_show_strP(page,col,PSTR(" "));
// 			}
			break;

		default:	break;
	}
}

void SchSetTimeDisplay(void)
{
	char disp[22] = {0};
	uint tmr[12] = {0};
	uint tmhour_temp[3] = {0};
	uchar i = 0;
	uchar j = 0;
	uchar k = 0;
	uchar z = 0;
	uchar page = 2;
	uchar col = 4;
	
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
	
	LCD_show_strP(0, 5, PSTR("Set Timer"));
	
	if(timemode_flag == TimeMode_12)
	{
		col = 3;
		for(z = 0;z <= 3;z++)
		{
			if(12 <= tmr[3*z] && tmr[3*z] <= 23) { tmr[3*z] = tmr[3*z] - 12; LCD_show_string((z+2),(col + 5)," PM");}
			else if(0 <= tmr[3*z] && tmr[3*z] <= 11){ LCD_show_string((z+2),(col + 5)," AM");}
		}
	}else if(timemode_flag == TimeMode_24)
	{
		col = 4;
		for(z = 0;z <= 3;z++)
		{
			LCD_show_string((z+2),(col+5),"  ");
		}	
	}
	
	if (set_timer_option < 3)
	{
		k = set_timer_option;
		
		timer_show(2,&tmr[0],k);
		
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[3],tmr[4]);
		LCD_show_string(3,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[5]);
		LCD_show_string(3,12,disp);
		
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[6],tmr[7]);
		LCD_show_string(4,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[8]);
		LCD_show_string(4,12,disp);
		
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[9],tmr[10]);
		LCD_show_string(5,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[11]);
		LCD_show_string(5,12,disp);
	}
	else if (set_timer_option < 6)
	{
		k = set_timer_option - 3;
		timer_show(3,&tmr[3],k);
		
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[0],tmr[1]);
		LCD_show_string(2,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[2]);
		LCD_show_string(2,12,disp);
			
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[6],tmr[7]);
		LCD_show_string(4,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[8]);
		LCD_show_string(4,12,disp);
			
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[9],tmr[10]);
		LCD_show_string(5,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[11]);
		LCD_show_string(5,12,disp);
	}
	else if (set_timer_option < 9)
	{
		k = set_timer_option - 6;
		timer_show(4,&tmr[6],k);
		
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[0],tmr[1]);
		LCD_show_string(2,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[2]);
		LCD_show_string(2,12,disp);
			
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[3],tmr[4]);
		LCD_show_string(3,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[5]);
		LCD_show_string(3,12,disp);
			
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[9],tmr[10]);
		LCD_show_string(5,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[11]);
		LCD_show_string(5,12,disp);
			
	}
	else if (set_timer_option < 12)
	{
		k = set_timer_option - 9;
		timer_show(5,&tmr[9],k);
		
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[0],tmr[1]);
		LCD_show_string(2,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[2]);
		LCD_show_string(2,12,disp);
			
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[3],tmr[4]);
		LCD_show_string(3,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[5]);
		LCD_show_string(3,12,disp);
			
		sprintf_P(disp,PSTR("%02d:%02d"),tmr[6],tmr[7]);
		LCD_show_string(4,col,disp);
		sprintf_P(disp,PSTR("%-3d "),tmr[8]);
		LCD_show_string(4,12,disp);
	}

	
	LCD_show_strP(2, 0, PSTR("a."));
	LCD_show_strP(3, 0, PSTR("b."));
	LCD_show_strP(4, 0, PSTR("c."));
	LCD_show_strP(5, 0, PSTR("d."));
	
	page = 2;
	for(i = 0, j = 2; i < 4; i++)
	{
		if (tmr[j] > 1)
		{
			LCD_show_strP(page, 16,PSTR("mins"));
		} 
		else
		{
			LCD_show_strP(page, 16,PSTR("min "));
		}
		j += 3;
		page++;
	}
	
	LCD_show_strP(7,0,PSTR("NEXT"));
	LCD_show_strP(7,19,PSTR("OK"))	;
	LCD_show_char(7,8,'-');
	LCD_show_icon(7,12,'+');
}

void SchSetWeeksDisplay(void)
{
	uchar i = 0;
	uchar col = 3;
	uchar page = 2;
	
	char strWeek[6] = {0};
	
	for (i = 0; i < 7; i++)
	{
		to_asc_week(i+1,strWeek);
		if (new_sch.weeks & (1<<i))
		{
			LCD_show_stringR(page, col, strWeek);
			col =col + strlen(strWeek)+ 2;
		}
		else
		{
			LCD_show_string(page, col, strWeek);
			col += strlen(strWeek);
			LCD_show_strP(page, col, PSTR(" "));
			col += 2;
		}
		
		if (2 == i)
		{
			page = 4;
			col = 1;
		}
	}
	
	page = 3;
	col = 4;
	for (i = 0; i < 7; i++)
	{
		to_asc_week(i+1,strWeek);
		if (weekday_sel == i)
		{
			LCD_show_icon(page, col, '^');
		}
		else
		{
			LCD_show_icon(page, col, ' ');
		}
		col = col + strlen(strWeek) + 2;
		if (2 == i)
		{
			page = 5;
			col = 2;
		}
	}
	
	LCD_show_strP(0, 5, PSTR("Set Weekday"));
	LCD_show_strP(7, 0, PSTR("NEXT"));
	LCD_show_char(7,8,'-');
	LCD_show_icon(7,12,'+');
	LCD_show_strP(7, 19, PSTR("OK"));
}



void SchSelModeDisplay(void)
{
	char temp[25];
	
	LCD_show_strP(0, 4, PSTR("Set Calendar"));
	
	strcpy_P(temp, PSTR("By Week"));
	
	if (new_sch.weeks && (new_sch.mode == BY_WEEKS))
	{
		LCD_show_stringR(3,3,temp);
	}
	else
	{
		LCD_show_string(3,3,temp);
	}
	
	strcpy_P(temp, PSTR("By Month"));
	
	if (new_sch.days && (new_sch.mode == BY_DAYS))
	{
		LCD_show_stringR(5,3,temp);
	}
	else
	{
		LCD_show_string(5,3,temp);
	}
	
	LCD_show_strP(7, 0, PSTR("NEXT"));
	LCD_show_strP(7, 19, PSTR("OK"));
	show_conf_sel(sch_mod_sel,'>');
	if (sch_mod_sel != sch_mod_sel_temp)
	{
		show_conf_sel(sch_mod_sel_temp,' ');
		sch_mod_sel_temp = sch_mod_sel;
	}
}


void SchSetDaysDisplay(void)
{
	int i;
	uchar col = 1;
	uchar page = 2;
	char strDays[6] = {0};
	
	if (((days_sel_temp == 16) && (days_sel == 17)) || ((days_sel_temp == 30) && (days_sel == 0)))
	{
		LCD_show_strP(2,0,PSTR("                     "));
		LCD_show_strP(4,0,PSTR("                     "));
	}
	days_sel_temp = days_sel;
		
	if (days_sel < 17)
	{
		for(i=0; i < 17; i++ )
		{
			sprintf_P(strDays, PSTR("%d"), i+1);
			
			if ((new_sch.days >> i) & 1)
			{
				LCD_show_stringR(page, col, strDays);
				col =col + strlen(strDays)+ 1;
			}
			else
			{
				LCD_show_string(page, col, strDays);
				col += strlen(strDays);
				LCD_show_strP(page, col, PSTR(" "));
				col += 1;
			}
			if (9 == i)
			{
				page = 4;
				col = 1;
			}
		}
		
	}
	else if (days_sel < 31)
	{
		for(i=17; i < 31; i++ )
		{
			sprintf_P(strDays, PSTR("%d"), i+1);
			
			if ((new_sch.days >> i) & 1)
			{
				LCD_show_stringR(page, col, strDays);
				col =col + strlen(strDays)+ 1;
			}
			else
			{
				LCD_show_string(page, col, strDays);
				col += strlen(strDays);
				LCD_show_strP(page, col, PSTR(" "));
				col += 1;
			}
			if (23 == i)
			{
				page = 4;
				col = 1;
			}
		}
	}
	
	
	page = 3;
	col = 1;
	
	if (days_sel < 17)
	{
		for (i = 0; i < 17; i++)
		{
			sprintf_P(strDays, PSTR("%d"), i+1);
			if (days_sel == i)
			{
				LCD_show_icon(page, col, '^');
			}
			else
			{
				LCD_show_icon(page, col, ' ');
			}
			col = col + strlen(strDays) + 1;
			if (9 == i)
			{
				page = 5;
				col = 1;
			}
		}
	}
	else{
		for (i = 17; i < 31; i++)
		{
			sprintf_P(strDays, PSTR("%d"), i+1);
			if (days_sel == i)
			{
				LCD_show_icon(page, col, '^');
			}
			else
			{
				LCD_show_icon(page, col, ' ');
			}
			col = col + strlen(strDays) + 1;
			if (23 == i)
			{
				page = 5;
				col = 1;
			}
		}
	}
	
	
	
	LCD_show_strP(0, 5, PSTR("Set Days"));
	LCD_show_strP(7, 0, PSTR("NEXT"));
	LCD_show_char(7,8,'-');
	LCD_show_icon(7,12,'+');
	LCD_show_strP(7, 19, PSTR("OK"));
}

void SpecialFeatDisplay(void)
{	
	char tem[12] = {0};
	char res[7] = {0};
	LCD_show_strP(0, 3, PSTR("Controller Info."));
	LCD_show_strP(3,4,PSTR("www.nxeco.com"));
	LCD_show_strP(5,3,PSTR("S/N:"));
	LCD_show_string(5,7,devcode);
	get_vesion_str(res);
	sprintf_P(tem,PSTR("Ver:%s"), res);
	LCD_show_string(7,12,tem);
}

void  SchDelDisplay(void)
{
	LCD_show_strP(3, 2, PSTR("Delete this plan?"));
	LCD_show_strP(7, 0, PSTR("CANCEL"));
	LCD_show_strP(7, 18, PSTR("DEL"));
}

void SchPlsSetTimeDisplay(void)
{
	LCD_show_strP(3, 3, PSTR("BACK Set timer"));
	LCD_show_strP(4, 3, PSTR("or CANCEL "));
	LCD_show_strP(7, 0, PSTR("CANCEL"));
	LCD_show_strP(7, 17, PSTR("BACK"));
}

void SchPlsSetWeekDisplay(void)
{	
	if (BY_WEEKS == new_sch.mode)
	{
		LCD_show_strP(3, 3, PSTR("BACK Set Weekday"));
	}
	else if (BY_DAYS == new_sch.mode)
	{
		LCD_show_strP(3, 3, PSTR("BACK Set Days"));
	}
	
	LCD_show_strP(4, 3, PSTR("or CANCEL"));
	LCD_show_strP(7, 0, PSTR("CANCEL"));
	LCD_show_strP(7, 17, PSTR("BACK"));
}

void SchNoPlanDisplay(void)
{
	LCD_show_strP(3, 2, PSTR("There is no plan!"));
	LCD_show_strP(7, 17, PSTR("BACK"));
}

void MasterConfigDisplay( void ) 
{
	LCD_show_strP(3, 4, PSTR("Set 12th zone as"));
	LCD_show_strP(4, 2, PSTR("master valve ?"));
	LCD_show_strP(7, 0, PSTR("CANCEL"));
	LCD_show_strP(7, 19, PSTR("OK"));
}

void SchConfirmCoverDisplay(void)
{
	LCD_show_strP(3,3,PSTR("Cover this plan?"));
	LCD_show_strP(7, 0, PSTR("CANCEL"));
	LCD_show_strP(7, 16, PSTR("COVER"));
}

void SchSprayLimitDisplay()
{
	char tem[30];
	sprintf_P(tem, PSTR("Max Time:%d min."),spray_time_limit);
	LCD_show_string(3, 2, tem);
	
	LCD_show_strP(7, 0, PSTR("CANCEL"));
	LCD_show_strP(7, 17, PSTR("BACK"));
}


/************************************************************************/
/* 函数名：UI_Display													*/
/* 功能：LCD显示控制													*/
/*参数：	无															*/
/*返回值：无															*/
/************************************************************************/
void UI_Display( void )
{
	if(UpdataDisplayOk == updata_display_flag)
	{
		return;
	}
	else
	{
		updata_display_flag = UpdataDisplayOk;
	}
	
	if (CurrentPage != NextPage)
	{
		CurrentPage = NextPage;
		LCD_clear();
	}
	
	switch(CurrentPage)
	{
		case AutoModePage:
			AutoModePageDisplay();
			break;
		case WaterAutoPage:
			AutoStartPageDisplay();
			break;
		case ScheduleMainPage:
			SchMainDisplay();
			break;
		case ManuWaterPage:
			ManuWaterPageDisplay();
			break;
		case ManuWaterStartPage:
			ManuWaterStartDisplay();
			break;
		case RainDelayPage:
			RainDelayDisplay();
			break;
		case ConfigPage:
			SettingDisplay();
			break;
		case OffModePage:
			OffModeDisplay();
			break;
		case FineTunePage:
			WeatherAdjustDisplay();
			break;
		case SetupDateTimePage:
			DataTimeSetDisplay();
			break;
		case WifiAccessPage:
			WifiAccessDisplay();
			break;
		case RegSmartDevPage:
			RegSmartDevDisplay();
			break;
		case SchSelValvePage:
			SchSelValveDisplay();
			break;
		case SchSetTimePage:
			SchSetTimeDisplay();
			break;
		case SchSetWeeksPage:
			SchSetWeeksDisplay();
			break;
		case SpecialFeatPage:
			SpecialFeatDisplay();
			break;
		case SchDelPage:
			 SchDelDisplay();
			 break;
		case SchPlsSetTimePage:
			SchPlsSetTimeDisplay();
			break;
		case  SchPlsSetWeekPage:
			SchPlsSetWeekDisplay();
			break;
		case SchSetDaysPage:
			SchSetDaysDisplay();
			break;
		case SchNoPlanPage:
			SchNoPlanDisplay();
			break;
		case SchConfirmCoverPage:
			SchConfirmCoverDisplay();
			break;
		case MasterConfigPage:
			MasterConfigDisplay();
			break;
		case SchSelModePage:
			SchSelModeDisplay();
			break;
		case SchSprayLimitPage:
			SchSprayLimitDisplay();
			break;
		case ManuWaterLimitPage:
			SchSprayLimitDisplay();
			break;
							
		default:
			break;
	}
}