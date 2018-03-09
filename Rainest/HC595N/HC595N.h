/*
 * _74HC595N.h
 *
 * Created: 2014/5/14 13:18:55
 *  Author: ZLF
 */ 


#ifndef HC595N_H_
#define HC595N_H_

#include "typedef.h"

#define HC595_DAT PC5
#define HC595_EN1 PB1
#define HC595_EN2 PB2
#define HC595_SH PC6
#define HC595_ST PB3

extern long long spray_state;               //16.7.5
extern long long spray_queue_state;
extern long long spray_array_state_temp;
extern long long spray_array_state;

void HC595N_init(void);
void spray_open(void);
void spray_close_all(SLink *L);
void spray_detection(SLink * L);
void spray_close_zone(SLink *L, uchar zone);
void add_spray_to_waterused(pSpray pspr);

#endif /* 74HC595N_H_ */