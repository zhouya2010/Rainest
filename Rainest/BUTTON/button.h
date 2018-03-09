/*
 * button.h
 *
 * Created: 2014/6/3 16:17:35
 *  Author: ZLF
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_


// #define PA0 0
// #define PA1 1
// #define PA2 2
// #define PA3 3
// #define PA4 4
// #define PA5 5
// #define PA6 6
// #define PA7 7
// 
// #define PB0 0
// #define PB1 1
// #define PB2 2
// #define PB3 3
// #define PB4 4
// #define PB5 5
// #define PB6 6
// #define PB7 7
// 
// #define PC0 0
// #define PC1 1
// #define PC2 2
// #define PC3 3
// #define PC4 4
// #define PC5 5
// #define PC6 6
// #define PC7 7
// 
// #define PD0 0
// #define PD1 1
// #define PD2 2
// #define PD3 3
// #define PD4 4
// #define PD5 5
// #define PD6 6
// #define PD7 7

void button_init(void);
unsigned int button_detection(void);
unsigned char button_rotate(void);
unsigned char button_operate(void);
unsigned char button_spray(void);
void sensor_init(void);
unsigned char get_sensor_state(void);

#endif /* BUTTON_H_ */