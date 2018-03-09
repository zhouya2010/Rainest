/*
 * json.c
 *
 * Created: 2014/6/4 10:18:16
 *  Author: ZY
 */ 
#include "mqtt-client.h" //16.7.7
#include "btn_ctr.h"
#include "data_rw.h"
#include "CMD_Ctr.h"
#include "HC595N.h"
#include "json.h"
#include "timer.h"
#include "wifi_receive.h"
#include "wifi.h"
#include "LCD.h"
#include "RTC.h"
#include "event_handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "cJSON.h"

Weather weather;
   
pSchedule sch_upload = NULL;
uchar delete_zone; 
static uchar get_sch_id = 1;

extern uchar CurrentPage;//当前显示页面ID
extern uchar NextPage;//下个显示页面ID
extern uchar adjust_mode_temp;	//位表示某一天
extern SLink * water_used_link;

extern SLink *spray_link;

static char cid[LenOfCid] = "0";
static char cid_temp[LenOfCid] = "0";


/************************************************************************/
/* 函数名：tobit*/
/* 功能：将喷淋路数由数组表示转换为16位位表示，如第二路开，则返回0x0002*/
/*参数：	路数数组*/
/*返回值：16位位表示的喷淋开启路数*/
/************************************************************************/
unsigned int tobit( char * const whichvalve)
{
	int i = 0;
	unsigned int channel = 0;
	for (i=0; i<MaxValve; i++)
	{
		if(whichvalve[i] == 0)
		{
			break;
		}
		
		if(whichvalve[i] > MaxValve)
		{
			channel = 0;
			break;
		}
		
		channel |=(1<<(whichvalve[i]-1));
	}
	return channel;	
}

//最多支持两位小数
int sscanf_f(const char * buf, float* dest)
{
	int h = 0;
	int l = 0;
	
	int res = 0;
	
	res = sscanf_P(buf, PSTR("%d.%d"), &h, &l);
	
	float f_h = (float)h;
	float f_l = (float)l;
	
	if (res < 0)
	{
		return res;
	}
	
	if (l >= 10)
	{
		*dest = f_h + f_l/100;
	}
	else
	{
		*dest = f_h + f_l/10;
	}
	
	return 1;
}

/************************************************************************/
/* 函数名：Js_parser_value*/
/* 功能：获取JSON指令中的整数*/
/*参数：	res:获取的整数存放地址	buffer：JSON指令字符串		key：获取参数名称*/
/*返回值：1成功 0失败*/
/************************************************************************/
int Js_parser_value_int( int *res,const char *buffer, char *key )
{
	unsigned char i = 0;
	char data[15] = {0};
	char *p = NULL;

	p = strstr(buffer,key);
	if(NULL != p)
	{
		p =p + strlen(key);
		if('\"' == *p) p++;
		if(':' == *p)
		{
			p++;
			if('\"' == *p) p++;
//			while(!((',' == *p) || ('}' == *p) || ('\"' == *p) || (';' == *p)))
			while((*p >= '0') && (*p <= '9'))
			{
				data[i] = *p;
				p++;
				i++;
				if (i>10)
				{
					return 0;
				}
			}
			sscanf_P(data, PSTR("%d"), res);
			return 1;
		}
		else return 0;
	}
	else return 0;	
}

/************************************************************************/
/* 函数名：Js_parser_string*/
/* 功能：获取JSON指令中的字符串*/
/*参数：	res:获取字符串存放地址	buffer：JSON指令字符串		key：获取参数名称*/
/*返回值：1成功 0失败*/
/************************************************************************/
int Js_parser_string( char *res,const char *buffer, char *key )
{
	char i = 0;
	char *p = NULL;
	char *pv = res;
		
	p = strstr(buffer,key);
	if(NULL != p)
	{
		p =p + strlen(key);
		if('\"' == *p) p++;
		if(':' == *p)
		{
			p++;
			if('\"' == *p) p++;
			while(!(('\"' == *p) || (';' == *p) || ('}' == *p) || ('\0' == *p)))
			{
				*pv = *p;
				p++;
				pv++;
				i++;
			}
			*pv = '\0';
			return 1;
		}
		else return 0;
	}
	else return 0;
}

/************************************************************************/
/* 函数名：Js_parser_array*/
/* 功能：获取JSON指令中的数组*/
/*参数：	res:获取数组存放地址	buffer：JSON指令字符串		key：获取参数名称*/
/*返回值：1成功 0失败*/
/************************************************************************/
int Js_parser_array( char *res, const char *buffer, char *key)
{
	unsigned char i = 0;
	unsigned char value[5] = {0};
	char *p = NULL;
	char *pv = res;
	
	p = strstr(buffer,key);
	if(NULL != p)
	{
		p =p + strlen(key);
		if('\"' == *p) p++;
		if(':' == *p)
		{
			p++;
			if('\"' == *p)	p++;
			while(!(('\"' == *p) || (';' == *p) || ('\0' == *p) || ('&' == *p))) // 16.7.4 wjy添加 ('&' == *p)
			{
				if((',' == *p) || (':' == *p) || ('-' == *p))
				{
					*pv = (unsigned char)atoi((const char *)value); //16.7.7
					pv++;
					p++;
					i = 0;
					memset(value,0,3);
					continue;
				}
				else
				{
					if(( '0' > *p) || ( '9' < *p )) return 0;
					value[i] = *p;
					p++;
					i++;
					if(i > 3) return 0;
				}
			}
			*pv = (unsigned char)atoi((const char *)value); //16.7.7
// 			*(++pv) = 0;
			return 1;
		}
	else return 0;
	}
	else return 0;
}

int Js_parser_array_int(int *res, const char *buffer, char *key )
{
	unsigned char i = 0;
	char value[5] = {0};
	char *p = NULL;
	int *pv = res;

	p = strstr(buffer,key);
	if(NULL != p)
	{
		p =p + strlen(key);
		if('\"' == *p) p++;
		if(':' == *p)
		{
			p++;
			if('\"' == *p)	p++;
			while(!(('\"' == *p) || (';' == *p) || ('\0' == *p) || ('&' == *p)))  // 16.7.4 wjy添加 ('&' == *p)
			{
				if((',' == *p) || (':' == *p) || ('-' == *p))
				{
					*pv = atoi(value);
					pv++;
					p++;
					i = 0;
					memset(value,0,5);
					continue;
				}
				else
				{
					if(( '0' > *p) || ( '9' < *p )) return 0;
					value[i] = *p;
					p++;
					i++;
					if(i > 3) return 0;
				}
			}
			*pv = atoi(value);
			return 1;
		}
		else return 0;
	}
	else return 0;
}

/************************************************************************/
/* 函数名：prarser_time													*/
/* 功能：获取JSON指令中的云端时间										*/
/*参数：	buffer：JSON指令字符串		key：获取参数名称				*/
/*返回值：云端时间														*/
/************************************************************************/
unsigned long prarser_time( char * const buffer,char *key )
{
	int i = 0;
	char data[11] = {0};
	char *p = NULL;

	p = strstr(buffer, key);
	if(NULL != p)
	{
		p =p + strlen(key);
		if('\"' == *p) p++;
		if(':' == *p)
		{
			p++;
			if('\"' == *p) p++;
			while(!((',' == *p) || ('}' == *p)|| ('\"' == *p)))
			{
				if(('0' > *p) || ('9' < *p))
				return 0;
				data[i] = *p;
				p++;
				i++;
			}
			return  strtoul(data, NULL, 10);
		}
		else	return 0;
	}
	else	return 0;
}

/************************************************************************/
/* 函数名：prarser_datetime												*/
/* 功能：获取JSON指令中的云端时间										*/
/*参数：	buffer：JSON指令字符串		key：获取参数名称				*/
/*返回值：云端时间														*/
/************************************************************************/
Time prarser_datetime(char * const buffer,char *key )  
{
	Time t;
	uint year = 0;
	uint mon = 0;
	uint mday = 0;
	uint wday = 0;
	uint hour = 0;
	uint min = 0;
	uint sec = 0;
	uint timecode = 0; //16.9.6 
	
	if(!Js_parser_value_int(&timecode,buffer,"timecode")) //16.9.6 
	{
		return t;
	}
	
	char *q = NULL;
	char enddate[50] = {0};
	Js_parser_string(enddate, buffer, key);

	q = strtok(enddate, "-");
	if(q) sscanf(q,"%d",&year);

	q= strtok(NULL,"-");
	if(q) sscanf(q,"%d",&mon);

	q= strtok(NULL,"-");
	if(q) sscanf(q,"%d",&mday);

	q= strtok(NULL,"-");
	if(q) sscanf(q,"%d",&wday);

	q= strtok(NULL,"-");
	if(q) sscanf(q,"%d",&hour);

	q= strtok(NULL,"-");
	if(q) sscanf(q,"%d",&min);

	q= strtok(NULL,"-");
	if(q) sscanf(q,"%d",&sec);

	
	
	if(timecode == (year + mon + mday + wday + hour + min + sec)) //16.9.6 
	{
		t.year = (uchar)(year - 2000);
		t.mon = (uchar)mon;
		t.mday = (uchar)mday;
		(wday == 0) ?(t.wday=7) : (t.wday = (uchar)wday);
		t.hour = (uchar)hour;
		t.min = (uchar)min;
		t.sec = (uchar)sec;
	}	

	return t;
}

/************************************************************************/
/* 函数名：parser_OpenValve*/
/* 功能：解析开水阀命令*/
/*参数：buffer：	JSON指令字符串*/
/*返回值：喷淋路数和时间*/
/************************************************************************/
int parser_open_valve(char * const buffer )
{
	char key[12] = {0};
	unsigned char strValve[MaxValve] = {0};
	unsigned int howlong[MaxValve] = {0}; // 16.7.4 wjy添加 多路RainOnce解析

	int i = 0;
	
	strcpy_P(key, PSTR("whichvalve"));
	if(Js_parser_array((char *)strValve, buffer, key))     //16.7.8
	{
// 		openv.whichvalve = tobit(strValve, sizeof(strValve));
// 		if (openv.whichvalve == 0)
// 		{
// 			return 0;
// 		}
		for (i = 0; i < MaxValve; i++)
		{
			if (strValve[i] > MaxValve)
			{
				return 0;
			}
		}
	}
	else
	{
		return 0;
	}
	
	strcpy_P(key, PSTR("howlong"));
	
	if(Js_parser_array_int((int *)howlong,buffer,key))  // 16.7.4 wjy添加 多路RainOnce解析
	{
		for(i = 0;i < MaxValve;i++)
		{
			if ((howlong[i] < 0) || (howlong[i] > MaxAutoTime))
			{
				return 0;
			}
		}
	}else
	{
		return 0;
	}
	
	if (ManuWaterMode != work_value)
	{
		for(i = 0; i < valves; i++)
		{
			wdt_reset();                    //16.7.26 问题等同manul water数据处理时间过长问题 需添加看门狗
			if ((strValve[i] > 0) && (strValve[i] <= valves))
			{
				start_spray(strValve[i],howlong[i] * 60);
			}
			else
			{
				break;
			}
		}
	}

	return 1;
}

/************************************************************************/
/* 函数名：parser_close_valve*/
/* 功能：解析关水阀命令*/
/*参数：	buffer：	JSON指令字符串*/
/*返回值：1成功 0 失败*/
/************************************************************************/
int parser_close_valve(char * const buffer ) //16.7.7
{
	char key[15] = {0};
	uchar strValve[MaxValve] = {0};
	int i;
	
	strcpy_P(key, PSTR("whichvalve"));
	if(Js_parser_array((char *)strValve, buffer, key)) //16.7.8
	{
		for (i = 0; i < MaxValve; i++)
		{
			if (strValve[i] > MaxValve)
			{
				return 0;
			}
		}
	}
	else
	{
		return 0;
	}
	
	if (ManuWaterMode != work_value) 
	{
		for(i = 0; i < MaxValve; i++)
		{
			wdt_reset(); //16.7.26   等同manul water start命令 问题 
			if ((strValve[i] > 0) && strValve[i] <= MaxValve)
			{
				spray_close_zone(spray_link, strValve[i]);
			}
			else
			{
				break;
			}
		}
	}
	
	return 1;
}

/************************************************************************/
/* 函数名：bit_reverse*/
/* 功能：将unsigned int 型的二进制数倒序*/
/*参数：	a: 源*/
/*返回值：结果*/
/************************************************************************/
unsigned  int bit_reverse(unsigned  int  a)
{
	int i = 0;
	unsigned int b = 0;
	for(i=0;i<16;i++)
	{
		b<<=1;
		b ^= a&1;
		a>>=1;
	}
	return b;
}

/************************************************************************/
/* 函数名：bit_to_asc*/
/* 功能：将uint型路数倒序后并转换成ASC码，保存到s的数组中*/
/*参数：	valve: 路数  s：字符串数组*/
/*返回值：无*/
/************************************************************************/
void bit_to_asc(unsigned  int valve, char *s)
{
	unsigned  int valve_temp = 0;
	valve_temp = bit_reverse(valve);
	sprintf_P(s, PSTR("%04X"), valve_temp);
}

void sprintf_float(char * res, float src)
{
	if (res != NULL)
	{
		sprintf_P(res, PSTR("%d.%d%d"), (int)src, ((int)(src*10))%10, ((int)(src*100))%10);
	}
}

void get_zone_time_string(char *dest)
{
	int i = 1;
	int j = get_length(spray_link);
	
	char temp[10] = {0};
	char temp2[10] = {0};
	char zone_temp[20] = {0};
	char time_temp[20] = {0};
	
	float remain_time;
	
	SLink *p = NULL;
	pSpray pspr;
	
	if (j > 0)
	{
		while(i <= j)
		{
			
			p = get_addr(spray_link,i);
			if (p != NULL)
			{
				pspr = (pSpray) p->data;
				
				remain_time = (float)pspr->howlong;
				
				if (1 == i)
				{
					sprintf_P(zone_temp, PSTR("%d"), pspr->zone);
					sprintf_float(time_temp, remain_time/60);
				}
				else
				{
					sprintf_P(temp, PSTR(":%d"), pspr->zone);
					strcat(zone_temp, temp);
					sprintf_float(temp2, remain_time/60);
					sprintf_P(temp, PSTR(":%s"), temp2);
					strcat(time_temp, temp);
				}
			}
			i++;
			if (i > 3)
			{
				break;
			}
			
		}
		
		sprintf_P(dest, PSTR("%s-%s"), zone_temp, time_temp);
	}
	else
	{
		return;
	}
}


void get_spray_zone_time(int addr, char * dest)
{
//	int i = 1;   //16.7.7 unused
	int j = get_length(spray_link);
	SLink *p = NULL;
	pSpray pspr;
	float remain_time;
	char zone_temp[20] = {0};
	char time_temp[20] = {0};
	
	if ((addr <= j) && (addr > 0)) 
	{
		p = get_addr(spray_link,addr);
		if (p != NULL)
		{
			pspr = (pSpray) p->data;
			
			remain_time = (float)pspr->howlong;
			sprintf_P(zone_temp, PSTR("%d"), pspr->zone);
			sprintf_float(time_temp, remain_time/60);
			sprintf_P(dest, PSTR("%s-%s"), zone_temp, time_temp);
		}
		
	}
}

void get_zone_time_now(char *dest)
{
	int i;
	char temp[20] = {0};
	int len = get_length(spray_link);
	
	for (i = 1; i <= len;i++)
	{
		if (i > devparam.mwspray)
		{
			break;
		}
		
		if (i > 1)
		{
			strcat(dest,",");
		}
		
		get_spray_zone_time(i,temp);
		strcat(dest,temp);
		
	}
}

void get_zone_time_inline(char * dest)
{
	int len = get_length(spray_link);
	int i;
	char temp[20] = {0};
		
	for (i = devparam.mwspray + 1; i<=len;i++)
	{
		if (i > devparam.mwspray + 1)
		{
			strcat(dest,",");
		}
		get_spray_zone_time(i,temp);
		strcat(dest,temp);
		if (i > 12)
		{
			break;
		}
	}
}

PROGMEM const  char  urlhead[] = "/api/rest/d";
PROGMEM const  char  conncloud[] = "/conncloud?";
PROGMEM const  char  senddevcodes[] = "/setno?";
PROGMEM const  char  getdevstatus[] = "/getdevstatus?";
PROGMEM const  char  sendzonenum[] = "/sendzonenum?";
PROGMEM const  char  getcommand[] = "/getcd?";
PROGMEM const  char  getdatetime[] = "/getdatetime?&cno=";
PROGMEM const  char  getsdf[] = "/getsdf?&cno=";
PROGMEM const  char  getsch[] = "/getsch?&cno=";
PROGMEM const  char  setwater[] = "/sntw?";
PROGMEM const  char  getdomain[] = "/getip?dno=";
PROGMEM const  char  getdevpm[] = "/getdevpm?&cno=";
PROGMEM const  char  sdstatus[] = "/sdstatus?&cno=";
PROGMEM const  char  sdevent[] = "/sdevent?";
PROGMEM const  char  senthumidity[] = "/humidity?&cno=";
PROGMEM const  char  getweather[] = "/getweatherbydevid?&cno=";
PROGMEM const  char  setraindelay[] = "/delay?&cno=";
PROGMEM const  char  setschweeks[] = "/seth?&cno=";
PROGMEM const  char  setschtimes[] = "/st?&cno=";
PROGMEM const  char  delsch[] = "/deles?&cno=";
PROGMEM const  char  setfine[] = "/fine?cno=";
PROGMEM const  char  getfine[] = "/getfine?&cno=";
PROGMEM const  char  stime[] = "/stime?";
PROGMEM const  char  sdsenor[] = "/uploadsensor?&cno=";
PROGMEM const  char  sdays[] = "/sday?&cno=";

/************************************************************************/
/* 函数名：get_cmd*/
/* 功能：获取访问云端接口指令*/
/*参数：	指令标志*/
/*返回值：无*/
/************************************************************************/

void get_cmd(  unsigned char flag )
{
	char strNum[6] = {0};
	
	cJSON *root = NULL;
	char *out = NULL;
	
	if (CmdFree == flag) //16.8.17 添加空闲指令,即不触发任何动作
	{
		return;
	}
	
	if (ReturnExecuted == flag) //16.8.19 
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","executedback");
		cJSON_AddStringToObject(root,"cno",connuid);
		cJSON_AddStringToObject(root,"cid",cid);
		
		goto cmd_exit;
	}
	
	if (ReturnCid == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","cmdfeedback");
		cJSON_AddStringToObject(root,"cno",connuid);
		cJSON_AddStringToObject(root,"cid",cid);
		cJSON_AddNumberToObject(root,"status",1);
		return_cid_flag = ReturnCidOK;
		
		goto cmd_exit;
	}
	
	if (GetCommand == flag)  //16.8.17 
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","getcd");
		cJSON_AddStringToObject(root,"cno",connuid);
		
		goto cmd_exit;
	}
	
	if (SentZoneTime == flag)
	{		
		char line[130] = {0};
		char now[20] = {0};
		cJSON *data=NULL;
		root = cJSON_CreateObject();

		cJSON_AddNumberToObject(root, "error", 200);
		
		cJSON_AddItemToObject(root,"data",data = cJSON_CreateObject());
		get_zone_time_now(now);
		cJSON_AddStringToObject(data,"now", now);
		get_zone_time_inline(line);
		cJSON_AddStringToObject(data,"inline", line);
		cJSON_AddStringToObject(root,"from","device");
		sent_zone_time_flag = SentZoneTimeOK;
		
		if(NULL != root)
		{
			out = cJSON_Print(root);
			send_mq_app_publish((unsigned char *)out); //16.7.8
		}
		goto exit;
	}

	if (ConnCloud == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "method", "conncloud");
		
		if(check_mac(devcode))   // 16.4.1 添加
		{
			cJSON_AddStringToObject(root,"dno", devcode);
		}
		else
		{
			set_mac_flag(0);
			wifimac_reload();
		}
		goto cmd_exit;
	}
	
	if (SendDevcodes == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "method", "setno");
		cJSON_AddStringToObject(root, "cno", connuid);
		cJSON_AddStringToObject(root, "pwd", serialcode);
		
		goto cmd_exit;
	}
	
	if (GetDevStatus == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "method", "getdevstatus");
		cJSON_AddStringToObject(root, "cno", connuid);
		
		goto cmd_exit;
	}
	
	if (SendZoneNum == flag)
	{		
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "method", "sendzonenum");
		cJSON_AddStringToObject(root, "cno", connuid);
		cJSON_AddNumberToObject(root, "num", valves);
		
		goto cmd_exit;
	}
	
	if (GetDateTime == flag)
	{
		char res[7] = {0};
			
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "method", "getdatetime");
		cJSON_AddStringToObject(root, "cno", connuid);
		get_vesion_str(res);
		cJSON_AddStringToObject(root, "ver", res);
// 		cJSON_AddStringToObject(root, "series", "PRO");
// 		cJSON_AddStringToObject(root, "model", "HWN12-200");
		
		goto cmd_exit;
	}
	
	if (SetWater == flag)
	{
		char param[25] = {0};
		pWaterUsedStruct pwus = NULL;
		
		pwus = get_water_used(water_used_link);
		
		if((pwus->zone > MaxValve) || (pwus->type > SparyUndef) || (pwus->zone == 0)) //排除错误数据
		{
			del_last_water_used(water_used_link);
			eeprom_read_water_used(water_used_link);
			if (get_length(water_used_link))
			{
				sentwater_flag = ReadyToSend;
			}
			else
			{
				sentwater_flag = NoDataToSend;		//上传数据成功
			}
			return;
		}
		
		if (NULL != pwus)
		{
			root = cJSON_CreateObject();
			cJSON_AddStringToObject(root,"method","sntw");
			cJSON_AddStringToObject(root,"c",connuid);
			sprintf_P(param,PSTR("%02X:%04X:%d:%02X:%02d%02d%02d%02d%02d%02d"),\
			pwus->zone, 
			pwus->howlong,
			pwus->type,
			pwus->adjust,
			pwus->end_time.year,
			pwus->end_time.mon,
			pwus->end_time.mday,
			pwus->end_time.hour,
			pwus->end_time.min,
			pwus->end_time.sec);
			cJSON_AddStringToObject(root,"p",param);
			
			goto cmd_exit;
		}
		
	}
	
	if(GetDomain == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","getip");
		cJSON_AddStringToObject(root,"dno",devcode);
		
		goto cmd_exit;
	}
	
	if (GetDevParam == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","getdevpm");
		cJSON_AddStringToObject(root,"cno",connuid);
		
		goto cmd_exit;
	}
	
	if (SendDevStatus == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","sdstatus");
		cJSON_AddStringToObject(root,"cno",connuid);
		
		switch(work_value)
		{
			case AutoMode:  cJSON_AddStringToObject(root,"status","AM");break;
			case  ManuWaterMode: cJSON_AddStringToObject(root,"status","MWM"); break;
			case ScheduleView:  cJSON_AddStringToObject(root,"status","SV"); break;
			case RainDelayView:	cJSON_AddStringToObject(root,"status","RDV"); break;
			case Configration:  cJSON_AddStringToObject(root,"status","CONF");break;
			case  OffMode:	cJSON_AddStringToObject(root,"status","OM"); break;
			case  WeatherAdjust: cJSON_AddStringToObject(root,"status","WA"); break;
			case  SpecialFeatures: cJSON_AddStringToObject(root,"status","SF"); break;
			default: break;
		}
		
		goto cmd_exit;
	}
	
// 	if (SendDevEvent == flag)            //16.7.5 此处屏蔽
// 	{
// 		char mode1[5] = {0};
// 		char mode2[5] = {0};
// 		char temp[30] = {0};
// 		
// 		root = cJSON_CreateObject();
// 		cJSON_AddStringToObject(root,"method","sdevent");
// 		cJSON_AddStringToObject(root,"cno",connuid);
// 		bit_to_asc(spray_state, mode1);
// 		bit_to_asc(spray_queue_state, mode2);
// 		sprintf(temp,"%s-%s",mode1,mode2);
// 		cJSON_AddStringToObject(root,"parm",temp);
// 		
// 		goto cmd_exit;
// 	}
	
	if (GetWeather == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","getweatherbydevid");
		cJSON_AddStringToObject(root,"cno",connuid);
		
		goto cmd_exit;
	}
	
	if (SetRainDelay == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","delay");
		cJSON_AddStringToObject(root,"cno",connuid);
		sprintf_P(strNum,PSTR("%d"),rain_delay.delay_time);
		cJSON_AddStringToObject(root,"interv",strNum);
		
		goto cmd_exit;
	}
		
	if (SetSch == flag)
	{
		char temp1[50] = {0};
		char temp2[20] = {0};
		int i = 0;           //16.7.8
		char j = 0;
		char tag[3] = {0};
			
		if(sch_upload->upload_status == NoUpload)
		{
			sch_upload->upload_status = Uploading;
		}
		
		
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","setschedule");
		sprintf_P(temp1,PSTR("%s"),connuid);
		cJSON_AddStringToObject(root,"cno",temp1);
		sprintf_P(temp1,PSTR("%d"),sch_upload->zones);
		cJSON_AddStringToObject(root,"zone",temp1);
		
		memset(temp1,0,sizeof(temp1));
		if(sch_upload->mode == BY_WEEKS)
		{
			for (i = 0; i < 7; i++)
			{
				j++;
				if(7 == j)
				{
					j = 0;
				}
				
				if(sch_upload->weeks &(1<<i))
				{
					sprintf_P(temp2,PSTR("%d"),j);
					strcat(temp1, temp2);
				}
			}
			cJSON_AddStringToObject(root,"date",temp1);
			
			sprintf(tag,"0");
		}
		else if(sch_upload->mode == BY_DAYS)
		{
			int days_h;
			int days_l;
			long days_temp = sch_upload->days;
			
			days_l = (int)(days_temp);
			days_h = (int)(days_temp >> 16);
			
			sprintf_P(temp1, PSTR("%04X%04X"),days_h, days_l);
			cJSON_AddStringToObject(root,"date",temp1);
			
			sprintf(tag,"1");
		}
		
		memset(temp1,0,sizeof(temp1));
		j = 0;
		for(i=0;i<4;i++)
		{
			if(sch_upload->howlong[i])
			{
				if(j>0)
				{
					strcat(temp1,",");
				}
				sprintf_P(temp2,PSTR("%d:%d-%d"),sch_upload->times[i*2], sch_upload->times[i*2+1],sch_upload->howlong[i]);
				strcat(temp1,temp2);
				j++;
			}
		}
		
		cJSON_AddStringToObject(root,"time",temp1);
		cJSON_AddStringToObject(root,"tag",tag);
	}
	
	if (DeleteSch == flag)
	{
		char temp[10] = {0};
		
		sprintf_P(temp, PSTR("%d"), delete_zone);
		
		root =  cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","deles");
		cJSON_AddStringToObject(root,"cno",connuid);
		cJSON_AddStringToObject(root,"num",temp);
		
		goto cmd_exit;
	}
	
	if (GetSchS == flag)
	{
		char temp[10] = {0};
		
		sprintf_P(temp, PSTR("%d"), get_sch_id);
		
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","getsch");
		cJSON_AddStringToObject(root,"cno",connuid);
		cJSON_AddStringToObject(root,"id",temp);
		
		goto cmd_exit;
	}
	
	if (SetAdjust == flag)
	{
		char disp[20] = {0};
		char temp[11] = {0};
		int i = 0;
				
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","fine");
		cJSON_AddStringToObject(root,"cno",connuid);
		for (i = 0; i < 5; i++)
		{
			sprintf_P(temp, PSTR("%02X"), weather_adjust.minadjust[i]);
			strcat(disp, temp);
		}
		strcat(content, disp);
		sprintf_P(temp, PSTR("%s-%02X"), disp,weather_adjust.adjust_mode);
		strcat(content, temp);
		cJSON_AddStringToObject(root,"fin",temp);
		
		goto cmd_exit;
	}
	
	if (GetAdjust == flag)
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","getfine");
		cJSON_AddStringToObject(root,"cno",connuid);
		
		goto cmd_exit;
	}
	
	if (SentSensor == flag)
	{
		sprintf_P(strNum, PSTR("%d"), sensor_value);
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","uploadsensor");
		cJSON_AddStringToObject(root,"cno",connuid);
		cJSON_AddStringToObject(root,"sensor",strNum);
		
		goto cmd_exit;
	}
	
	if (SetMaster == flag)
	{
		sprintf_P(strNum, PSTR("%d"), devparam.mastertag);
		
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","setmastertag");
		cJSON_AddStringToObject(root,"cno",connuid);
// 		cJSON_AddStringToObject(root,"mastertag",strNum);
		cJSON_AddNumberToObject(root,"mastertag",devparam.mastertag);
		
		goto cmd_exit;
	}
	
	if (SetDevType == flag)
	{		
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,"method","type");
		cJSON_AddStringToObject(root,"cno",connuid);
		cJSON_AddStringToObject(root,"s","pro");
		cJSON_AddStringToObject(root,"m","HWN12-200");
		
		goto cmd_exit;
	}
	
cmd_exit:
	if(NULL != root)
	{
		out = cJSON_Print(root);
		send_mq_publish((unsigned char *)out);      //16.7.8
	}
		
exit:
	cJSON_Delete(root);
	free(out);
}

/************************************************************************/
/* 函数名：tobit_week*/
/* 功能：将星期转换成位表示，1~7位表示周一至周日*/
/*参数：	路数数组*/
/*返回值：16位位表示的喷淋开启路数*/
/************************************************************************/
uchar tobit_week( char* strWeek )
{
	int i = 0;
	uchar weeks = 0;
	for (i = 0; i < 7; i++)
	{
		if (0 == strWeek[i])
		{
			strWeek[i] = 7;
		}

		if ((strWeek[i] < 1 ) || (strWeek[i] > 7))
		{
			break;
		}
		else
		{
			weeks |=(1<<(strWeek[i]-1));
		}
	}
	return weeks;
}

/************************************************************************/
/* 函数名：parser_Schedule*/
/* 功能：解析喷淋计划命令*/
/*参数：	buffer：JSON指令字符串*/
/*返回值：获得的喷淋计划*/
/************************************************************************/
int parser_schedule(Schedule *psch,char * const buffer) //16.7.7  passing argument 2 of 'parser_schedule' discards 'const' qualifier from pointer target type
{
	char strWeek[7] = {0};
	char key[10] = {0};
	char xishu[10] = {0};
	Schedule sh;
	
	memset(&sh,0,sizeof(Schedule));
	memset(strWeek, 0xff, sizeof(strWeek));
	sh.times[0] = 6;
	sh.times[2] = 10;
	sh.times[4] = 14;
	sh.times[6] = 18;

	sh.is_work = 0;
	sh.upload_status = Uploaded;
	sh.days = 0;
	strcpy_P(key, PSTR("howlong"));
	if(!Js_parser_array_int((int *)&sh.howlong[0], buffer, key)) //16.7.7
	{
		return 0;
	}
	strcpy_P(key, PSTR("times"));
	if(!Js_parser_array((char *)&sh.times[0], buffer, key)) //16.7.8
	{
		return 0;
	}
	
	for (int i = 0; i < (2 * TimesOfSchedule); i++)
	{
		if (i%2 == 0)
		{
			if (sh.times[i] > 23)
			{
				return 0;
			}
			
		} 
		else
		{
			if (sh.times[i] > 59)
			{
				return 0;
			}
		}
	}
	
	strcpy_P(key, PSTR("tag"));
	
	if (!Js_parser_value_char(&sh.mode, buffer, key))
	{
		return 0;
	}
	if(BY_WEEKS == sh.mode)
	{
		strcpy_P(key, PSTR("weeks"));
		if(Js_parser_array(strWeek, buffer, key))
		{
			sh.weeks = tobit_week(strWeek);
		}
		else
		{
			return 0;
		}	
	}
	else if (BY_DAYS == sh.mode)
	{		
		char str_days[20] = {0};
		int days_h;
		int days_l;
		strcpy_P(key, PSTR("weeks"));
		
		if(Js_parser_string(str_days, buffer, key))
		{
			sscanf_P(str_days, PSTR("%4x%4x"), &days_h, &days_l);
			sh.days |= ((long)days_h) << 16;
			sh.days |= ((long)days_l & 0x0000ffff);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
		
	strcpy_P(key, PSTR("num"));
	if(!Js_parser_value_char(&sh.zones, buffer, key))
	{
		return 0;
	}
	
	strcpy_P(key, PSTR("xishu"));
	if (Js_parser_string(xishu, buffer, key))
	{
		if(sscanf_f(xishu, &sh.xishu) < 0)
		{
			return 0;
		}
		
	}
	else
	{
		return 0;
	}
	
	if (!schedule_is_legal(sh))
	{
		return 0;
	}
	
	*psch = sh;
	return 1;
}

/************************************************************************/
/* 函数名：parser_DevParam*/
/* 功能：解析设备参数*/
/*参数：	buffer：JSON指令字符串*/
/*返回值：获得的设备参数*/
/************************************************************************/
int parser_dev_param(pDevParam devparam, char * const buffer ) //16.7.7   passing argument 2 of 'parser_dev_param' discards 'const' qualifier from pointer target type
{
	DevParam devp;
	char key[15] = {0};
	unsigned char mastertag = 0;
	memset((DevParam*)&devp,0,sizeof(DevParam));
	strcpy_P(key,PSTR("mwspray"));
	if(Js_parser_value_char(&devp.mwspray,buffer,key))
	{
		if((devp.mwspray < 1) || (devp.mwspray > MaxWMSpray))
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	// 	strcpy_P(key,PSTR("zonecount"));
	// 	Js_parser_value_char(&devp.zonecount,buffer,key);

	strcpy_P(key,PSTR("timezone"));
	Js_parser_value_char(&devp.timezone,buffer,key);
	
	
	
	strcpy_P(key,PSTR("unit"));
	Js_parser_value_char(&devp.tem_unit_flag,buffer,key);
	strcpy_P(key,PSTR("mastertag"));
	Js_parser_value_char(&mastertag,buffer,key);
	if (mastertag <= 1)
	{
		devp.mastertag = mastertag;
	}
	
	strcpy_P(key,PSTR("modeid"));
	if(Js_parser_value_char(&devp.program_id,buffer,key))
	{
		if (devp.program_id > PROMGRAM_D || devp.program_id < PROMGRAM_A)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	
	
	* devparam = devp;
	return 1;
}

/************************************************************************/
/* 函数名：Js_parser_value_char*/
/* 功能：获取JSON指令中的整数(char型)*/
/*参数：	res:获取的整数存放地址	buffer：JSON指令字符串		key：获取参数名称*/
/*返回值：1成功 0失败*/
/************************************************************************/
int Js_parser_value_char( unsigned char *res,const char *buffer, char *key )
{
	unsigned char i = 0;
	char data[6] = {0};
	char *p = NULL;

	p = strstr(buffer,key);
	if(NULL != p)
	{
		p =p + strlen(key);
		if('\"' == *p) p++;
		if(':' == *p)
		{
			p++;
			if('\"' == *p) p++;
			while(!((',' == *p) || ('}' == *p) || ('\"' == *p) || (';' == *p) || ('\0' == *p)))
			{
				if(( '0' > *p) || ( '9' < *p )) return 0;
				if( i > 4) return 0;
				data[i] = *p;
				p++;
				i++;
			}
			*res = (unsigned char)atoi(data);
			return 1;
		}
		else return 0;
	}
	else return 0;
}

/************************************************************************/
/* 函数名：parser_Sdstatus*/
/* 功能：解析推送旋钮状态返回信息*/
/*参数：	buffer :云端返回的数据*/
/*返回值：无*/
/************************************************************************/
void parser_Sdstatus(const char *buffer)
{
	char mode[5] = {0};
	char str_mode[5] = {0};
	char key[10] = {0};
	strcpy_P(key,PSTR("sdstatus"));
	switch(work_value)
	{
		case AutoMode:	strcpy_P(mode,PSTR("AM"));		break;
		case  ManuWaterMode:	strcpy_P(mode,PSTR("MWM")); break;
		case ScheduleView:	strcpy_P(mode,PSTR("SV"));	break;
		case RainDelayView:	strcpy_P(mode,PSTR("RDV")); break;
		case Configration: strcpy_P(mode,PSTR("CONF")); break;
		case  OffMode:	strcpy_P(mode,PSTR("OM"));break;
		case  WeatherAdjust: strcpy_P(mode,PSTR("WA"));break;
		case  SpecialFeatures:	strcpy_P(mode,PSTR("SF")); break;
		default: break;
	}
	Js_parser_string(str_mode,buffer,key);
	if (!strcmp(mode,str_mode))
	{
		send_work_value_flag = SentWorkModeOK;
	}
}

/************************************************************************/
/* 函数名：parser_Sdevent*/
/* 功能：解析推送阀门状态返回信息*/
/*参数：	buffer :云端返回的数据*/
/*返回值：无*/
/************************************************************************/
void parser_Sdevent(const char *buffer) //16.7.5 此处未被使用,仅添加注释，需注意bit_to_asc()第一参数类型为unsigned int
{
	char mode[5] = {0};
	char str_mode1[12] = {0};
	char str_mode2[12] = {0};
	char key[10] = {0};
		
	bit_to_asc(spray_state, mode);
	strcat(str_mode1, mode);
	strcat_P(str_mode1, PSTR("-"));
	bit_to_asc(spray_queue_state, mode);
	strcat(str_mode1, mode);
	
	strcpy_P(key,PSTR("sdevent"));
	Js_parser_string(str_mode2,buffer,key);
	if (!strcmp(str_mode1,str_mode2))
	{
		send_spary_status = SentSparyStatusOK;
	}
}

void setw_reback_handle( char * const data )  //16.7.7 passing argument 1 of 'setw_reback_handle' discards 'const' qualifier from pointer target type 
{
	char key[5] = {0};
 	char temp1[30] = {0};
	char temp2[30] = {0};
	pWaterUsedStruct pwus = NULL;
	
	pwus = get_water_used(water_used_link);
	if (NULL != pwus)
	{		
		sprintf_P(temp2,PSTR("%02X:%04X:%d:%02X:%02d%02d%02d%02d%02d%02d"),\
			pwus->zone,
			pwus->howlong,
			pwus->type,
			pwus->adjust,
			pwus->end_time.year,
			pwus->end_time.mon,
			pwus->end_time.mday,
			pwus->end_time.hour,
			pwus->end_time.min,
			pwus->end_time.sec);	
	}
	strcpy_P(key, PSTR("sntw"));
	Js_parser_string(temp1, data, key);
	
	if (!strcmp(temp1, temp2))
	{
		del_last_water_used(water_used_link);
		eeprom_read_water_used(water_used_link);
		if (get_length(water_used_link))
		{
			sentwater_flag = ReadyToSend;
		}
		else
		{
			sentwater_flag = NoDataToSend;		//上传数据成功
		}
	}
}

void parse_raindelay(char * const data )//16.7.7
{
	int temp = 0;
	char key[10] = {0};
	
	strcpy_P(key, PSTR("interv"));
	Js_parser_value_int(&temp, data, key);
	if((temp > MaxRainDelayTime) || (temp < 0))
	{
		return;
	}
	else
	{
		rain_delay.delay_time = temp;
		rain_delay.begin_time = current_time;
		setting_delaytime_flag = 0;
		save_raindelay();
	}
}

void parse_deletesch(const char * data)
{
	char key[5] = {0};
	uchar zone = 0;
	strcpy_P(key, PSTR("deles"));
	Js_parser_value_char(&zone, data,key);
	
	reset_del_zone(zone);
	
	if (zone == delete_zone)
	{
		delete_zone = get_del_zone();
		if (!delete_zone)
		{
			set_del_zone_flag = SetSchOK;
		}
	}
}

void parse_getfine(char * const data)		
{
	int i = 0;
	char *p =NULL;
	char key[10] = {0};
	uchar tag[5] = {0};
		
	FineTune ft;
	
	adjust_mode_temp = 0;
	ft.adjust_mode = 0;
	
	p = data;
	for (i = 0; i < 5; i++)
	{
		strcpy_P(key, PSTR("finetune"));
		p =strstr(p, key);
		if (NULL != p)
		{
			if(Js_parser_value_char(&ft.minadjust[i], p, key))
			{
				strcpy_P(key, PSTR("tag"));
				if(Js_parser_value_char(&tag[i], p, key))
				{
					if (1 == tag[i])
					{
						ft.adjust_mode |= 1 << (4 - i);
					}
				}
				else
				{
					return;
				}
				p++;
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	ft.set_t = current_time;
	weather_adjust = ft;
	save_finetune(weather_adjust);
	memcpy((char *)adjust_value_temp, (char *)weather_adjust.minadjust, 5);
	get_adjust_flag = GetAdjustOK;
}


void dev_remove(void)
{
	conn_cloud_flag = DisConnCloud;
	send_devcodes_flag = DisSendDevcodes;
	user_band_flag = DisBanded;
	
	if(RegSmartDevPage == CurrentPage)
	{
		LCD_clear();
	}
	reset_band_status();
}

int time_legal(Time t)
{
	if ((t.sec < 0) || (t.sec > 59))
	{
		return 0;
	}
	
	if ((t.min < 0) || (t.min > 59))
	{
		return 0;
	}
	
	if ((t.hour < 0) || (t.hour > 23))
	{
		return 0;
	}
	
	if ((t.mday < 1) || (t.mday > 31))
	{
		return 0;
	}
	
	if ((t.mon < 1) || (t.mon > 12))
	{
		return 0;
	}
	
	if ((t.wday < 1) || (t.wday > 7))
	{
		return 0;
	}
	
	if (t.year  > 99)
	{
		return 0;
	}
	
	else		
		return 1;
}

void get_cid(char * const data)
{
	char key[5] = {0};
	strcpy_P(key, PSTR("cid"));
	Js_parser_string(cid, data, key);
}

void error_handle( int error_type, char * const data) //16.7.7  passing argument 2 of 'error_handle' discards 'const' qualifier from pointer target type
{
	if (SetDevcodeErr == error_type) //16.4.1新添加
	{
		set_mac_flag(0);
		wifimac_reload();
		return;
	}//
	
	if (DevBoundedErr == error_type)		//设备已经绑定
	{
		send_devcodes_flag = SendDevcodesOk;// 		Js_parser_string(devid,data,"devid");
		return;
	}
	
	if (DelSchIDErr == error_type)		//该计划云端已经删除
	{
		parse_deletesch(data);
		return;
	}
	
	if (DevIsolatedErr == error_type)  //设备分离
	{
		check_command_flag = CheckCmdOk; //16.8.17
		dev_remove();
		return;
	}
	
	if ((GetStatusUidErr == error_type) || (GetTimeUidErr == error_type)\
		 || (SetnoUidErr == error_type) || (GetWeatherUidErr == error_type)\
		 || (GetDomainUidErr == error_type) || (GetCdUidErr == error_type))
	{
		conn_cloud_flag = DisConnCloud ;
		return;
	}
	
	if (DevidErr == error_type)
	{
		send_devcodes_flag = DisSendDevcodes;
		return;
	}
	
	if (SentSchErr == error_type)
	{
		if (!schedule_is_legal(*sch_upload))
		{
			sch_upload = NULL;
			set_sch_flag = SetSchOK;
		}
		
		return;
	}
}

DevParam get_update_param( const char *buffer )
{
	DevParam devp;
	char key[15] = {0};
	
	memset((DevParam*)&devp,0,sizeof(DevParam));
	
	strcpy_P(key,PSTR("mastertag"));
	Js_parser_value_char(&devp.mastertag,buffer,key);
	
	return devp;
}



/************************************************************************/
/* 函数名：parse_commands*/
/* 功能：		解析JSON命令*/
/*参数：	data：JSON指令字符串		command_name：命令名称*/
/*返回值：无*/
/************************************************************************/
static void parse_commands(const char *data, const char *command_name)
{
	char key[20] = {0};
	
	if (!strcmp_P(command_name, PSTR("RainOnce")))		//即时喷淋开
	{
		get_cid((char * const)data);
		return_cid_flag = NeedReturnCid;
		if(strcmp(cid,cid_temp))           //16.8.17 添加cid比较，避免重复执行
		{
			if (parser_open_valve((char * const)data))  //16.7.8
			{
				strncpy(cid_temp,cid,LenOfCid);                  //16.7.8
			}
		}
		
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("RainClose")))		//即时喷淋关
	{
		get_cid((char * const)data);
		return_cid_flag = NeedReturnCid;
		if(strcmp(cid,cid_temp))      //16.8.17  添加cid比较，避免重复执行
		{
			if (parser_close_valve((char * const)data))		//16.7.8
			{
				strncpy(cid_temp,cid,LenOfCid);			//16.7.8
			}
		}
		
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("UpdateSchedule")))		//更新喷淋计划
	{
		Schedule sch;
			if(parser_schedule(&sch,(char * const)data)) //16.7.8
			{
				insert_epschedule(sch_link,sch); //16.7.6
				insert_sch(sch_link, sch);
				get_cid((char * const)data);			//16.7.8
				return_cid_flag = NeedReturnCid;
			}

		return;
	}
	
	if (!strcmp_P(command_name, PSTR("SetSchedule")))		//添加喷淋计划
	{
		Schedule sch;
			if(parser_schedule(&sch, (char * const)data))
			{
				insert_epschedule(sch_link,sch);   //16.7.6
				insert_sch(sch_link, sch);
				//			save_schedule(sch_link);
				get_cid((char * const)data);			//16.7.8
				return_cid_flag = NeedReturnCid;
			}
		
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("DeleteSchedule")))		//删除喷淋计划
	{
		uchar zone = 0;
			strcpy_P(key, PSTR("num"));
			Js_parser_value_char(&zone, data, key);
			del_sch(sch_link, zone);
			get_cid((char * const)data);		//16.7.8
			return_cid_flag = NeedReturnCid;
		
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("RainDelay")))		//延迟喷淋命令
	{
			get_cid((char * const)data);			//16.7.8
			return_cid_flag = NeedReturnCid;
			parse_raindelay((char * const)data);  //
		
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("UpdateParam")))		//更新参数命令
	{
			get_cid((char * const)data);			//16.7.8
			return_cid_flag = NeedReturnCid;
			get_param_flag =  DisGetParam;
			strcpy_P(key,PSTR("mastertag"));
			Js_parser_value_char(&devparam.mastertag,data,key);

		return;
	}
	
	if (!strcmp_P(command_name, PSTR("UpdateFine")))		//更新微调命令
	{
			get_cid((char * const)data);		//16.7.8
			return_cid_flag = NeedReturnCid;
			get_adjust_flag = DisGetAdjust;
		
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("UpdateZipCode")))		//更新Zip Code
	{
			get_cid((char * const)data);		//16.7.8
			return_cid_flag = NeedReturnCid;
			get_adjust_flag = DisGetAdjust;
			get_weather_flag = NoGetWeather;
		
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("UpdateMode")))		//更新 program mode
	{
			get_cid((char * const)data);			//16.7.8
			return_cid_flag = NeedReturnCid;
			del_all_sch(sch_link);
			get_schedule_flag = UpdataAllSch;
			get_param_flag = DisGetParam;
		return;
	}
	
	if (!strcmp_P(command_name, PSTR("UpdataServe")))
	{

			get_cid((char * const)data);		//16.7.8
			return_cid_flag = NeedReturnCid;
			get_domain_flag = DisGetDomain;

		return;
	}
	
	if (!strcmp_P(command_name, PSTR("Upgrade")))
	{
		char ver1[10] = {0};
		char ver2[10] = {0};
			strcpy_P(key, PSTR("version"));
			if (Js_parser_string(ver1,data,key))
			{
				get_vesion_str(ver2);
				if((strcmp(ver1,ver2) > 0) && (WIFI_MODE == net_mode))
				{
					upgrade_firmware();
				}
				else
				{
					get_cid((char * const)data);			//16.7.8
					return_cid_flag = NeedReturnCid;
				}
			}
		
		return;
	}
	
	if(!strcmp_P(command_name,PSTR("Binding")))
	{
			get_cid((char * const)data);		//16.7.8
			return_cid_flag = NeedReturnCid;
			user_band_flag = UserBandOk;
			save_band_status();
			if(RegSmartDevPage == CurrentPage)
			LCD_clear();

		return;
	}	
	if(!strcmp_P(command_name,PSTR("UnBinding")))
	{
			get_cid((char * const)data);			//16.7.8
			return_cid_flag = NeedReturnCid;
			dev_remove();

		return;
	}
	
	if(!strcmp_P(command_name,PSTR("SyncSpray")))
	{
			get_cid((char * const)data);			//16.7.8
			return_cid_flag = NeedReturnCid;
			sent_zone_time_flag = NeedSentZoneTime;

		return;
	}
}

/************************************************************************/
/* 函数名：parse_cloud_API*/
/* 功能：		解析JSON指令入口*/
/*参数：	data：JSON指令字符串		len：JSON指令字符串长度*/
/*返回值：无*/
/************************************************************************/
void parse_cloud_API( const char *  data ,const unsigned int len)
{
	unsigned int error = 0;
	unsigned int band_status = 0;
	char command_name[20] = {0};
	char key[15] = {0};
//	char *out;   //16.7.7 unused variable 'out'
	cJSON *json = NULL;
	char *method;
	
	
	json=cJSON_Parse(data);
	if (!json)
	{
		return;
	}
	
//	USART0_send_n(data,len);
	
	error = cJSON_GetObjectItem(json, "error")->valueint;
	
	if (200 == error)
	{
		
		method = cJSON_GetObjectItem(json, "method")->valuestring;
		
		if (!strcmp_P(method,PSTR("cmdfeedback")))   //缺省设置，暂时无任务
		{
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("command")))
		{
			strcpy_P(key,PSTR("command_name"));
			Js_parser_string(command_name, data,key);
			parse_commands(data, command_name);
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("getcd"))) //16.8.17
		{
			check_command_flag = CheckCmdOk;
			strcpy_P(key,PSTR("command_name"));
			Js_parser_string(command_name, data,key);
			parse_commands(data, command_name);
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("conncloud")))
		{
			cJSON * data = cJSON_GetObjectItem(json, "data");
			
			if (NULL != data)
			{
				char *str_connuid = cJSON_GetObjectItem(data, "connuid")->valuestring;
				strcpy(connuid, str_connuid);
				conn_cloud_flag = ConnCloudOk;
			}
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("setno")))	//发送设备号和序列码
		{
			send_devcodes_flag = SendDevcodesOk;
			goto exit;
		}
		

		if (!strcmp_P(method, PSTR("getdatetime")))
		{
			Time t;
			strcpy_P(key,PSTR("time"));
			t = prarser_datetime((char * const)data,key);  //16.7.8
			if (time_legal(t))
			{
				RTC_time_set(t);
				rain_delay.begin_time = t;
				save_raindelay();		//更新延迟喷淋设置时间
				weather_adjust.set_t = t;
				save_finetune(weather_adjust);		//更新微调设置时间
				get_time_flag =GetTimeOK;
			}
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("getdevstatus")))		//获取关联状态
		{
			strcpy_P(key, PSTR("status"));
			Js_parser_value_int((int *)&band_status,data,key ); //16.7.8
			if (1 == band_status)
			{
				user_band_flag = UserBandOk;
				save_band_status();
				if(RegSmartDevPage == CurrentPage)
				LCD_clear();
			}
			
			get_devstatus_flag = OkGet;
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("sdstatus")))			//推送工作模式
		{
			parser_Sdstatus(data);
			goto exit;
		}
		
// 		if (!strcmp_P(method, PSTR("sdevent")))		//推送当前阀门状态  16.7.5 此处屏蔽
// 		{
// 			parser_Sdevent(data);
// 			goto exit;
// 		}	
		
		if (!strcmp_P(method, PSTR("stime")))			//上传路数时间
		{
			sent_zone_time_flag = SentZoneTimeOK;
			goto exit;
		}	
		
		if (!strcmp_P(method, PSTR("uploadsensor"))) //上传传感器状态
		{
			sent_sensor_state = SentSensorOk;
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("getdevpm")))	//获得设备参数
		{
			DevParam devp;
			unsigned int spray_time_limit_tmp;
			
			if (parser_dev_param(&devp, (char * const)data)) //16.7.8
			{
				if (devparam.timezone != devp.timezone)
				{
					get_time_flag = GetTimeFailed;
				}
				
				strcpy_P(key, PSTR("waterlong"));
				if(Js_parser_value_int((int *)&spray_time_limit_tmp,data, key)) //16.7.7
				{
					if ((MaxManuTime < spray_time_limit_tmp) || (spray_time_limit_tmp == 0))
					{
						return;
					}
					else
					{
						spray_time_limit = spray_time_limit_tmp;
						save_spray_time_limit(spray_time_limit);
					}
				}
				else
				{
					return;
				}
				
				devparam = devp;
				get_param_flag = GetParamOK;
				save_devparam();
			}
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("getweatherbydevid")))    //获取天气
		{
			double temp = 0;
			
			strcpy_P(key, PSTR("min_temp"));
			if(!Js_parser_value_int(&weather.tempminF, data, key))
			{
				goto exit;
			}
			strcpy_P(key, PSTR("max_temp"));
			if(!Js_parser_value_int(&weather.tempmaxF, data, key))
			{
				goto exit;
			}
			strcpy_P(key, PSTR("rain_probability"));
			if(!Js_parser_value_char(&weather.raining, data, key))
			{
				goto exit;
			}
			temp = (weather.tempmaxF - 32)*5/9;
			weather.tempmaxC = (int)temp;
			temp =  (weather.tempminF - 32)*5/9;
			weather.tempminC = (int)temp;
			get_weather_flag = GetWeatherOk;
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("delay")))		//上传延迟时间
		{
			char temp1[5];
			char temp2[5];
			strcpy_P(key, PSTR("data"));
			Js_parser_string(temp1, data, key);
			sprintf_P(temp2, PSTR("%d"), rain_delay.delay_time);
			if(!strcmp(temp2, temp1))
			{
				set_raindelay_flag = SetRainDelayOK;
			}
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("getfine")))		//获取微调
		{
			parse_getfine((char * const)data);   //16.7.8
			goto exit;
		}

		if (!strcmp_P(method, PSTR("fine")))		//上传微调
		{
			set_adjust_flag = SetAdjustOK;
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("deles")))		//本地删除喷淋计划
		{
			parse_deletesch(data);
			goto exit;
		}
		
		if (!strcmp_P(method,PSTR("setschedule")))
		{
			if(sch_upload->zones == cJSON_GetObjectItem(json, "data")->valueint)
			{
				set_sch_flag = SetSchOK;
				sch_upload->upload_status = Uploaded;
				updata_epschedule(sch_link,*sch_upload);  //16.7.6
				updata_sch(sch_link, *sch_upload);
			}
			goto exit;
		}
			
		if (!strcmp_P(method, PSTR("sdstatus")))			//推送工作模式
		{
			parser_Sdstatus(data);
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("sntw")))		//上传用水量
		{
			setw_reback_handle((char * const)data);  //16.7.8
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("getsch")))     //自动更新喷淋计划
		{
			Schedule sch;
			char num = 0;
				
			if(parser_schedule(&sch, (char * const)data))
			{
				if (sch.zones == get_sch_id)
				{
					insert_epschedule(sch_link,sch); //16.7.6
					insert_sch(sch_link, sch);
//					save_schedule(sch_link);  
					get_sch_id ++;
					if (get_sch_id > valves)
					{
						get_sch_id = 1;
						get_schedule_flag = GetScheduleOK;
					}
				}
			}
			else
			{
				strcpy_P(key, PSTR("num"));
				if(Js_parser_value_char((unsigned char*)&num, data, key)) //16.7.7
				{
					if (num == get_sch_id)
					{
						get_sch_id ++;
						if (get_sch_id > valves)
						{
							get_sch_id = 1;
							get_schedule_flag = GetScheduleOK;
						}
					}
				}
			}
				
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("setmastertag")))		//上传MASTER
		{
			sent_master_state = SENT_OK;
			goto exit;
		}
		
		if (!strcmp_P(method, PSTR("type")))		//上传MASTER
		{
			sent_device_type_flag = SENT_OK;
			goto exit;
		}
		
		if(!strcmp_P(method, PSTR("sendzonenum")))
		{
			sent_zone_num_flag = SENT_OK;
			goto exit;
		}
		
	}
	else
	{
		error_handle(error, (char * const)data); //16.7.8
	}


	exit:
		cJSON_Delete(json);
}
