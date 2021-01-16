//*****************************************************************************
//  File Name: LCD_KEY.H
//  Function:  KT Wireless Mic Transmitter Products Demoboard 
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V1.0    2012-08-01  Initial draft
//  V1.1    2017-02-10  规范化整理
//*****************************************************************************
#ifndef LCD_KEY_H
#define LCD_KEY_H

//-----------------------------------------------------------------------------
//LCD引脚定义
//-----------------------------------------------------------------------------
sbit LCD_RS = P1^5; // RS=1：数据； RS=0：指令
sbit LCD_RST = P1^6; // 低电平复位
sbit LCD_CS = P1^7; //片选
sbit LCD_SCL = P1^3; // 
sbit LCD_SDA = P1^4; //

//rom的连线用内部弱上拉的话需要在读写函数中适当的delay，现在的程序就是这样，不需
//要外部上拉，如果用推挽的话，可以把一些delay去掉，但是ROM_OUT不能用推挽
sbit Rom_IN=P0^7; /*字库IC 接口定义:Rom_IN 就是字库IC 的SI*/
sbit Rom_OUT=P1^0; /*字库IC 接口定义:Rom_OUT 就是字库IC 的SO*/
sbit Rom_SCK=P1^1; /*字库IC 接口定义:Rom_SCK 就是字库IC 的SCK*/
sbit Rom_CS=P1^2; /*字库IC 接口定义Rom_CS 就是字库IC 的CS#*/

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
void display_ascii_string(UINT8 x,UINT8 y,UINT8 *text);
void display_graphic_8x16(UINT8 page,UINT8 column,UINT8 *dp);
void display_graphic_32x32(UINT8 page,UINT8 column,UINT8 *dp);
void clear_screen();

#endif
