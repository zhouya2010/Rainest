/*
 * IIC.h
 *
 * Created: 2014/5/14 18:58:30
 *  Author: ZLF
 */ 


#ifndef IIC_H_
#define IIC_H_


void IIC_SDA_H(void);
void IIC_SDA_L(void);
void IIC_SCK_H(void);
void IIC_SCK_L(void);
void IIC_clock(void);
void IIC_start(void);
void IIC_write(unsigned char);
unsigned char IIC_read(void);
void IIC_reg_write(unsigned char,unsigned char,unsigned char);
char IIC_reg_read(unsigned char,unsigned char);
unsigned char IIC_response(void);
void IIC_not_response(void);
void IIC_stop(void);


#endif /* IIC_H_ */