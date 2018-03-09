/*
 * RS3231.h
 *
 * Created: 2014/5/14 19:16:12
 *  Author: ZLF
 */ 

#ifndef RS3231_H_
#define RS3231_H_

#include "typedef.h"

#define RTC_address 0x68

void RTC_init(void);

unsigned int RTC_second(void);
unsigned int RTC_minute(void);
unsigned int RTC_hour(void);
unsigned int RTC_day(void);
unsigned int RTC_date(void);
unsigned int RTC_month(void);
unsigned int RTC_year(void);

void RTC_time_get(void);
void RTC_time_set(Time time);

unsigned char BCD_uchar(unsigned char);
unsigned char uchar_BCD(unsigned char);

uchar get_last_mday(uchar year, uchar mon);
int DateDif(Time t_end,Time t_begin);
int MinDif(Time t_end,Time t_begin);

#endif /* RS3231_H_ */