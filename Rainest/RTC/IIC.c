/*
 * IIC.c
 *
 * Created: 2014/5/14 18:58:14
 *  Author: ZLF
 */ 

#include <avr/io.h>
#include "IIC.h"

#ifndef F_CPU
#define F_CPU              12000000UL
#endif
#include <util/delay.h>

void IIC_SCK_H(void){PORTC |= (1 << PC0);}	//时钟信号高
void IIC_SCK_L(void){PORTC &= ~(1 << PD0);}	//时钟信号低
void IIC_SDA_H(void){PORTC |= (1 << PC1);}	//数据信号高
void IIC_SDA_L(void){PORTC &= ~(1 << PC1);}	//数据信号低

/************************************************************************/
/*Description:一个I2C时钟周期                                            */
/************************************************************************/
void IIC_clock(void)
{
	_delay_us(2);
	IIC_SCK_H();
	_delay_us(2);
	IIC_SCK_L();
}

/************************************************************************/
/*Description:I2C的起始信号                                              */
/************************************************************************/
void IIC_start(void)
{
	IIC_SDA_H();
	IIC_SCK_H();
	_delay_us(8);
	IIC_SDA_L();
	_delay_us(8);
	IIC_SCK_L();
	_delay_us(8);
}

/************************************************************************/
/*Description:I2C的结束信号                                                  */
/************************************************************************/
void IIC_stop(void)
{
	IIC_SDA_L();
	IIC_SCK_H();
	_delay_us(8);
	IIC_SDA_H();
	_delay_us(8);
}

/************************************************************************/
/*Description:I2C的应答信号                                              */
/*Back:有应答返回0，无应答返回1                                           */
/************************************************************************/
unsigned char IIC_response(void)	//应答信号
{
	unsigned char flag = 0;
	DDRC &= ~(1 << PC1);
	if(PINC & 0x02)
	{
		flag = 1;
	}
	else
	{
		IIC_SCK_H();
		_delay_us(2);
		IIC_SCK_L();
		_delay_us(2);
	}
	DDRC |= (1 << PC1);
	return flag;
}

/************************************************************************/
/*Description:一个周期内I2C不响应                                        */
/************************************************************************/
void IIC_not_response(void)
{
	IIC_SDA_H();
	_delay_us(2);
	IIC_clock();
}

/************************************************************************/
/*Description:I2C的写字节操作                                            */
/*Parameter:待写入的字节                                                 */
/************************************************************************/
void IIC_write(unsigned char temp)
{
	unsigned char i = 0;
	unsigned char dat = 0;
	
	dat = temp;
	for(i = 0;i < 8;i++)
	{
		if(dat & 0x80)
		{
			IIC_SDA_H();
		}
		else
		{
			IIC_SDA_L();
		}
		IIC_clock();
		
		dat = dat << 1;
	}
	IIC_SDA_H();
	
	while(IIC_response());
}

/************************************************************************/
/*Description:I2C读一个字节                                              */
/*Back:读到的字节                                                        */
/************************************************************************/
unsigned char IIC_read(void)
{
	unsigned char i = 0;
	char dat = 0;
	
	DDRC &= ~(1 << PC1);
	for(i = 0;i < 8;i++)
	{
		dat = dat << 1;
		IIC_SCK_H();
		if(PINC & 0x02)
		{
			dat |= 0x01;
		}
		else
		{
			dat &= 0xFE;
		}
		IIC_SCK_L();
		_delay_us(2);
	}
	DDRC |= (1 << PC1);
	return dat;
}

/************************************************************************/
/*Description:对指定器件的指定寄存器写一个指定的值，适用345或5883          */
/*Parameter:设备地址，寄存器地址，寄存器的数据                             */
/************************************************************************/
void IIC_reg_write(unsigned char address,unsigned char reg,unsigned char dat)
{
	IIC_start();
	IIC_write(address << 1);
	IIC_write(reg);
	IIC_write(dat);
	IIC_stop();
}

/************************************************************************/
/*Description:读指定器件的一个寄存器值，适用345或5883                     */
/*Parameter:设备地址，寄存器地址                                          */
/*Back:读到的值                                                          */
/************************************************************************/
char IIC_reg_read(unsigned char address,unsigned char reg)
{
	char dat = 0;
	
	IIC_start();
	IIC_write(address << 1);
	IIC_write(reg);
	IIC_start();
	IIC_write((address << 1)|(0x01));
	dat = IIC_read();
	//dat = IIC_read();
	IIC_not_response();
	IIC_stop();
	return dat;
}