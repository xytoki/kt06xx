//*****************************************************************************
//  File Name: interface.h
//  Function:  KT Wireless Mic Transmitter Products Demoboard Interface Function Define
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V1.0    2015-8-27   初始版本
//  V1.1    2017-02-08  格式规范化整理
//  V0.3    2017-04-27  加减频率的时候，先把导频关了，tune完台后再恢复原来导频的设置（main.c）
//						调台中一直以相同的步进进行tune台，不会调一段时间后步进变成4倍和10倍（main.c）
//*****************************************************************************

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "sh88f4051.h" 
#include "intrins.h"
//#include "IR.h"

//-----------------------------------------------------------------------------
// 功能及参数定义
//-----------------------------------------------------------------------------
#define I2C //I2C功能定义

#ifdef I2C
    #define I2C_WORD_MODE
#endif

//#define IR        //红外对频
//#define SOUTAI        //自动搜台对频

#ifdef IR
    #define    MIC      MIC_B
    #define    MIC_A    0xA0
    #define    MIC_B    0xB0
#endif

#ifdef SOUTAI
    //自动搜台对频公共频率可修改，但是幅度不能太大，并且需要和RX一致
    #define MIC_Frequency     645350    
#endif

//-----------------------------------------------------------------------------
//常用数据类型定义
//-----------------------------------------------------------------------------
typedef unsigned char   UINT8;
typedef unsigned int    UINT16;
typedef unsigned long   UINT32;
typedef char            INT8;
typedef int             INT16;
typedef long            INT32;
typedef bit             BOOL;

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
static UINT32 xdata Memery_Frequency; //保存频道信息 
static UINT32 xdata Load_Frequency;   //flash里面保存的频率信息
//static UINT8 VOLUME = 6;
static UINT8 Key_UP_flag=0;    //按键长按快速往上加标志状态
static UINT8 Key_DOWN_flag=0;  //按键长按快速往下减标志状态
static BOOL Key_RF_POW_flag=1; //功率切换是否去抖标志
//static BOOL temp_RF_POW_SW=0;

//-----------------------------------------------------------------------------
//定义VOLUME 参数
//-----------------------------------------------------------------------------
#define    key_Vol     0
#define    key_Freq    1

#define    Dis_None    0
#define    Dis_Freq    1
#define    Dis_Vol     2

#define    VOLUME_TOP       15 //volume=15
#define    VOLUME_BOTTOM    0  //volume=0

#define    VBAT_FULL       0x190 //充满电标准值，具体效果可自行调节
#define    VBAT_LOW        0x177 //低电压报警值，具体效果可自行调节

#define    LED_ON        0
#define    LED_OFF       1

//-----------------------------------------------------------------------------
//定义LCD & KEY参数
//-----------------------------------------------------------------------------
#define RD_MODE     0xC0    // 110 Binary
#define WR_MODE     0xA0    // 101
#define RMW_MODE    0xA0    // 101
#define CMD_MODE    0x80    // 100

#define LCD_COM        4
#define LCD_SEG        32

#define SYS_EN        0x01    // Enable System Clock
#define SYS_DIS       0x00

#define LCD_ON        0x03    // Turn on LCD
#define LCD_OFF       0x02    // Turn off LCD

#define CRYSTAL_32K    0x14
#define INT_256K    0x18
#define EXT_256K    0x1C

#define LCD_BIAS    0x29    // 1/3 bias, 4coms used
//#define LCD_BIAS    0x28    // 1/2 bias, 4coms used

#define FREQ_ZERO_A     0x09
#define FREQ_ZERO_B     0x09
#define FREQ_ZERO_C     0x00
#define FREQ_ZERO_D     0x03/*0*/

#define FREQ_ONE_A      0x00
#define FREQ_ONE_B      0x00
#define FREQ_ONE_C      0x00
#define FREQ_ONE_D      0x03/*1*/

#define FREQ_TWO_A      0x0A
#define FREQ_TWO_B      0x09
#define FREQ_TWO_C      0x02
#define FREQ_TWO_D      0x01/*2*/

#define FREQ_THREE_A    0x00
#define FREQ_THREE_B    0x09/*3*/
#define FREQ_THREE_C    0x02
#define FREQ_THREE_D    0x03/*3*/

#define FREQ_FOUR_A     0x03
#define FREQ_FOUR_B     0x00/*4*/
#define FREQ_FOUR_C     0x02
#define FREQ_FOUR_D     0x03/*4*/

#define FREQ_FIVE_A     0x03
#define FREQ_FIVE_B     0x09/*5*/
#define FREQ_FIVE_C     0x02
#define FREQ_FIVE_D     0x02/*5*/

#define FREQ_SIX_A      0x0B
#define FREQ_SIX_B      0x09/*6*/
#define FREQ_SIX_C      0x02
#define FREQ_SIX_D      0x02/*6*/

#define FREQ_SEVEN_A    0x00
#define FREQ_SEVEN_B    0x01/*7*/
#define FREQ_SEVEN_C    0x00
#define FREQ_SEVEN_D    0x03/*7*/

#define FREQ_EIGHT_A    0x0B
#define FREQ_EIGHT_B    0x09/*8*/
#define FREQ_EIGHT_C    0x02
#define FREQ_EIGHT_D    0x03/*8*/

#define FREQ_NINE_A     0x03
#define FREQ_NINE_B     0x09/*9*/
#define FREQ_NINE_C     0x02
#define FREQ_NINE_D     0x03/*9*/

#define CH_ZERO_A       0x0F       
#define CH_ZERO_B       0x05/*0*/
                               
#define CH_ONE_A        0x06       
#define CH_ONE_B        0x00/*1*/  
                               
#define CH_TWO_A        0x0B       
#define CH_TWO_B        0x06/*2*/  
                               
#define CH_THREE_A      0x0F     
#define CH_THREE_B      0x02/*3*/
                               
#define CH_FOUR_A       0x06     
#define CH_FOUR_B       0x03/*4*/
                               
#define CH_FIVE_A       0x0D     
#define CH_FIVE_B       0x03/*5*/
                               
#define CH_SIX_A        0x0D       
#define CH_SIX_B        0x07/*6*/  
                               
#define CH_SEVEN_A      0x07     
#define CH_SEVEN_B      0x00/*7*/
                               
#define CH_EIGHT_A      0x0F     
#define CH_EIGHT_B      0x07/*8*/
                               
#define CH_NINE_A       0x0F     
#define CH_NINE_B       0x03/*9*/

#define LCD_PILOT       ((29<<4) | 0x08)
#define LCD_PILOT_DIS   ((29<<4) | 0x00)
#define LCD_MUTE        ((25<<4) | 0x08)
#define LCD_UNMUTE      ((25<<4) | 0x00)
#define LCD_LOCK        ((18<<4) | 0x08)
#define LCD_DOT         ((11<<4) | 0x04)

#define LCD_BATH        ((30<<4) | 0x0F)
#define LCD_BATM        ((30<<4) | 0x0D)
#define LCD_BATL        ((30<<4) | 0x09)
#define LCD_BATZ        ((30<<4) | 0x01)

//-----------------------------------------------------------------------------
//EEPROM操作部分宏定义
//-----------------------------------------------------------------------------
#define EEPROM_UNLOCK_KEY1     0x56  //第一个密钥
#define EEPROM_UNLOCK_KEY2     0xAE  //第二个密钥

//-----------------------------------------------------------------------------
//关机控制引脚定义
//-----------------------------------------------------------------------------
sbit LDO_CTRL   =    P1^7; // 控制开关机POP声
sbit VBAT_IN    =    P1^6; // 控制开关机POP声

//-----------------------------------------------------------------------------
//I2C引脚定义
//-----------------------------------------------------------------------------
sbit SDA        =    P3^0;                                            
sbit SCL        =    P3^1;

//-----------------------------------------------------------------------------
//KEY引脚定义
//-----------------------------------------------------------------------------
sbit RF_POW_SW  =   P4^0; // 射频大小控制
sbit IR_SEND    =   P1^3;
sbit powerOn    =   P3^5;
sbit porN       =   P3^4;

//-----------------------------------------------------------------------------
//LCD引脚定义
//-----------------------------------------------------------------------------
sbit LCD_LED    =    P1^1; // LCD 背光
sbit LCD_WR     =    P1^0; // LCD 控制
sbit LCD_DATA   =    P3^7; // LCD 数据
sbit LCD_CS     =    P4^1; // LCD 片选

sbit Key_UP     =    P4^2; // 频率向上控制
sbit Key_DOWN   =    P3^2; // 频率向下控制
sbit Key_SET    =    P3^3; // 频率设定

//-----------------------------------------------------------------------------
// SYS Function PROTOTYPES
//-----------------------------------------------------------------------------
void SYSCLK_Init(void);
void PORT_Init(void);
void Erase_Eeprom (UINT8 page);
void Write_Eeprom(UINT8 wr_data,UINT8 page,UINT8 offset);
UINT8 Read_Eeprom(UINT8 code *addr);
void Save_Freq (void);
void Load_Freq (void);
void KT_MicTX_Batter_Detecter (void);
void KT_MicTX_RFSwitch (void);
void KT_MicTX_Next_Fre (void); // KT_MicTX加100KHz
void KT_MicTX_Previous_Fre (void); // KT_MicTX减100KHz
void KT_MicTX_Mute (void); // KT_MicTX静音
void ex2Init(void);
void timer0Init(void);
void KT_MicTX_Init (void); // KT_MicTX初始化
void KEY_UP (void);
void KEY_DOWN (void);
void KEY_DIS (void);
void KT_MicTX_PowerDown_Detecter (void);

//-----------------------------------------------------------------------------
// LCD_KEY Function PROTOTYPES
//-----------------------------------------------------------------------------
void Delay_ms(UINT16 iTime_Number);
void Delay_us(UINT8 iTime_Number);
UINT8 Key_Scan (void);
void LCD_Init(void);
void Scan_segs(UINT8 dByte, UINT16 Tscan);
void SendBit_1621(UINT8 cSenddata, UINT8 cCounter);
void SendComBit_1621(UINT8 cSenddata); //data的低位先写入HT1621，低位在前
void WriteFix_1621(UINT16 iSenddata);
void Write_1621(UINT8 cAddr,UINT8 cCom);
void Display_Freq_Num(UINT8 cAddress,UINT8 cNum);
void Display_Frequency_and_RFPower(INT32 lFreq);
void RF_Power(UINT8 cPower_Level);
void Display_Ch_Num(UINT8 address, UINT8 num);

//-----------------------------------------------------------------------------
// I2C Function PROTOTYPES
//-----------------------------------------------------------------------------
void I2C_Delay(void);                                            
void I2C_Start(void);
void I2C_Senddata(UINT8 senddata);
void I2C_Ack(void);
void I2C_Stop(void);
UINT8 I2C_Receivedata(void);
void I2C_Word_Write(UINT8 device_address, UINT8 reg_add, UINT16 writeword);
UINT16 I2C_Word_Read(UINT8 device_address, UINT8 reg_add);

//-----------------------------------------------------------------------------
// IR_SEND Function PROTOTYPES
//-----------------------------------------------------------------------------
void H_26us(UINT8 udelay);
void L_26us(UINT8 udelay);
void IR_8byte(UINT8 udata);
void IR_send(UINT32 CHAN);

