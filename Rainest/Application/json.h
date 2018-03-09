/*
 * json.h
 *
 * Created: 2014/6/4 10:20:01
 *  Author: ZY
 */ 


#ifndef JSON_H_
#define JSON_H_

#include "typedef.h"
#include "schedule_execute.h"

extern Weather weather;
extern pSchedule sch_upload;
extern uchar delete_zone;
extern const char  urlhead[];

void parse_cloud_API(const char *data ,const unsigned int len);//解析JSON指令入口
int Js_parser_array( char *res, const char *buffer, char *key);//获取JSON指令中的数组
int Js_parser_value_int( int *res, const char *buffer, char *key);//获取JSON指令中的整数
int Js_parser_value_char( unsigned char *res,const char *buffer, char *key);//获取JSON指令中的整数
int Js_parser_string(char *res,const char *buffer, char *key);//获取JSON指令中的字符串
unsigned long prarser_time(char * const buffer,char *key);//获取JSON指令中的云端时间
Time prarser_datetime(char * const buffer,char *key );//获取JSON指令中的云端时间
unsigned int tobit(char * const whichvalve);//将喷淋路数由数组表示转换为16位位表示
int parser_open_valve(char * const buffer);//解析开水阀命令
int parser_close_valve(char * const buffer);//解析关水阀命令
int parser_schedule(Schedule *psch, char * const buffer );//解析喷淋计划命令
int parser_dev_param(pDevParam devparam, char * const buffer );//解析设备参数
void setw_handle( void );//上传用水量成功后处理
void get_cmd( unsigned char flag);//获取访问云端接口指令
void get_cid(char * const data);

#endif /* JSON_H_ */