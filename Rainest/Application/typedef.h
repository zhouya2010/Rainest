/*
 * typedef.h
 *
 * Created: 2014/7/2 17:02:20
 *  Author: ZY
 */ 

#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include <avr/io.h>

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

#ifndef ul
#define ul unsigned long
#endif

//系统时钟(Hz)
#ifndef F_CPU
#define F_CPU              12000000UL
#endif

#define  MaxValve				48	//设备最大可控制路数
#define  MaxValveManu			48	//手动控制路数
#define  LenOfDevcodes			12	//设备号长度
#define  LenOfConnuid			10	//链接码长度
#define  LenOfSerialCode		4	//随机码长度
#define  LenOfDevid				8	//设备ID长度
#define  LenOfServe				25	//服务器地址长度
#define  LenOfCid				15	//Cid长度
#define  LenOfPath				51	//路径最大长度
#define  MaxScheduleNum			36	//最大可设置喷淋计划数
#define  TimesOfSchedule		4	//单个喷淋计划可设置时段数
#define  MaxManuTime			600	//设置最大手动喷淋时间(min)
#define  MaxAutoTime			600	//最大及时喷淋时间(min)
#define  MaxRainDelayTime		(72*60)	//最大延迟喷淋时间 单位：小时
#define  MaxtAdjustValue		150	//最大可设微调数
#define  NumOfAdjustDays		5	//微调可设置天数
#define  MaxWMSpray				3	//最大可设同时喷淋路数
#define  WaterUsedNum			(50-1) //设置最大保存喷淋记录条数
#define  MasterZone				12	//主阀门
#define  SprayEepromNum			80

typedef  struct SprayTimely_Struct {
	uint whichvalve;
	uint howlong;
}SprayTimely,*pSprayTimely;


typedef struct Schedule_Struct{
	uchar upload_status;	//喷淋计划上传状态
	uchar is_work;	//喷淋计划当前状态 0-启用 1-停用
	uchar is_weather;//是否根据天气调节 0-启用 1-不启用，为100%
	uchar weeks;//一周内那几天喷,位标志，0~6位分别对应周一至周日
	uchar times[2 * TimesOfSchedule];//每天喷淋的时间点，10进制表示，两个字节表示一个时间，共4个时间段 08 ：20
	uchar zones;//控制路数 1~12路
	uint howlong[TimesOfSchedule]; //每个定时的喷淋时间,单位：min
	uchar mode;   //执行模式：0-按星期执行  1-按天执行
	float xishu; //喷淋时长调节系数
	long days;	//二进制表示，最低位为第1天
	}Schedule,*pSchedule;
	
typedef struct 	DevParam_Struct{
// 	uchar hassensor;//是否有传感器
// 	uchar sensorinterv;//传感器上传数据时间间隔,单位小时
// 	char fwver[4];//版本号
	uchar mwspray;//设备可同时喷淋的最大路数 [1~3]
// 	uchar zonecount;//设备最多可连接的阀门数
	uchar timezone;//时区ID
// 	uchar isweather;// ON/OFF weather system 0-by manual, 1-by weather
// 	int maxelectric;//设备最大电流
	uchar mastertag;//是否启用master功能
	uchar tem_unit_flag;//温度单位
	uchar program_id;
	}DevParam,*pDevParam;
	
#ifndef _TM_DEFINED
#define _TM_DEFINED
typedef struct tm_struct {
	uchar sec;     // seconds after the minute - [0,59] 
	uchar min;    //  minutes after the hour - [0,59] 
	uchar hour;   //  hours since midnight - [0,23] 
	uchar mday;  //	day of the month - [1,31] 
	uchar mon;   // months since January - [1,12] 
	uchar year;    // years since 2000 
	uchar wday;  // days since Sunday - [1,7] 
}Time, *pTime;
#endif

typedef struct SchRemainTime{
	uchar hour;
	uchar min;
	uint zone;
}SchReTime;

typedef struct SentWater_Struct{
	uint howlong;
	uint zone;	
	uchar type; //0表示即时喷淋，1手动喷淋，2喷淋计划,3传感器
	uchar adjust;//微调值
	Time add_time;//开始执行喷淋时间记录
	Time end_time;//结束喷淋时间记录
}WaterUsedStruct,*pWaterUsedStruct;

typedef struct Spray_Struct{
	uchar zone;//喷淋路数
	uchar adjust;//微调值
	uchar type; //0表示即时喷淋，1手动喷淋，2喷淋计划，3传感器
	uchar work_status;	//喷淋计划当前状态 0-未执行 1-正在执行 2-执行完成
	uint howlong;//喷淋时长(递减)
	uint src_time;//保留原设定时长
	Time add_time;//添加喷淋时间记录
}Spray,*pSpray;

typedef struct RainDelay_Struct{
	int delay_time;
	Time begin_time;
}RainDelay, *pRainDelay;

typedef struct FineTune_Struct{
		uchar minadjust[5];
		uchar adjust_mode;		//By Weather or by manual, 位表示，
										//从第五位开始，往低位，依次表示1~5天的微调模式
		Time  set_t;					//更新时间
}FineTune, *pFineTune;
	
typedef struct Weather_Struct{
	int tempminF;
	int tempmaxF;
	int tempminC;
	int tempmaxC;
	uchar raining;
	}Weather;
	
typedef struct node{
	void* data;
	struct node *next;
}SLink;

/************************************************************************/
/*                                           CMD						*/
/************************************************************************/
#define	ConnCloud				1		//云服务连接
#define	SendDevcodes			2		//设备上传序列码
#define	GetDevStatus			3		//获得设备关联状态
#define	SendZoneNum				4		//设备上传可用的路数编号(没启用)
#define  GetCommand				5		//获取app端指令
#define  SetDevParam			6		//设备设置参数
#define	GetDevParam				7		//获取设备参数
#define	UpDateDevParam			8		//修改设备参数
#define  GetUserInfo			9		//获取用户信息
#define	GetNewSoft				10		//设备端软件更新
// #define  GetSchF				11		//获得喷淋计划(以Flag为参数)
#define  GetDateTime			12		//设备获取服务器时间
#define	SetWater				13		//设备上传用水量
// #define	SetHumidity			14		//设备上传土壤湿度
#define  GetSchS				15		//获得喷淋计划(以ID为参数)
#define  GetDomain				16		//设备轮询域名
#define  SendDevStatus			17		//上传旋钮开关状态
#define  SendDevEvent			18		//上传喷淋开状态
#define  GetWeather				19		//获取天气状态
#define  SetRainDelay			20		//上传延迟喷淋
#define  SetSchWeeks			21		//上传喷淋计划-星期
#define  SetSchTimes			22		//上传喷淋计划-定时时间
#define  DeleteSch				23		//删除喷淋计划
#define  SetAdjust				24		//上传微调参数
#define  GetAdjust				25		//获取微调参数
#define  SentZoneTime			26		//发送路数时间
#define  SentSensor				27		//发送传感器值
#define  SchSetDays				28		//发送天设置
#define	 SetSch					29		//上传喷淋计划
#define  ReturnCid			    30      //返回CID
#define  SetMaster				31		//设置Master
#define  SetDevType				32		//发送设备型号
#define  CmdFree				33      //访问云端查看有无丢失命令  //16.8.17
#define  ReturnExecuted			34		//命令执行成功确认返回		//16.8.19


/************************************************************************/
/*                                   Mode	Select						*/
/************************************************************************/
#define	 SystemCheckMode			0
#define  AutoMode					4
#define  OffMode					5
#define	 ManuWaterMode				3
#define  Configration				6
#define  ScheduleView				1
#define  RainDelayView				2
#define  WeatherAdjust				8
#define  DeviceParam				9
#define  SpecialFeatures			7

/************************************************************************/
/*                                  Operate Mode						*/
/************************************************************************/
#define  Operate1			1
#define  Operate2			2
#define  Operate3			3
#define	 Operate4			4

/************************************************************************/
/*                                  Time Mode						*/
/************************************************************************/
#define  TimeMode_12		12
#define  TimeMode_24		24

/************************************************************************/
/*                                  EEPROM Addr							*/
/************************************************************************/
#define  IsHaveSensor			 0
#define  BackLightAddr			(IsHaveSensor + sizeof(char))
#define  TimeModeAddr			(BackLightAddr + sizeof(char))  //16.9.21 时间模式地址
#define  ServeAddr				(TimeModeAddr + sizeof(char))
#define  BandStatusAddr			(ServeAddr + sizeof(char) * LenOfServe)		//设备绑定状态 1个字节
#define  DevcodeAddr			(BandStatusAddr+sizeof(char))
#define  DevcodeFlagAddr		(DevcodeAddr +sizeof(char) * LenOfDevcodes)
#define  SerialCodeAddr			(DevcodeFlagAddr + sizeof(char))	//6个字节
#define  DevParamAddr			(SerialCodeAddr+sizeof(char)*LenOfSerialCode)	//设备参数地址(17个字节)
#define  RainDelayAddr			(DevParamAddr+sizeof(DevParam))
#define  ManuWaterSetAddr		(RainDelayAddr+sizeof(RainDelay))	//手动喷淋设置保存
#define  FineTuneAddr			(ManuWaterSetAddr + MaxValveManu*2)//微调设置保存
#define  DeleteSchAddr			(FineTuneAddr + sizeof(FineTune))	//记录删除的路数，共16路
#define	 EpScheduleHeadAddr		(DeleteSchAddr+ 6)                 //喷淋计划的首路数    //16.7.6 地址名称修改
#define  EpSchedule1Addr		(EpScheduleHeadAddr + 1)			//喷淋计划首地址

// #define	 ScheduleNumAddr		(DeleteSchAddr+ 6)	//喷淋计划个数地址
// #define	 Schedule1Addr			(ScheduleNumAddr + 1)	//喷淋计划首地址

#define  SparyHeadAddr			(EpSchedule1Addr + sizeof(EpSchedule) * (MaxValve+1))  
#define  SparyTagAddr			(SparyHeadAddr + 1)
#define  SparyFirstAddr			(SparyTagAddr + 1)

#define  WaterUsedHead			(SparyFirstAddr + sizeof(Spray)*(SprayEepromNum+1))
#define  WaterUsedTag			(WaterUsedHead + 1)
#define  WaterUsed1Addr			(WaterUsedTag + 1)


#define  SprayLimitTimeAddr		(E2END - 4)
#define	 FirstExeFlagAddr		(E2END-1)
#define  BootFlag				E2END

/************************************************************************/
/*                                   Net_Connect_Flag					*/
/************************************************************************/
#define  NetDisConnected			0
#define  NetConnected				1

/************************************************************************/
/*                                   Conn_Cloud_Flag					*/
/************************************************************************/
#define  DisConnCloud				0
#define  ConnCloudOk				1
#define  ConnCloudOff				2    //2016.9.16  

/************************************************************************/
/*                                   Send_Devcodes_Flag					*/
/************************************************************************/
#define  DisSendDevcodes			0
#define  SendDevcodesOk				1

/************************************************************************/
/*                                   User_Band_Flag						*/
/************************************************************************/
#define  DisBanded						0
#define  UserBandOk						1

/************************************************************************/
/*                                   Get_Time_Flag						*/
/************************************************************************/
#define  GetTimeFailed				1
#define	 GetTimeOK					0

/************************************************************************/
/*                                   WateringFlag						*/
/************************************************************************/
#define  WateringOFF					0
#define	 WateringStrat					1

/************************************************************************/
/*                                   Get_Param_Flag						*/
/************************************************************************/
#define  DisGetParam					1
#define	 GetParamOK						0

/************************************************************************/
/*                                   SentWater_Flag						*/
/************************************************************************/
#define  NoDataToSend					0
#define	 ReadyToSend					1

/************************************************************************/
/*                                   Updata_Display_Flag				*/
/************************************************************************/
#define  UpdataDisplayOk					0
#define	 UpdataDisplay						1

/************************************************************************/
/*                                   Get_Domain_Flag					*/
/************************************************************************/
#define  DisGetDomain						0
#define	 GetDomainOK						1

/************************************************************************/
/*                                   Send_work_value_Flag				*/
/************************************************************************/
#define  SentWorkMode						0
#define	 SentWorkModeOK						1

/************************************************************************/
/*                                   Send_Spary_Status_Flag				*/							 
/************************************************************************/
#define  SentSparyStatus					0
#define	 SentSparyStatusOK					1

/************************************************************************/
/*                                   sent_zone_time_flag				*/
/************************************************************************/
#define  NeedSentZoneTime					0
#define	 SentZoneTimeOK						1

/************************************************************************/
/*                                   sent_sensor_flag					*/									 
/************************************************************************/
#define  SentSensorStatusOk					0
#define	 SentSensorStatus					1

/************************************************************************/
/*                                   get_weather_flag					*/									 
/************************************************************************/
#define  NoGetWeather							0
#define	 GetWeatherOk							1

/************************************************************************/
/*                                   set_raindelay_flag					*/									
/************************************************************************/
#define  SetRainDelayOK						0
#define	 NeedSetRainDelay					1

/************************************************************************/
/*                                   isweather							*/
/************************************************************************/
#define  AdjustByManual						0
#define	 AdjustByWeather					1

/************************************************************************/
/*                                   set_adjust_flag		 			 */
/************************************************************************/
#define  SetAdjustOK							0
#define	 NeedSetAdjust							1

/************************************************************************/
/*                                   get_adjust_flag		 			*/
/************************************************************************/
#define  DisGetAdjust							0
#define	 GetAdjustOK								1
/************************************************************************/
/*                                   Get_Schedule_Flag					*/
/************************************************************************/
#define  GetScheduleOK					0
#define  UpdataAllSch					1

/************************************************************************/
/*                                   set_sch_weeks_flag					*/
/************************************************************************/
#define  SetSchOK								0
#define	 NeedSetSch								1

/************************************************************************/
/*                                   adjust_mode						*/
/************************************************************************/
#define  ByWeather									0
#define	 ByManual									1

/************************************************************************/
/*                                   sensor_value						*/
/************************************************************************/
#define  NeedSentSensor								0
#define	 SentSensorOk								1
#define  SensorOff									0
#define  SensorOn									1

/************************************************************************/
/*                                   sch_upload_status					*/
/************************************************************************/
#define  NoUpload									0
#define	 Uploading									1
#define  Uploaded									2

/************************************************************************/
/*                                   work_status						*/
/************************************************************************/
#define  NoExecute									0
#define	 Executing									1
#define  ExeCompleted								2

/************************************************************************/
/*                                   spray_value						*/
/************************************************************************/
#define KEY1								1	//zone1~4
#define KEY2								2  //zone5~8
#define KEY3								3  //zone9~12

/************************************************************************/
/*                                  return_cid						    */
/************************************************************************/
#define NeedReturnCid						1
#define ReturnCidOK						    2

/************************************************************************/
/*                                  return_executed					    */
/************************************************************************/
#define NeedReturnExecuted					1
#define ReturnExecutedOK					2


/************************************************************************/
/*                                  check_command					    */ //16.8.17
/************************************************************************/
#define NeedCheckCmd					1
#define CheckCmdOk						2

/************************************************************************/
/*                                   page		ID						*/
/************************************************************************/
#define  ConnNetPage				0
#define	 ConnCloudPage				1
#define  UserBandPage				2
#define  AutoModePage				3
#define  WaterAutoPage				4
#define  ScheduleMainPage			5
#define  ManuWaterPage				6
#define  ManuWaterStartPage			7
#define  RainDelayPage				8
#define  ConfigPage					9
#define  OffModePage				10
#define  FineTunePage				11
#define  DeviceParamPage			12
#define  SetupDateTimePage			13
#define  WifiAccessPage				14
#define  RegSmartDevPage			15
#define  SchSelValvePage			16
#define  SchSetTimePage				17
#define  SchSetWeeksPage			18
#define  SpecialFeatPage			19
#define  SchDelPage					20
#define  SchPlsSetTimePage			21
#define  SchPlsSetWeekPage			22
#define  SchNoPlanPage				23
#define  SchConfirmCoverPage		24
#define  MasterConfigPage			25
#define  SchSetDaysPage				26
#define  SchSelModePage				27
#define  SchSprayLimitPage			28
#define  ManuWaterLimitPage			29

/************************************************************************/
/*                                  Weeks								*/
/************************************************************************/
#define  MON			1
#define  TUES			2
#define  WED			3
#define  THURS			4
#define	 FRI			5
#define  SAT			6
#define  SUN			7

/************************************************************************/
/*                                  Config_Confirm_Key					*/
/************************************************************************/
#define  CANCEL						0
#define  CONFIRM					1

/************************************************************************/
/*                                  visit_cloud_time_flag				*/
/************************************************************************/
#define  Fast						Sec_3
#define  Slow						Sec_6
#define  VerySlow					Sec_10

/************************************************************************/
/*                                  Setup Date Time						*/
/************************************************************************/
#define  SetHour					0
#define  SetMin						1
#define  SetTimeMode				2
#define  SetWday					3
#define  SetMon						4
#define  SetMday					5
#define  SetYear					6


/************************************************************************/
/*                                  time interrupt						*/						
/************************************************************************/
#define  Sec_half							100
#define  Sec_1								200
#define  Sec_2								400
#define  Sec_3								600
#define  Sec_4								800
#define  Sec_5								1000
#define  Sec_6								1200
#define  Sec_8								1600
#define  Sec_7								1400
#define  Sec_10								2000
#define  Sec_15								3000

/************************************************************************/
/*                                       error type						*/					
/************************************************************************/
#define  SetDevcodeErr			5000
#define  SetnoUidErr			5102
#define  DevBoundedErr			5104
#define  GetWeatherErr			5105
#define  GetTimeUidErr			5200
#define  DevidErr				5800
#define  DevIsolatedErr			5801
#define  GetCdUidErr			5802
#define  GetStatusUidErr		6200
#define  GetDomainUidErr		6600
#define  GetWeatherUidErr		7001
#define  DelSchIDErr			7302
#define  SentSchErr				7501

/************************************************************************/
/*                                      喷淋类型  						*/
/************************************************************************/
#define	 SparyByApp			0
#define  SparyByManu		1
#define	 SparyBySch			2
#define  SparyBySensor		3
#define  SparyByDelay		4
#define  SparyUndef			5

/************************************************************************/
/*                                      ETH STATE						*/
/************************************************************************/
#define ETH_DISCONN		0
#define ETH_CONN		1

/************************************************************************/
/*                                      NET MODE						*/
/************************************************************************/
#define WIFI_MODE	 0
#define ETH_MODE	 1

/************************************************************************/
/*                                    SCHDEULE MODE						*/
/************************************************************************/
#define BY_WEEKS	 0
#define BY_DAYS		 1

/************************************************************************/
/*                                    GET DEVSTATUS						*/
/************************************************************************/
#define DisGet		0
#define OkGet		1

/************************************************************************/
/*                                    COMM FLAG							*/
/************************************************************************/
#define SENT_FAILED	 0
#define SENT_OK		 1

/************************************************************************/
/*                                  PROMGRAM ID							*/
/************************************************************************/
#define PROMGRAM_A	 1
#define PROMGRAM_B	 2
#define PROMGRAM_C	 3
#define PROMGRAM_D	 4

//extern char serve[LenOfServe];//服务器IP
extern char content[LenOfPath];//WEB地址
extern char devcode[LenOfDevcodes+1];	//设备序列号
extern char serialcode[LenOfSerialCode+1]; //设备随机码
extern char connuid[LenOfConnuid+1];//l链接码

extern volatile unsigned char webclient_flag;
extern volatile unsigned char work_value ;	//旋钮开关选择
extern volatile unsigned char operate_value ;//四个按键
extern volatile unsigned char spray_value ;//八个按键
extern volatile unsigned char sensor_value;	//传感器值

extern volatile unsigned int visit_cloud_time;//访问接口时间

extern DevParam devparam;//设备参数

extern Time volatile current_time;//当前时间

extern int version;//版本号

extern unsigned char net_mode;

extern unsigned int spray_time_limit;

extern int valves; //阀门路数

extern unsigned char timemode_flag;
#endif /* TYPEDEF_H_ */