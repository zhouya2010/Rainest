/*
 * btn_ctr.h
 *
 * Created: 2014/7/9 13:57:09
 *  Author: ZY
 */ 


#ifndef BTN_CTR_H_
#define BTN_CTR_H_

#include "typedef.h"


extern uint ManuValveTime[MaxValveManu];//手动模式8路定时时间
extern uint ManuRemaningTime;
extern uchar time_option;
extern Time tm_setup;
extern uchar timemode_setup;
extern uchar config_select;
extern uchar manu_sel;
extern Schedule new_sch;
extern uchar set_timer_option;
extern uchar weekday_sel;
extern uchar days_sel; 
extern uchar sch_id;
extern uchar plan_sel;
extern int rain_delay_temp;
extern uchar setting_delaytime_flag;
extern uchar minadjust_temp;
extern uchar show_success_flag;
extern uchar adjust_value_temp[5];
extern uchar adjust_days[5];
extern uchar adjust_sel;
extern FineTune weather_adjust;
extern uchar watering_flag;
extern uchar sch_mod_sel;

void btn_ctr( void ); //按键控制页面跳转以及其他操作
void weather_adjust_init(void);

#endif /* BTN_CTR_H_ */