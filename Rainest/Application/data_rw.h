/*
 * data_rw.h
 *
 * Created: 2014/7/9 20:26:53
 *  Author: ZY
 */ 


#ifndef DATA_READ_H_
#define DATA_READ_H_

#include "typedef.h"
#include <stdint.h>

extern DevParam devparam;//设备参数
extern SLink *sch_link;
extern RainDelay rain_delay;

void read_param(void);//读取参数
void get_code(void);//获得设备码和随机码
//void read_schedule(void);//读取喷淋计划            //16.7.6 不再使用
//void save_schedule( SLink *L );//保存喷淋计划
//void reset_sch_num(void);//重置喷淋计划个数
void get_devcode(char * code);//获取SIN
void get_serialcode( char * code );//获取随机码
void save_band_status(void);//保存用户绑定状态
pWaterUsedStruct get_water_used(SLink *L);//获取所有路数中相同的最大用水量
void reset_water_used_eeprom( uchar i );//清零某路用水量
void save_raindelay( void );//保存延迟时间
void save_manuwater_set(void);//保存手动设置数据
void recovery_system(void);//恢复出厂设置
void save_finetune( FineTune ft);//保存微调设置
void read_finetune(void);//读取微调设置
//void update_schedule(uchar i,Schedule NewSch);   //16.7.6 不再使用
uchar get_del_zone(void);
void set_del_zone(uchar zone);
void reset_del_zone(uchar zone);
void save_devparam( void );
void reset_band_status( void );

void eeprom_read_water_used(SLink *L);
void eeprom_write_water_used(SLink *L, WaterUsedStruct wus);
void save_all_water_used(SLink *L);//保存用水量到EEPROM
void get_mac(unsigned char *mac);
void save_mac( char *mac );
unsigned char get_mac_flag(void);
void set_backlight_flag(unsigned char data);
int get_backlight_flag(void);
void set_timemode_flag(unsigned char timemode);
void get_timemode_flag(unsigned char * timemode);
int get_version_id(void);
void get_vesion_str(char *res) ;
void save_domain(char *serve);
void read_daomain(char *dest);
void read_spray_time_limit(void);
void save_spray_time_limit(uint16_t howlong);
void save_spray_link( SLink *L );
void eep_update_spray(Spray spr);
void eep_spray_delete_first(void);
void eep_save_spray(Spray spray);       //16.7.7  implicit declaration of function 'eep_save_spray'
void set_mac_flag(unsigned char flag);  //16.7.7
  
int add_epschedule(SLink *L,Schedule sch_new);                       //保存喷淋计划            //16.7.6 新添加
int updata_epschedule(SLink *L,Schedule sch_new); //更新eeprom喷淋计划数据
void insert_epschedule(SLink *L,Schedule sch_new);  //插入或更新eeprom喷淋计划
int delete_epschedule(SLink * L,unsigned char zone);//删除eeprom喷淋计划数据
void read_epschedule(void);							//读取eeprom喷淋计划数据
void reset_epschedule(void);						//清空eeprom喷淋计划
#endif /* DATA_READ_H_ */