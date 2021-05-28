//*****************************************************************************
//  File Name: main.c
//  Function:  KT Wireless Mic Transmitter Products Demoboard
//*****************************************************************************
//        Revision History
//  Version    Date        Description
//  V1.0    2016-04-26  初始版本
//  V1.1    2017-02-08  格式规范化整理
//  V0.3    2017-04-27  加减频率的时候，先把导频关了，tune完台后再恢复原来导频的设置（main.c）
//						调台中一直以相同的步进进行tune台，不会调一段时间后步进变成4倍和10倍（main.c）
//*****************************************************************************

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "KT_WirelessMicTxdrv.h"

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
#ifdef IR_RECEIVE
extern UINT16 IR_Counter;
extern BOOL Int_Mode_Flag;
extern UINT8 xdata IRDataCode[4];
#endif

BOOL temp_RF_POW_SW=0; //发射功率状态临时存储


//-----------------------------------------------------------------------------
//函 数 名：main
//功能描述：主程序
//函数说明：
//全局变量：无
//输    入：
//返    回：无
//设 计 者：Zhou Dongfeng            时间：2015-12-28
//修 改 者：Zhou Dongfeng            时间：2016-01-28
//版    本：V2.0        增加了自动搜台与红外对频选项        
//-----------------------------------------------------------------------------
void main (void)
{
    UINT32 lCounter=0x20000;
    
    PORT_Init ();    
    SYSCLK_Init ();                                        
    LCD_Init();
    
#ifdef IR_RECEIVE             
    timer0Init();
    ex2Init();
#endif

    while(!KT_WirelessMicTx_PreInit());
       while(!KT_WirelessMicTx_Init());
    KT_MicTX_Init();
    
    while (1)
    {
         if ((lCounter >= 0x20000))
        {
            LCD_LED = 1; // 关背光
            KT_MicTX_Batter_Detecter();
            if(RF_POW_SW != temp_RF_POW_SW)
            {
                KT_MicTX_RFSwitch();
            }
            lCounter=0;
        } 
        else
        {
            lCounter++;
        }
        KT_MicTX_PowerDown_Detecter();
        switch(Key_Scan())
        {
            case 1    :    LCD_LED = 0;    KT_MicTX_Next_Fre();        lCounter=0;    break;
            case 2    :    LCD_LED = 0;    KT_MicTX_Previous_Fre();    lCounter=0;    break;
            case 3    :    LCD_LED = 0;    KT_MicTX_Mute();            lCounter=0;    break;
            default    :    Key_UP_flag=0;    Key_DOWN_flag=0;            break; 
        }
        
#ifdef IR_RECEIVE
//IRDataCode[1]的高4bit为通道地址，区分A、B路手柄，低8bit为频率信息的最高位，
//IRDataCode[2]为频率信息的中位，IRDataCode[3]为频率信息的最低位
        if(KT_IRB_ADDRESS == (IRDataCode[1] & 0xF0) )
        {
            IRDataCode[1] &= 0x0F;
//            EA=0;
            Memery_Frequency = *((INT32 xdata*)IRDataCode);
            KT_WirelessMicTx_Tune(Memery_Frequency);
            Display_Frequency_and_RFPower(Memery_Frequency);
//            Save_Freq();
//            EA=1;
        }
#endif
    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicTX_Init
//功能描述：芯片初始化
//函数说明：
//全局变量：Memery_Frequency; Load_Frequency;
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：Zhou Dongfeng            时间：2015-12-28
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_MicTX_Init(void)
{
    Load_Freq();
    Memery_Frequency=Load_Frequency;
    KT_WirelessMicTx_Tune(Memery_Frequency);
    KT_WirelessMicTx_EQGAIN(EQ_25H,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_40H,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_63H,EQ_GAIN_0dB);

    KT_WirelessMicTx_EQGAIN(EQ_100H,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_160H,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_250H,EQ_GAIN_0dB);

    KT_WirelessMicTx_EQGAIN(EQ_400H,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_630H,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_1KH,EQ_GAIN_0dB);

    KT_WirelessMicTx_EQGAIN(EQ_1K6,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_2K5,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_4KH,EQ_GAIN_0dB);

    KT_WirelessMicTx_EQGAIN(EQ_6K3,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_10K,EQ_GAIN_0dB);
    KT_WirelessMicTx_EQGAIN(EQ_16K,EQ_GAIN_0dB);

    KT_WirelessMicTx_EQSW(EQ_DISABLE);
    KT_WirelessMicTx_ECHO(ECHO_DISABLE,ECHO_RATIO_0,ECHO_DELAY_22ms);
    Delay_ms(200);
        
    KT_MicTX_RFSwitch();
    KT_WirelessMicTx_PASW(PA_ON);

    KT_WirelessMicTx_Pilot_Fdev(PILOT_FDEV_5K);
    KT_WirelessMicTx_Pilot(PILOT_ENABLE);
    WriteFix_1621(LCD_PILOT);
}

//-----------------------------------------------------------------------------
//函 数 名：PORT_Init
//功能描述：MCU端口初始化
//函数说明：Configure the Crossbar and GPIO ports.
//全局变量：无
//输    入：
//返    回：无
//设 计 者：Zhou Dongfeng            时间：2015-12-22
//修 改 者：                        时间：
//版    本：V1.0                
//-----------------------------------------------------------------------------
void PORT_Init(void) 
{
    //  PX M0 M1 设置IO口状态，
    //当为 0  0 则为 准双向模式，
    //当为 0  1 则为 推挽输出模式，
    //当为 1  0 则为 输入模式(高阻态)，
    //当为 1  1 则为 开漏模式，
    P1M0 = 0x44;     
    P1M1 = 0x83;    
    P1     = 0xbc;
      
    P3M0 = 0x01;
    P3M1 = 0xb3;
    P3     = 0x7c;    

    P4M0 = 0x01;                 
    P4M1 = 0x02;       
    P4     = 0x04;
}

//-----------------------------------------------------------------------------
//函 数 名：SYSCLK_Init
//功能描述：MCU时钟初始化
//函数说明：use the internal 16.6MHz
//全局变量：无
//输    入：
//返    回：无
//设 计 者：Zhou Dongfeng            时间：2014-04-11
//修 改 者：                        时间：
//版    本：V1.0                
//-----------------------------------------------------------------------------
void SYSCLK_Init(void)
{
    CLKCON = 0x00; //  f SYS = fOSCS =16.6MHz     
//    CLKCON = 0x20; //  f SYS = fOSCS1/2
//    CLKCON = 0x40; //  f SYS = fOSCS1/4
    //CLKCON = 0x60; //  f SYS = fOSCS1/12
    //ADCON  = 0x00;
    //CMPCON = 0x00;  
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicTX_PowerDown_Detecter
//功能描述：关机程序
//函数说明：
//全局变量：
//输    入：无
//返    回：无
//设 计 者：Zhou Dongfeng            时间：2015-12-28
//修 改 者：                        时间：
//版    本：V1.0                                        
//-----------------------------------------------------------------------------
void KT_MicTX_PowerDown_Detecter (void)
{
    if(VBAT_IN == 0)
    {
        Delay_ms(5);
        if(VBAT_IN == 0) //防止电容放电不干净反复上电
        {    
            if(    Memery_Frequency != Load_Frequency )
                Save_Freq();
            KT_WirelessMicTx_PowerDownProcedure();    
            LDO_CTRL = 0;
            Delay_ms(15);
            if(LDO_CTRL==0)
            {
                (*((void (code*)(void))0x0000))();
            }
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicTX_Batter_Detecter
//功能描述：电池电压监测并显示
//函数说明：
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_MicTX_Batter_Detecter (void)
{
    UINT16 xdata Batter_Voltage;

    Batter_Voltage = KT_WirelessMicTx_BatteryMeter_Read();

    if( (Batter_Voltage <= BATTERY_MAX) && (Batter_Voltage > BATTERY_HIGHTH) )
    {
        WriteFix_1621(LCD_BATH);
    }
    else if( (Batter_Voltage <= BATTERY_HIGHTH) && (Batter_Voltage > BATTERY_MIDDLETH) )
    {
        WriteFix_1621(LCD_BATM);
    }
    else if( (Batter_Voltage <= BATTERY_MIDDLETH) && (Batter_Voltage > BATTERY_LOWTH) )
    {
        WriteFix_1621(LCD_BATL);
    }
    else
    { 
        WriteFix_1621(LCD_BATZ);
    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicTX_RFSwitch
//功能描述：发射功率切换
//函数说明：
//全局变量：temp_RF_POW_SW
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_MicTX_RFSwitch (void)
{
    if (RF_POW_SW == 0)
    {
        if(Key_RF_POW_flag)//去抖
            Delay_ms(100);
        if (RF_POW_SW == 0) 
        {
            KT_WirelessMicTx_PAGain(10); //差分10dBm，单端7dBm
            RF_Power(3);
        }
    }
    else
    {
        if(Key_RF_POW_flag)//去抖
            Delay_ms(100); 
        if (RF_POW_SW == 1)    
        {
            KT_WirelessMicTx_PAGain(42); //差分18dBm，单端15dBm
            RF_Power(7);
        }
    }
    temp_RF_POW_SW = RF_POW_SW;
    Display_Frequency_and_RFPower(Memery_Frequency);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicTX_Next_Fre
//功能描述：频率加BAND_STEP，长按加4倍的BAND_STEP，超长按加40倍的BAND_STEP
//函数说明：
//全局变量：Memery_Frequency; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：KANG Hekai                时间：2013-03-29
//版    本：V1.2
//          V1.5    2013-03-29    切换频率后检测RF开关的位置决定发射功率
//  		V1.6    2017-04-27    加频率的时候，先把导频关了，tune完台后再恢复原来导频的设置
//								  调台中一直以相同的步进进行tune台，不会调一段时间后步进变成4倍和10倍
//-----------------------------------------------------------------------------
void KT_MicTX_Next_Fre (void) // 加250KHz
{
    UINT16 pilotSave;
	Key_RF_POW_flag = 0;
	pilotSave=KT_Bus_Read(0x1F);	  //读导频的设置
 	KT_WirelessMicTx_Pilot(PILOT_DISABLE);
    KT_WirelessMicTx_PAGain(0);
    KT_WirelessMicTx_PASW(PA_OFF);                        
    Delay_ms(5);

    do
    {
        Delay_ms(100);
        Memery_Frequency = Memery_Frequency + BAND_STEP;
        if((Memery_Frequency > BAND_TOP) || (Memery_Frequency < BAND_BOTTOM))
		{
        	Memery_Frequency = BAND_BOTTOM;
		}
		Display_Frequency_and_RFPower(Memery_Frequency);
    }while(Key_UP==0);

    KT_WirelessMicTx_Tune(Memery_Frequency);
    KT_MicTX_RFSwitch();
    KT_WirelessMicTx_PASW(PA_ON);
	KT_Bus_Write(0x1F,pilotSave);//恢复导频的设置
    Key_RF_POW_flag = 1;
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicTX_Previous_Fre
//功能描述：频率减BAND_STEP，长按减4倍的BAND_STEP，超长按减20倍的BAND_STEP
//函数说明：
//全局变量：Memery_Frequency; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：KANG Hekai                时间：2013-03-29
//版    本：V1.2
//          V1.5    2013-03-29    切换频率后检测RF开关的位置决定发射功率
//  		V1.6    2017-04-27    减频率的时候，先把导频关了，tune完台后再恢复原来导频的设置
//								  调台中一直以相同的步进进行tune台，不会调一段时间后步进变成4倍和10倍
//-----------------------------------------------------------------------------
void KT_MicTX_Previous_Fre (void) // 减250KHz
{
    UINT16 pilotSave;
	Key_RF_POW_flag = 0;
	pilotSave=KT_Bus_Read(0x1F);	  //读导频的设置
	KT_WirelessMicTx_Pilot(PILOT_DISABLE);
    KT_WirelessMicTx_PAGain(0);
    KT_WirelessMicTx_PASW(PA_OFF);                    
    Delay_ms(5);

	do
    {
        Delay_ms(100);
        Memery_Frequency = Memery_Frequency - BAND_STEP;
        if((Memery_Frequency > BAND_TOP) || (Memery_Frequency < BAND_BOTTOM))
		{
        	Memery_Frequency = BAND_TOP;
		}
		Display_Frequency_and_RFPower(Memery_Frequency);
    }while(Key_DOWN==0);

    KT_WirelessMicTx_Tune(Memery_Frequency);
    KT_MicTX_RFSwitch();
    KT_WirelessMicTx_PASW(PA_ON);
	KT_Bus_Write(0x1F,pilotSave);//恢复导频的设置
    Key_RF_POW_flag = 1;
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicTX_Mute
//功能描述：按下在静音和有声两种模式下来回变化
//函数说明：
//全局变量：Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_MicTX_Mute (void)
{
    UINT16 reg1C;

    reg1C = KT_Bus_Read(0x1C);
    if( (reg1C & 0x0001) == 0) // 按键按下检查静音标志位，如果静音标志为低则静音
    {
        KT_WirelessMicTx_MuteSel(AUDIO_MUTE);
        KT_WirelessMicTx_Pilot(PILOT_DISABLE);
        WriteFix_1621(LCD_MUTE);    
    }
    else // 按键按下检查静音标志位，如果静音标志为高则取消静音
    {
        KT_WirelessMicTx_MuteSel(AUDIO_UNMUTE);
        KT_WirelessMicTx_Pilot(PILOT_ENABLE);
        WriteFix_1621(LCD_UNMUTE);    
    }
    Key_UP_flag=0;
    Key_DOWN_flag=0;
}

//-----------------------------------------------------------------------------
//函 数 名：Erase_Eeprom
//功能描述：擦除EEPROM
//函数说明：
//全局变量：无
//输    入：uchar page
//返    回：无
//设 计 者：Zhou Dongfeng            时间：2015-12-23
//修 改 者：                        时间：
//版    本：V1.0                
//-----------------------------------------------------------------------------
void Erase_Eeprom (UINT8 page)
{
    EA = 0;    //Disable global interrupt
    FLASHCON = 0x01;    // for EEPROM
    
    XPAGE = page;

    IB_CON1 = 0xE6;    //sector erase
    IB_CON2 = 0x05;
    IB_CON3 = 0x0A;
    IB_CON4 = 0x09;
    IB_CON5 = 0x06;
    _nop_();        // these 4 nops are necessary
    _nop_();
    _nop_();
    _nop_();
    FLASHCON = 0x00;    // restore FLASHCON
    XPAGE = 0x00;    // clear XPAGE
    EA = 1;
}

//-----------------------------------------------------------------------------
//函 数 名：Write_Eeprom
//功能描述：EEPROM写程序
//函数说明：EEPROM,在指定位置写入一个字节数据
//全局变量：无
//输    入：wr_data:要写入的数据、offset：位置
//返    回：无
//设 计 者：Zhou Dongfeng            时间：2015-12-23
//修 改 者：                        时间：
//版    本：V1.0                
//-----------------------------------------------------------------------------
void Write_Eeprom(UINT8 wr_data,UINT8 page,UINT8 offset)
{ 
    EA = 0;    //Disable global interrupt
    FLASHCON = 0x01;    // for EEPROM
    XPAGE =  page;
    IB_OFFSET = offset;
    IB_DATA = wr_data;

    IB_CON1 = 0x6E;    //program one byte
    IB_CON2 = 0x05;
    IB_CON3 = 0x0A;
    IB_CON4 = 0x09;
    IB_CON5 = 0x06;
    _nop_();        // these 4 nops are necessary           
    _nop_();                                                         
    _nop_();
    _nop_();
    FLASHCON = 0x00;    // restore FLASHCON
    XPAGE = 0x00;    // clear XPAGE
    EA = 1;
}

//-----------------------------------------------------------------------------
//函 数 名：Read_Eeprom
//功能描述：EEPROM读Byte程序
//函数说明：读取EEPROM指定位置字节数据
//全局变量：无
//输    入：addr:地址
//返    回：的数
//设 计 者：Zhou Dongfeng            时间：2015-12-23
//修 改 者：                        时间：
//版    本：V1.0                
//-----------------------------------------------------------------------------
UINT8 Read_Eeprom(UINT8 code *addr)
{
    UINT8 num = 0;

    FLASHCON |= 0x01;    //FAC = 1     choose the information block
    num = *(addr);

    FLASHCON &= 0xFE;    //FAC = 0     choose the main block
    return num;
}

//-----------------------------------------------------------------------------
//函 数 名：Load_Freq
//功能描述：读取保存在MCU的FLASH里的频率值
//函数说明：
//全局变量：Load_Frequency
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void Load_Freq (void)
{
    INT8 i;
    Load_Frequency=0;
    for(i=2;i>=0;i--)
    {
        Load_Frequency = (Load_Frequency<<8) | Read_Eeprom((UINT8 code *)i);
    }
}

//-----------------------------------------------------------------------------
//函 数 名：Save_Freq
//功能描述：将频率保存在MCU的FLASH里
//函数说明：
//全局变量：Memery_Frequency
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void Save_Freq (void)
{
    INT8 i;
    UINT8 a[4];
    a[0]= Memery_Frequency;
    a[1]= Memery_Frequency>>8;
    a[2]= Memery_Frequency>>16;
    a[3]= Memery_Frequency>>24;
    Erase_Eeprom(0);
    for(i=3;i>=0;i--)
    {
        Write_Eeprom(a[i],0,i);
    }
}

#ifdef IR_RECEIVE
//

//-----------------------------------------------------------------------------
//函 数 名：timer0Init
//功能描述：定时器0初始化
//函数说明：default 1/12 system clock
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void timer0Init(void)
{
    TCON1 = 0x00;
    TMOD = 0x01;//timer0 16bit
    TL0 = 0;
    TH0 = 0;
}

//-----------------------------------------------------------------------------
//函 数 名：ex2Init
//功能描述：外部中断2初始化
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void ex2Init(void)
{    
    EXF0 = 0x04;//下降沿触发
    IEN1 |= 0x04;//ex2=1
    EA = 1;
}

//-----------------------------------------------------------------------------
//函 数 名：ex2Int
//功能描述：外部中断2中断服务程序
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void ex2Int(void) interrupt 9
{
    IEN1 &= ~0x04; //EX2=0，外部中断2
    TR0 = 0; //停止定时器0
    IR_Counter = TH0;
    IR_Counter = (IR_Counter << 8) | TL0;
    TL0 = 0;
    TH0 = 0;
    TR0 = 1; //启动定时器0 
    if(EXF0==0x08)
    {Int_Mode_Flag=0;}
    else
    {Int_Mode_Flag=1;}
    EXF0 ^=0x0c;
    //红外对码功能暂时没有添加
    //Task_IR();
    IEN1 |= 0x04; //EX2=1，外部中断2使能
}
#endif

