/*
 * UI.h
 *
 * Created: 2014/6/30 14:02:01
 *  Author: ZY
 */ 


#ifndef UI_H_
#define UI_H_

#include "typedef.h"

extern  uchar updata_display_flag; //更新显示 0-不更新 1-更新
extern uchar CurrentPage;//当前显示页面ID
extern uchar NextPage;//下个显示页面ID
extern uchar MCDwarning_flag;

void UI_Display(void);//LCD显示控制

#endif /* UI_H_ */