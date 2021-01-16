//*********************************************************************
//  File Name: interface.h
//  Function:  KT Wireless Mic Transmitter Products Handheld Demoboard
//*********************************************************************
//        Revision History
//  Version	Date		Description
//  V1.0	2012-08-01	Initial draft

#include "c8051f310.h"										// SFR declarations

#define uchar unsigned char
#define ulong unsigned long
#define uint unsigned int

#define I2C

//------------------------------------------------------------------------------------
//I2C引脚定义
//------------------------------------------------------------------------------------
sbit SDA		=	P1^2;											// 分配P1.2给SDA
sbit SCL		=	P1^3;											// 分配P1.3给SCL

//------------------------------------------------------------------------------------
//关机控制引脚定义
//------------------------------------------------------------------------------------
sbit LDO_CTRL	=	P0^0;											// 控制开关机POP声
sbit VBAT_IN	=	P0^1;											// 控制开关机POP声

//------------------------------------------------------------------------------------
//LCD引脚定义
//------------------------------------------------------------------------------------
sbit LCD_LED	=	P0^3;										// LCD 背光
sbit LCD_WR		=	P0^4;										// LCD 控制
sbit LCD_DATA	=	P0^5;										// LCD 数据
sbit LCD_CS		=	P0^6;										// LCD 片选

//------------------------------------------------------------------------------------
//KEY引脚定义
//------------------------------------------------------------------------------------
sbit Key_UP		=	P0^7;										// 频率向上控制
sbit Key_DOWN	=	P1^0;										// 频率向下控制
sbit Key_SET	=	P1^1;										// 频率设定
sbit RF_POW_SW	=	P3^2;										// 射频大小控制

// ------------------------LCD Commands Definition -------------------------------
#define RD_MODE		0xC0	// 110 Binary
#define WR_MODE 	0xA0	// 101
#define RMW_MODE	0xA0	// 101
#define CMD_MODE	0x80	// 100

#define LCD_COM		4
#define LCD_SEG		32

#define IC_COM		4
#define IC_SEG		32

#define SYS_EN		0x01	// Enable System Clock
#define SYS_DIS		0x00

#define LCD_ON		0x03	// Turn on LCD
#define LCD_OFF		0x02	// Turn off LCD

#define TIMER_DIS	0x04
#define TIMER_EN	0x06
#define TIMER_CLS	0x0C

#define WDT_DIS		0x05
#define WDT_EN		0x07
#define WDT_CLS		0x0E

#define TONE_OFF	0x08
#define TONE_ON		0x09

#define CRYSTAL_32K	0x14
#define INT_256K	0x18
#define EXT_256K	0x1C

#define LCD_BIAS	0x29	// 1/3 bias, 4coms used
//#define LCD_BIAS	0x28	// 1/2 bias, 4coms used

#define TONE_2K		0X60
#define TONE_4K		0X40

#define IC_IRQ_DIS	0x80
#define IC_IRQ_EN	0x88

// -----------------------------------------------------------------------------
#define FREQ_ZERO_A		0x09
#define FREQ_ZERO_B		0x09
#define FREQ_ZERO_C 	0x00
#define FREQ_ZERO_D 	0x03/*0*/

#define FREQ_ONE_A 		0x00
#define FREQ_ONE_B 		0x00
#define FREQ_ONE_C 		0x00
#define FREQ_ONE_D 		0x03/*1*/

#define FREQ_TWO_A 		0x0A
#define FREQ_TWO_B 		0x09
#define FREQ_TWO_C 		0x02
#define FREQ_TWO_D 		0x01/*2*/

#define FREQ_THREE_A	0x00
#define FREQ_THREE_B	0x09/*3*/
#define FREQ_THREE_C	0x02
#define FREQ_THREE_D	0x03/*3*/

#define FREQ_FOUR_A 	0x03
#define FREQ_FOUR_B 	0x00/*4*/
#define FREQ_FOUR_C		0x02
#define FREQ_FOUR_D		0x03/*4*/

#define FREQ_FIVE_A 	0x03
#define FREQ_FIVE_B 	0x09/*5*/
#define FREQ_FIVE_C 	0x02
#define FREQ_FIVE_D 	0x02/*5*/

#define FREQ_SIX_A 		0x0B
#define FREQ_SIX_B 		0x09/*6*/
#define FREQ_SIX_C 		0x02
#define FREQ_SIX_D 		0x02/*6*/

#define FREQ_SEVEN_A 	0x00
#define FREQ_SEVEN_B 	0x01/*7*/
#define FREQ_SEVEN_C 	0x00
#define FREQ_SEVEN_D 	0x03/*7*/

#define FREQ_EIGHT_A 	0x0B
#define FREQ_EIGHT_B 	0x09/*8*/
#define FREQ_EIGHT_C 	0x02
#define FREQ_EIGHT_D 	0x03/*8*/

#define FREQ_NINE_A 	0x03
#define FREQ_NINE_B 	0x09/*9*/
#define FREQ_NINE_C 	0x02
#define FREQ_NINE_D 	0x03/*9*/

//#define MHZ				((22<<4) | 0x08)
#define LCD_PILOT			((29<<4) | 0x08)
#define LCD_MUTE			((25<<4) | 0x08)
#define LCD_UNMUTE			((25<<4) | 0x00)
#define LCD_LOCK			((18<<4) | 0x08)
#define LCD_DOT				((11<<4) | 0x04)

#define LCD_BATH 			((30<<4) | 0x0F)
#define LCD_BATM 			((30<<4) | 0x0D)
#define LCD_BATL 			((30<<4) | 0x09)
#define LCD_BATZ 			((30<<4) | 0x01)

#define RF_POWER_L0		((2<<4) | 0x08)

#define RF_POWER_L1_A	((2<<4) | 0x08)
#define RF_POWER_L1_B	((3<<4) | 0x04)

#define RF_POWER_L2_A 	((2<<4) | 0x08)
#define RF_POWER_L2_B	((3<<4) | 0x0C)

#define RF_POWER_L3_A	((2<<4) | 0x08)
#define RF_POWER_L3_B	((3<<4) | 0x0C)
#define RF_POWER_L3_C	((10<<4) | 0x08)

#define RF_POWER_L4_A	((2<<4) | 0x08)
#define RF_POWER_L4_B	((3<<4) | 0x0C)
#define RF_POWER_L4_C	((10<<4) | 0x08)
#define RF_POWER_L4_D	((11<<4) | 0x08)

#define RF_POWER_L5_A	((2<<4) | 0x08)
#define RF_POWER_L5_B	((3<<4) | 0x0C)
#define RF_POWER_L5_C	((10<<4) | 0x08)
#define RF_POWER_L5_D	((11<<4) | 0x08)
#define RF_POWER_L5_E	((14<<4) | 0x08)

#define RF_POWER_L6_A	((2<<4) | 0x08)
#define RF_POWER_L6_B	((3<<4) | 0x0C)
#define RF_POWER_L6_C	((10<<4) | 0x08)
#define RF_POWER_L6_D	((11<<4) | 0x08)
#define RF_POWER_L6_E	((14<<4) | 0x08)
#define RF_POWER_L6_F	((15<<4) | 0x04)

#define RF_POWER_L7_A	((2<<4) | 0x08)
#define RF_POWER_L7_B	((3<<4) | 0x0C)
#define RF_POWER_L7_C	((10<<4) | 0x08)
#define RF_POWER_L7_D	((11<<4) | 0x08)
#define RF_POWER_L7_E	((14<<4) | 0x08)
#define RF_POWER_L7_F	((15<<4) | 0x0C)

#define CH			((22<<4) | 0x08)

#define CH_ZERO_A		0x0F       
#define CH_ZERO_B 		0x05/*0*/
                               
#define CH_ONE_A 		0x06       
#define CH_ONE_B 		0x00/*1*/  
                               
#define CH_TWO_A 		0x0B       
#define CH_TWO_B 		0x06/*2*/  
                               
#define CH_THREE_A 		0x0F     
#define CH_THREE_B 		0x02/*3*/
                               
#define CH_FOUR_A 		0x06     
#define CH_FOUR_B 		0x03/*4*/
                               
#define CH_FIVE_A 		0x0D     
#define CH_FIVE_B 		0x03/*5*/
                               
#define CH_SIX_A 		0x0D       
#define CH_SIX_B 		0x07/*6*/  
                               
#define CH_SEVEN_A 		0x07     
#define CH_SEVEN_B 		0x00/*7*/
                               
#define CH_EIGHT_A 		0x0F     
#define CH_EIGHT_B 		0x07/*8*/
                               
#define CH_NINE_A 		0x0F     
#define CH_NINE_B 		0x03/*9*/


#ifdef I2C
void I2C_Word_Write(uchar device_address, uchar reg_add, uint writeword);
uint I2C_Word_Read(uchar device_address, uchar reg_add);
void I2C_Byte_Write(uchar device_address, uchar reg_add, uchar writedata);
uchar I2C_Byte_Read(uchar device_address, uchar reg_add);
#endif

//------------------------------------------------------------------------------------
//LCD显示部分函数定义
//------------------------------------------------------------------------------------
uchar Key_Scan (void);
void Delay_ms(uint iTime_Number);
void Delay_us(uchar iTime_Number);

void LCD_Init(void);

void Scan_segs(uchar dByte, uint Tscan);
void SendBit_1621(uchar cSenddata, uchar cCounter);
void SendComBit_1621(uchar cSenddata); //data 的低cnt 位寫入HT1621，低位在前
void WriteFix_1621(uint iSenddata);
void Write_1621(uchar cAddr,uchar cCom);

void Display_Freq_Num(uchar cAddress,uchar cNum);
void Display_Frequency_and_RFPower(long lFreq);
void RF_Power(uchar cPower_Level);
//void Display_Ch_Num(uchar address, uchar num);
