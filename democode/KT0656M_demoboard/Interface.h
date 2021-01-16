//*****************************************************************************
//  File Name: interface.h
//  Function:  KT Wireless Mic Receiver Products Demoboard
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V1.0    2012-08-28  Initial draft
//  V1.1    2013-11-22  改变变量类型定义方法
//  V1.2    2017-02-10  规范化整理
//*****************************************************************************
#ifndef INTERFACE_H
#define INTERFACE_H

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <c8051f340.h>

//-----------------------------------------------------------------------------
//常用数据类型定义
//-----------------------------------------------------------------------------
typedef unsigned char    UINT8;
typedef unsigned int     UINT16;
typedef unsigned long    UINT32;
typedef char             INT8;
typedef int              INT16;
typedef long             INT32;
typedef bit              BOOL;

//-----------------------------------------------------------------------------
//通道参数定义
//-----------------------------------------------------------------------------
#define CH_A    0
#define CH_B    1

//sbit chip_en_A_M  =    P2^3;

//-----------------------------------------------------------------------------
//关机控制引脚定义
//-----------------------------------------------------------------------------
sbit VBAT_IN    = P3^7;

sbit LDO_CTRL   = P3^6;
#define LDO_CTRL_WRITE0() LDO_CTRL=0
#define LDO_CTRL_WRITE1() LDO_CTRL=1

//-----------------------------------------------------------------------------
//MUTE引脚定义
//-----------------------------------------------------------------------------
sbit Mute_A    =    P3^2; // A路mute控制
sbit Mute_B    =    P3^3; // B路mute控制

//-----------------------------------------------------------------------------
//KEY引脚定义
//-----------------------------------------------------------------------------
sbit Key_UP_A    =    P2^0; // 向上控制
sbit Key_SET_A   =    P2^1; // 设定
sbit Key_DOWN_A  =    P2^2; // 向下控制

sbit Key_UP_B    =    P0^4; // 向上控制
sbit Key_SET_B   =    P0^3; // 设定
sbit Key_DOWN_B  =    P0^2; // 向下控制

sbit CHIP_EN_AM  = 	  P2^3;
sbit CHIP_EN_AS  = 	  P3^5;
sbit CHIP_EN_BM  = 	  P3^4;
sbit CHIP_EN_BS  = 	  P3^1;

//-----------------------------------------------------------------------------
//LCD Commands Definition
//-----------------------------------------------------------------------------
#define RD_MODE     0xC0 // 110 Binary
#define WR_MODE     0xA0 // 101
#define RMW_MODE    0xA0 // 101
#define CMD_MODE    0x80 // 100

#define LCD_COM     4
#define LCD_SEG     32

#define IC_COM      4
#define IC_SEG      32

#define SYS_EN      0x01 // Enable System Clock
#define SYS_DIS     0x00

#define LCD_ON      0x03 // Turn on LCD
#define LCD_OFF     0x02 // Turn off LCD

#define TIMER_DIS   0x04
#define TIMER_EN    0x06
#define TIMER_CLS   0x0C

#define WDT_DIS     0x05
#define WDT_EN      0x07
#define WDT_CLS     0x0E

#define TONE_OFF    0x08
#define TONE_ON     0x09

#define CRYSTAL_32K 0x14
#define INT_256K    0x18
#define EXT_256K    0x1C

#define LCD_BIAS    0x29 // 1/3 bias, 4coms used
//#define LCD_BIAS    0x28 // 1/2 bias, 4coms used

#define TONE_2K     0X60
#define TONE_4K     0X40

#define IC_IRQ_DIS  0x80
#define IC_IRQ_EN   0x88

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80
//-----------------------------------------------------------------------------
//LCD显示部分函数定义
//-----------------------------------------------------------------------------
UINT8 Key_Scan (void);
void Delay_ms(UINT16 iTime_Number);
void Delay_us(UINT8 iTime_Number);
void LCD_Init(void);
void Display_Freq_Num(UINT8 cAddress,UINT8 cNum);
void RF_Power(UINT8 cPower_Level);
void Audio_Lever(UINT8 cAF_Level);
void Display_Ch_Num(UINT8 address, UINT8 num);

#endif

