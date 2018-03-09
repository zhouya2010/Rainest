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

unsigned char net_connect_flag = NetDisConnected;	//��������״̬
unsigned char user_band_flag = DisBanded;  //�û��Ƿ�󶨱�־ 
unsigned char get_devstatus_flag = DisGet;
unsigned char conn_cloud_flag = DisConnCloud;//�Ƿ������ƶ˳ɹ���־ 
unsigned char send_devcodes_flag = DisSendDevcodes;//�Ƿ��ϴ��豸��������ƶ˳ɹ�
unsigned char get_time_flag = GetTimeFailed;	//�Ƿ��ȡ���ƶ�ʱ��
unsigned char get_param_flag = DisGetParam ;//�Ƿ��ȡ����
unsigned char sentwater_flag = NoDataToSend;//�Ƿ�����ˮ��Ҫ�ϴ����ƶ�
unsigned char get_domain_flag = GetDomainOK;//��ѯ������־
unsigned char send_work_value_flag = SentWorkMode; //������ťѡ��ģʽ��־
unsigned char send_spary_status = SentSparyStatus; //���͵�ǰ����·��״̬��־
unsigned char sent_sensor_flag = SentSensorStatusOk;//���ʹ�������־
unsigned char set_adjust_flag = SetAdjustOK;
unsigned char get_adjust_flag = DisGetAdjust;
unsigned char get_weather_flag = NoGetWeather;
unsigned char set_raindelay_flag = SetRainDelayOK;//�ϴ��ӳ����ܱ�־
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

#define ConnectLimit	10


/************************************************************************/
/* ��������	UserBand_CMD												*/
/* ���ܣ��û������豸���ʽӿ����̿���									*/
/*������	��															*/
/*����ֵ����															*/		
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
	else if(NeedReturnCid == return_cid_flag)//16.8.17 ���ӷǰ�״̬����cid
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
/* ��������	AUTO_CMD													*/
/* ���ܣ�		�Զ�ģʽ�·����ƶ˽ӿ����̿���							*/
/*������	��															*/
/*����ֵ����															*/
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
	
	if(conn_cloud_flag == ConnCloudOk)   //2016.9.16  conncloudͨ�������½ӿڲ��ܵ���
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
/* ��������		CMD_Ctr													*/
/* ���ܣ�        ���Ʒ����ƶ�ָ�����̡���Ϊ����ģʽ��1��δ�����豸״̬	*/
/* 						2��AUTOģʽ	3���ֶ�ģʽ							*/
/*������	��															*/
/*����ֵ����                											*/
/************************************************************************/
void cmd_ctr( void )
{
	if (UserBandOk != user_band_flag)	//δ��״̬
	{
		user_band_cmd();
	}
	else
	{
		auto_cmd();
	}
}

