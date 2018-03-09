/*
 * RS3231.c
 *
 * Created: 2014/5/14 19:15:47
 *  Author: ZLF
 */ 

#include "RTC.h"
#include "IIC.h"
#include <avr/io.h>

extern volatile Time current_time;

#define isLeapYear(year) (((year%4==0)&&(year%100!=0))||((year%4==0)&&(year%400==0)))

void RTC_init(void)
{
	DDRC |= (1 << PC0)|(1 << PC1);
	PORTC &= ~(1 << PC0)|(1 << PC1);
	RTC_time_get();
}

unsigned char BCD_uchar(unsigned char data)
{
	unsigned char target = 0;
	unsigned char ten = 0;
	unsigned char ge = 0;
	ge = data & 0x0F;
	ten = (data >> 4) & 0x0F;
	target = ten * 10 + ge;
	return target;
}

unsigned char uchar_BCD(unsigned char data)
{
	unsigned char target = 0;
	unsigned char ten = 0;
	unsigned char ge = 0;
	ten = data / 10;
	ge = data % 10;
	target = (ten << 4) + ge;
	return target;
}

unsigned int RTC_second(void)
{
	unsigned int second = 0;
	second = IIC_reg_read(RTC_address,0x00);
	second = BCD_uchar(second);
	return second;
}

unsigned int RTC_minute(void)
{
	unsigned int minute = 0;
	minute = IIC_reg_read(RTC_address,0x01);
	minute = BCD_uchar(minute);
	return minute;
}

unsigned char ampm = 0;
unsigned int RTC_hour(void)
{
	unsigned int hour = 0;
	hour = IIC_reg_read(RTC_address,0x02);
	hour = BCD_uchar(hour);
	return hour;
}

unsigned int RTC_day(void)
{
	unsigned int day = 0;
	day = IIC_reg_read(RTC_address,0x03);
	return day;
}

unsigned int RTC_date(void)
{
	unsigned int date = 0;
	date = IIC_reg_read(RTC_address,0x04);
	date = BCD_uchar(date);
	return date;
}

unsigned int RTC_month(void)
{
	unsigned int month = 0;
	month = IIC_reg_read(RTC_address,0x05);
	month = BCD_uchar(month);
	return month;
}

unsigned int RTC_year(void)
{
	unsigned int year = 0;
	year = IIC_reg_read(RTC_address,0x06);
	year = BCD_uchar(year);
	return year;
}

void RTC_time_get(void)
{
	current_time.sec = RTC_second();
	current_time.min = RTC_minute();
	current_time.hour = RTC_hour();
	current_time.wday = RTC_day();
	current_time.mday = RTC_date();
	current_time.mon = RTC_month();
	current_time.year = RTC_year();
}

void RTC_time_set(Time time)
{
	IIC_reg_write(RTC_address,0x00,uchar_BCD(time.sec));
	IIC_reg_write(RTC_address,0x01,uchar_BCD(time.min));
	IIC_reg_write(RTC_address,0x02,uchar_BCD(time.hour));
	IIC_reg_write(RTC_address,0x03,uchar_BCD(time.wday));
	IIC_reg_write(RTC_address,0x04,uchar_BCD(time.mday));
	IIC_reg_write(RTC_address,0x05,uchar_BCD(time.mon));
	IIC_reg_write(RTC_address,0x06,uchar_BCD(time.year));
}

uchar get_last_mday(uchar year, uchar mon)
{
	uint year_ture = year + 2000;
	uchar month12[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	if(mon >12) return 30;
	if(isLeapYear(year_ture))  month12[1]++;
	
	return month12[mon-1];
}

int YearToDay(uchar year)
{
	uint year_ture = year + 2000;
	int sum = 0;
	int i = 0;
	for (i=2000; i < year_ture; i++)
	{
		sum = sum + 365 + isLeapYear(i);
	}
	return sum;
}

int MonthToDay(uchar year,uchar mon)
{
	uint year_ture = year + 2000;
	int sum = 0;
	int i  = 0;
	for (i=1; i<mon; i++)
	{
		sum = sum + get_last_mday(year_ture,i);
	}
	return sum;
}

int DateToDay(Time t)
{
	int sum = YearToDay(t.year) + MonthToDay(t.year, t.mon) + t.mday;
	return sum;
}

/*
返回两个日期之间间隔天数
*/
int DateDif(Time t_end,Time t_begin)
{
	return (DateToDay(t_end) - DateToDay(t_begin));
}

/*
返回两个日期之间间隔分钟数
*/

int MinDif(Time t_end,Time t_begin)
{
	int day_dif = 0;
	int m_dif = 0;
	int m_end = 0;
	int m_begin = 0;
	
	day_dif = DateDif(t_end,t_begin);
	if (day_dif < 0)	return -1;
	
	m_end =day_dif*24*60 + t_end.hour*60 + t_end.min;
	m_begin = t_begin.hour*60 + t_begin.min;
	m_dif = m_end - m_begin;
	if(m_dif < 0)
	{
		 return -1;
	}
	else
	{
		return m_dif;
	}
}
