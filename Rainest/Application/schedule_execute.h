/*
 * schedule_execute.h
 *
 * Created: 2014/7/11 15:42:16
 *  Author: ZY
 */ 


#ifndef SCHEDULE_EXECUTE_H_
#define SCHEDULE_EXECUTE_H_

#include "typedef.h"


void init_list( SLink **L);//链表初始化

void updata_sch( SLink *L, Schedule NewSch );//更新喷淋计划
void insert_sch(SLink *L, Schedule sch);//插入喷淋计划 
void del_sch( SLink *L, uchar zone );//删除喷淋计划
void del_all_sch(SLink *L);// 删除整个喷淋计划
int locate_zone(SLink *L,uchar x);	//获取路数为x的位置
pSchedule get_upload_sch_addr( SLink *L );	//获取需要上传的喷淋计划地址
void sch_inquire(SLink *L,Time tm);//查看喷淋计划是否需要执行

void start_spray( uchar zones, uint howlong );	//喷淋开


void add_spray(SLink *L, Spray spr);
void del_spray(SLink *L, uchar zone);
int locate_spray(SLink *L,uchar zone);
void clean_spray(SLink *L);

void del_first_water_used(SLink *L);
void del_last_water_used(SLink *L);
void add_water_used(SLink *L, WaterUsedStruct wus);

SLink* get_addr( SLink *L,int i );//获取位置为i的元素指针
int get_length( SLink *L );//获取链表长度
void clear_link(SLink *L);
int ins_elem( SLink *L, void * data, int i );
int schedule_is_legal(Schedule sch);
#endif /* SCHEDULE_EXECUT_H_ */