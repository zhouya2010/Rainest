/*
 * CMD_Ctr.c
 *
 * Created: 2014/6/10 9:01:49
 *  Author: ZY
 */ 
#include "btn_ctr.h"
#include "CMD_Ctr.h"
#include "json.h"
#include <avr/pgmspace.h>

unsigned char net_connect_flag = NetDisConnected;	//网络连接状态
unsigned char user_band_flag = DisBanded;  //用户是否绑定标志 
unsigned char get_devstatus_flag = DisGet;
unsigned char conn_cloud_flag = DisConnCloud;//是否连接云端成功标志 
unsigned char send_devcodes_flag = DisSendDevcodes;//是否上传设备序列码给云端成功
unsigned char get_time_flag = GetTimeFailed;	//是否获取到云端时间
unsigned char get_param_flag = DisGetParam ;//是否获取参数
unsigned char sentwater_flag = NoDataToSend;//是否有用水量要上传到云端
unsigned char get_domain_flag = GetDomainOK;//轮询域名标志
unsigned char send_work_value_flag = SentWorkMode; //发送旋钮选择模式标志
unsigned char send_spary_status = SentSparyStatus; //发送当前喷淋路数状态标志
unsigned char sent_sensor_flag = SentSensorStatusOk;//发送传感器标志
unsigned char set_adjust_flag = SetAdjustOK;
unsigned char get_adjust_flag = DisGetAdjust;
unsigned char get_weather_flag = NoGetWeather;
unsigned char set_raindelay_flag = SetRainDelayOK;//上传延迟喷淋标志
unsigned char set_sch_flag = SetSchOK;
unsigned char set_del_zone_flag = SetSchOK;
unsigned char get_schedule_flag = GetScheduleOK;
unsigned char sent_zone_time_flag = NeedSentZoneTime;
unsigned char sent_sensor_state = NeedSentSensor;
unsigned char sent_master_state = SENT_OK;
unsigned char return_cid_flag = ReturnCidOK;
unsigned char return_executed_flag = ReturnExecutedOK;  //16.8.19
unsigned char sent_device_type_flag = SENT_FAILED;
unsigned char sent_zone_num_flag = SENT_OK;
unsigned char check_command_flag = CheckCmdOk;  //16.8.17

static unsigned char sent_cloud_count = 0;
static unsigned char content_flag = 0;
static unsigned char content_flag_tmp = 0;

extern SLink * water_used_link;

// PROGMEM const  char  mainserve[] = "www.rainmq.com";
// PROGMEM const  char  mainserve[] = "139.162.2.26";
//Test Serve
//PROGMEM const  char  mainserve[] = "45.33.46.130";

#define ConnectLimit	10


/************************************************************************/
/* 函数名：	UserBand_CMD												*/
/* 功能：用户关联设备访问接口流程控制									*/
/*参数：	无															*/
/*返回值：无															*/		
/************************************************************************/
void user_band_cmd( void )
{
	if (DisConnCloud == conn_cloud_flag)
	{
		get_cmd(ConnCloud);
	}
	else if(DisSendDevcodes == send_devcodes_flag )
	{
		get_cmd(SendDevcodes);
	}
	else if(NeedReturnExecuted == return_executed_flag) //16.8.19
	{
		get_cmd(ReturnExecuted);
	}
	else if(NeedReturnCid == return_cid_flag)//16.8.17 增加非绑定状态返回cid
	{
		get_cmd(ReturnCid);
	}
	else if (NeedCheckCmd == check_command_flag)//16.8.17
	{
		get_cmd(GetCommand);
	//	check_command_flag = CheckCmdOk;
	}
	else if (GetTimeFailed == get_time_flag)
	{
		get_cmd(GetDateTime);
	}
	else if (DisGet == get_devstatus_flag)
	{
		get_cmd(GetDevStatus);
	}
}

/************************************************************************/
/* 函数名：	AUTO_CMD													*/
/* 功能：		自动模式下访问云端接口流程控制							*/
/*参数：	无															*/
/*返回值：无															*/
/************************************************************************/
void auto_cmd(void)
{
	if (GetDomainOK != get_domain_flag)
	{
		content_flag = GetDomain;
	}
	else if (DisConnCloud == conn_cloud_flag)   
	{
		content_flag = ConnCloud; 
		sent_cloud_count++;                 //2016.9.16 
		if(sent_cloud_count >= 10)
		{
			sent_cloud_count = 0;
			conn_cloud_flag = ConnCloudOff;
		}		
	}
	
	if(conn_cloud_flag == ConnCloudOk)   //2016.9.16  conncloud通过后，以下接口才能调用
	{
		sent_cloud_count = 0;
		
		if (NeedReturnExecuted == return_executed_flag) //16.8.19
		{
			content_flag = ReturnExecuted;
		}
		else if(NeedReturnCid == return_cid_flag)
		{
			content_flag = ReturnCid;
		}
		else if (NeedCheckCmd == check_command_flag)//16.8.17
		{
			content_flag = GetCommand;
		}
		else if(DisSendDevcodes == send_devcodes_flag)
		{
			content_flag = SendDevcodes;
		}
		else if (GetTimeFailed == get_time_flag)
		{
			content_flag = GetDateTime;
		}
		else if (SENT_FAILED == sent_zone_num_flag)
		{
			content_flag = SendZoneNum;
		}
		// 	else if (SentSparyStatus == send_spary_status)
		// 	{
		// 		content_flag = SendDevEvent;
		// 	}
		else if (NeedSentZoneTime == sent_zone_time_flag)
		{
			content_flag = SentZoneTime;
		}
		else if (NeedSentSensor == sent_sensor_state)
		{
			content_flag = SentSensor;
		}
		else if (GetParamOK != get_param_flag)
		{
			content_flag = GetDevParam;
		}
		else if (NoGetWeather == get_weather_flag)
		{
			content_flag = GetWeather;
		}
		else if (NeedSetRainDelay == set_raindelay_flag)
		{
			content_flag = SetRainDelay;
		}
		else if (NeedSetAdjust == set_adjust_flag)
		{
			content_flag = SetAdjust;
		}
		else if (DisGetAdjust == get_adjust_flag)
		{
			content_flag = GetAdjust;
		}
		else if (NeedSetSch == set_del_zone_flag)
		{
			content_flag = DeleteSch;
		}
		else if (SentWorkMode == send_work_value_flag)
		{
			content_flag = SendDevStatus;
		}
		else if (ReadyToSend == sentwater_flag)
		{
			content_flag = SetWater;
		}
		else if(NeedSetSch == set_sch_flag)
		{
			content_flag = SetSch;
		}
		else if(UpdataAllSch == get_schedule_flag)
		{
			content_flag = GetSchS;
		}
		else if(SENT_FAILED == sent_device_type_flag)
		{
			content_flag = SetDevType;
		}
		else if (SENT_FAILED == sent_master_state)
		{
			content_flag = SetMaster;
		}
		else
		{
			content_flag = CmdFree; //16.8.17
			sent_cloud_count = 0;
		}
	}
	
	
	get_cmd(content_flag);
	
	
// 	if(content_flag != ConnCloud)     //2016.9.16  
// 	{
// //		content_flag_tmp = content_flag;
// 		sent_cloud_count = 0;
// 	}
// 	else
// 	{
// 		sent_cloud_count++;
// 	}
}

// void set_mainserve(void)
// {
// 	strcpy_P(serve,mainserve);
// }

/************************************************************************/
/* 函数名：		CMD_Ctr													*/
/* 功能：        控制访问云端指令流程。分为三种模式：1、未关联设备状态	*/
/* 						2、AUTO模式	3、手动模式							*/
/*参数：	无															*/
/*返回值：无                											*/
/************************************************************************/
void cmd_ctr( void )
{
	if (UserBandOk != user_band_flag)	//未绑定状态
	{
		user_band_cmd();
	}
	else
	{
		auto_cmd();
	}
}

