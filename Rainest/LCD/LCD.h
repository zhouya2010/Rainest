/*
 * LCD.h
 *
 * Created: 2014/6/27 11:32:03
 *  Author: Administrator
 */ 


#ifndef LCD_H_
#define LCD_H_

//#include <avr/io.h>

/**
 * display normal or reverse
 *0 - normal
 *1- reverse
 **/
#define DISPALY_MOD 1

#define CS  PB0	//Chip select.    2PIN
#define RES PA6	//Reset pin 3PIN
#define RS PA7	//“H”: data. “L”: Instruction command. 4PIN
#define SCK PA4	//the serial clock input 13PIN
#define SID PA5	//serial data input 14PIN

#define uchar unsigned char
#define uint unsigned int

#define SID_H()	do{PORTA |= (1 << SID);}while(0)
#define SID_L()		do{PORTA &= ~(1 << SID);}while(0)
#define SCK_H()	do{PORTA |= (1 << SCK);}while(0)
#define SCK_L()	do{PORTA &= ~(1 << SCK);}while(0)
#define RS_H()		do{PORTA |= (1 << RS);}while(0)
#define RS_L()		do{PORTA &= ~(1 << RS);}while(0)
#define CS_H()		do{PORTB |= (1 << CS);}while(0)
#define CS_L()		do{PORTB &= ~(1 << CS);}while(0)
#define RES_H()	do{PORTA |= (1 << RES);}while(0)
#define RES_L()	do{PORTA &= ~(1 << RES);}while(0)

void LCD_clear(void);	//清屏
void LCD_init(void);//LCD初始化
void LCD_show_string(uchar page, uchar col, const char *Data);	//显示字符串
void LCD_show_stringR(uchar page, uchar col, const char *Data);	//显示字符串（反显）
void LCD_show_strP(uchar page, uchar col, const char *Data);	//使用PSTR方式显示字符串  如 LCD_show_stringP(1,0,PSTR("hello world"))；
void LCD_show_number(uchar page, uchar col, const uint Data);//显示数字
void LCD_show_numberR(uchar page, uchar col, const uint Data);//显示数字(反显)
void LCD_show_char(uchar page, uchar col, const char Data);//显示一个字符
void LCD_show_charR(uchar page, uchar col, const char Data);//显示一个字符(反显)
void LCD_show_icon(uchar page, uchar col, const char Data);//显示一个图标（6*8）
void LCD_show_number2(uchar page, uchar col, const uint Data, uchar len);//显示数字

void LCD_show_char916(uchar page, uchar col, const char Data );
void LCD_show_str916( uchar page, uchar col, const char *Data );
void LCD_show_num916( uchar page, uchar col, unsigned int Data );

#endif /* LCD_H_ */