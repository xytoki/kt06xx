//*********************************************************************
//  File Name: main.c
//  Function:  KT Wireless Mic Transmitter Products Handheld Demoboard
//*********************************************************************
//        Revision History
//  Version	Date		Description
//  V1.0	2012-08-01	Initial draft.
//  V1.1	2012-08-20	The function KT_MicTX_PowerDown_Detecter is modified. 
//	V1.2	2012-08-28	修正了tune台后PA一直为LOW的BUG
//	V1.3	2012-11-22	修正了换台后需要摘电池的BUG
//	V1.4	2012-12-11	修正了快速开关机死机的BUG
//	V1.5	2013-03-29	修正了开关机死机和关机反复上电的BUG。
//						修改了换频率后发射功率滞后变化的问题。
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <intrins.h>
#include <stdio.h>
#include <KT_WirelessMicTxdrv.h>
#include <interface.h>

//------------------------------------------------------------------------------------
// Global VARIABLES
//------------------------------------------------------------------------------------
extern uchar code RF_L0[],RF_L1[],RF_L2[],RF_L3[],RF_L4[],RF_L5[],RF_L6[],RF_L7[];

long xdata Memery_Frequency = 662000;
long xdata Load_Frequency;

bit temp_RF_POW_SW=0;
uchar Key_UP_flag=0;
uchar Key_DOWN_flag=0;
uchar power_flag=0;
//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
void SYSCLK_Init (void);
void PORT_Init (void);

void KT_MicTX_Init (void);									// KT_MicTX初始化
void KT_MicTX_Next_Fre (void);								// KT_MicTX加100KHz
void KT_MicTX_Previous_Fre (void);							// KT_MicTX减100KHz
void KT_MicTX_Mute (void);									// KT_MicTX静音
void KT_MicTX_PowerDown_Detecter (void);
void KT_MicTX_Batter_Detecter (void);
void KT_MicTX_RFSwitch (void);

void Save_Freq (void);
void Load_Freq (void);

//------------------------------------------------------------------------------------
//函 数 名：main
//功能描述：主程序
//函数说明：
//调用函数：SYSCLK_Init(); PORT_Init(); LCD_Init(); KT_WirelessMicTx_PreInit();
//			KT_WirelessMicTx_Init(); KT_MicTX_Init(); KT_MicTX_PowerDown_Detecter();
//			KT_MicTX_Batter_Detecter(); KT_MicTX_RFSwitch(); Key_Scan();
//			KT_MicTX_Next_Fre(); KT_MicTX_Previous_Fre(); KT_MicTX_Mute();
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-03-22
//版    本：V1.0
//	        V1.4				MOVE KT_MicTX_PowerDown_Detecter（）；
//	        V1.5	2013-03-29	修改lCounter=0x20000;
//								增加LDO_CTRL = 1解决关机后反复上电和死机问题;   
//------------------------------------------------------------------------------------
void main (void)
{
	uchar ckey=0;
	ulong lCounter=0x20000;
//	uchar fdev_mon;
	
	PCA0MD &= ~0x40;                       					// WDTE = 0 (clear watchdog timer enable)
	PORT_Init ();                          					// Initialize crossbar and GPIO
	SYSCLK_Init ();                        					// Initialize system clock to 24.5MHz

	LCD_Init();

	LDO_CTRL = 1;  											//正常工作后使LDO_CTRL状态正确

	while(!KT_WirelessMicTx_PreInit());
   	while(!KT_WirelessMicTx_Init());
	KT_MicTX_Init();
	while (1)
	{
	 	if ( (lCounter >= 0x20000) )
		{
//			fdev_mon = KT_WirelessMicTx_Fdev_Monitor();
			LCD_LED = 1;									// 关背光

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
		ckey = Key_Scan();
		switch(ckey)
		{
			case 1	:	LCD_LED = 0;	KT_MicTX_Next_Fre();		lCounter=0;	break;
			case 2	:	LCD_LED = 0;	KT_MicTX_Previous_Fre();	lCounter=0;	break;
			case 3	:	LCD_LED = 0;	KT_MicTX_Mute();			lCounter=0;	break;
			default	:	Key_UP_flag=0;	Key_DOWN_flag=0;			break; 
		}
	}
}

//------------------------------------------------------------------------------------
//函 数 名：SYSCLK_Init
//功能描述：MCU时钟初始化
//函数说明：
// This routine initializes the system clock to use the internal 24.5MHz / 8 
// oscillator as its clock source.  Also enables missing clock detector reset.
//调用函数：无
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-03-29
//版    本：V1.3				ADD VDM0CN &= 0x7f;   
//			V1.5	2013-03-29	去掉VDM0CN &= 0x7f;打开电池电压检测复位功能   
//------------------------------------------------------------------------------------
void SYSCLK_Init (void)
{
	OSCICN |= 0x03;   //0xc3								// configure internal oscillator for 24.5MHz

//	VDM0CN &= 0x7f;
	VDM0CN = 0x80;										// enable VDD monitor
	Delay_ms(1);
	RSTSRC = 0x02;										// enable VDD monitor as a reset source
}

//------------------------------------------------------------------------------------
//函 数 名：PORT_Init
//功能描述：MCU端口初始化
//函数说明：Configure the Crossbar and GPIO ports.
//调用函数：使用弱上拉
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-03-29
//版    本：V1.3				ADD LDO_CTRL = 0;
//	        V1.5	2013-03-29	修改了延时时间。修改了LDO_CTRL=0的时机。
//------------------------------------------------------------------------------------
void PORT_Init (void)
{
	XBR0    = 0x00;											// Disable all
	XBR1    = 0x40;											// Enable crossbar and Disable weak pull-ups
//	LDO_CTRL = 0;											//防止上电RESET
	P0MDOUT	= 0x7D;											// 
	P0MDIN	|= 0x82;										// 
	P0		|= 0x82;										// 
//	LDO_CTRL = 0;											//防止上电RESET
//	LDO_CTRL = 1;											//防止上电RESET

	P1MDOUT	&= 0xF0;	
	P1MDIN	|= 0x0F;										// 
	P1		|= 0x0F;										// 

	P3MDOUT	&= 0xFB;
	P3MDIN	|= 0x04;										// 
	P3		|= 0x04;										// 
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicTX_Init
//功能描述：芯片初始化
//函数说明：
//调用函数：KT_WirelessMicTx_BatteryMeter_SW(); KT_WirelessMicTx_Mic_Sens();
//			KT_WirelessMicTx_Comp_Dis(); KT_WirelessMicTx_Fdev_Monitor_TC();
//			KT_WirelessMicTx_PreEmpasis_SW(); KT_WirelessMicTx_Pilot();
//			KT_WirelessMicTx_Pilot_Fdev(); WriteFix_1621(); Load_Freq();
//			KT_WirelessMicTx_Tune(); KT_WirelessMicTx_PASW(); KT_MicTX_RFSwitch();
//全局变量：Memery_Frequency; Load_Frequency;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-03-22
//版    本：V1.0
//	        V1.5--------修改了延时时间。
//------------------------------------------------------------------------------------
void KT_MicTX_Init(void)
{
	KT_WirelessMicTx_BatteryMeter_SW(BATTERY_METER_ENABLE);
	KT_WirelessMicTx_Mic_Sens(MIC_SENS_GAIN_5);
	KT_WirelessMicTx_Comp_Dis(COMPANDOR_ENABLE);
	KT_WirelessMicTx_Fdev_Monitor_TC(FDEV_MONITOR_TC_250ms);
	KT_WirelessMicTx_PreEmpasis_SW(PREEMPASIS_ENABLE);


	Load_Freq();
	Memery_Frequency = Load_Frequency;
	KT_WirelessMicTx_Tune(Memery_Frequency);
	Delay_ms(200);

	KT_WirelessMicTx_PASW(PA_ON);
	KT_MicTX_RFSwitch();
//	Delay_ms(200);

	KT_WirelessMicTx_Pilot_Fdev(PILOT_FDEV_5K);
	KT_WirelessMicTx_Pilot(PILOT_ENABLE);
	WriteFix_1621(LCD_PILOT);	
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicTX_Next_Fre
//功能描述：频率加BAND_STEP，长按加4倍的BAND_STEP，超长按加40倍的BAND_STEP
//函数说明：
//调用函数：KT_WirelessMicTx_Tune(); Display_Frequency_and_RFPower();
//全局变量：Memery_Frequency; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-03-29
//版    本：V1.2
//			V1.5	2013-03-29	切换频率后检测RF开关的位置决定发射功率
//------------------------------------------------------------------------------------
void KT_MicTX_Next_Fre (void)									// 加250KHz
{
	uint reg8;

	KT_WirelessMicTx_PAGain(0);
	KT_WirelessMicTx_PASW(PA_OFF);
	reg8 = KT_Bus_Read(0x08);
	KT_Bus_Write(0x08, reg8 + (1<<9));						
	Delay_ms(5);

	if(Key_UP_flag < 20)
	{
		if(Key_DOWN_flag < 10)
			Memery_Frequency = Memery_Frequency + BAND_STEP;
		else
			Memery_Frequency = Memery_Frequency + BAND_STEP*4;

		Key_UP_flag = Key_UP_flag + 1;
	}
	else
	{
		Memery_Frequency = Memery_Frequency + BAND_STEP*20;
		Key_UP_flag = 20;
	}
	if( (Memery_Frequency > BAND_TOP) || (Memery_Frequency < BAND_BOTTOM) )
		Memery_Frequency = BAND_BOTTOM;

	Key_DOWN_flag = 0;
	KT_WirelessMicTx_Tune(Memery_Frequency);

	if (temp_RF_POW_SW == 0)
	{
		KT_WirelessMicTx_PAGain(PA_Pos6dBm);
		RF_Power(3);
	}
	else
	{
		KT_WirelessMicTx_PAGain(PA_Pos12dBm);
		RF_Power(7);
	}
	KT_WirelessMicTx_PASW(PA_ON);

	Display_Frequency_and_RFPower(Memery_Frequency);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicTX_Previous_Fre
//功能描述：频率减BAND_STEP，长按减4倍的BAND_STEP，超长按减20倍的BAND_STEP
//函数说明：
//调用函数：KT_WirelessMicTx_Tune(); Display_Frequency_and_RFPower();
//全局变量：Memery_Frequency; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-03-29
//版    本：V1.2
//			V1.5	2013-03-29	切换频率后检测RF开关的位置决定发射功率
//------------------------------------------------------------------------------------
void KT_MicTX_Previous_Fre (void)								// 减250KHz
{
	uint reg8;

	KT_WirelessMicTx_PAGain(0);
	KT_WirelessMicTx_PASW(PA_OFF);
	reg8 = KT_Bus_Read(0x08);
	KT_Bus_Write(0x08, reg8 + (1<<9));						
	Delay_ms(5);

	if(Key_DOWN_flag < 20)
	{
		if(Key_DOWN_flag < 10)
			Memery_Frequency = Memery_Frequency - BAND_STEP;
		else
			Memery_Frequency = Memery_Frequency - BAND_STEP*4;

		Key_DOWN_flag = Key_DOWN_flag + 1;
	}
	else
	{
		Memery_Frequency = Memery_Frequency - BAND_STEP*20;
		Key_DOWN_flag = 20;
	}
	if( (Memery_Frequency < BAND_BOTTOM) || (Memery_Frequency > BAND_TOP) )
		Memery_Frequency = BAND_TOP;

	Key_UP_flag = 0;
	KT_WirelessMicTx_Tune(Memery_Frequency);

	if (temp_RF_POW_SW == 0)
	{
		KT_WirelessMicTx_PAGain(PA_Pos6dBm);
		RF_Power(3);
	}
	else
	{
		KT_WirelessMicTx_PAGain(PA_Pos12dBm);
		RF_Power(7);
	}
	KT_WirelessMicTx_PASW(PA_ON);

	Display_Frequency_and_RFPower(Memery_Frequency);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicTX_Mute
//功能描述：按下在静音和有声两种模式下来回变化
//函数说明：
//调用函数：KT_Bus_Read(); KT_WirelessMicTx_MuteSel(); WriteFix_1621();
//全局变量：Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void KT_MicTX_Mute (void)
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);
	if( (reg1C & 0x0001) == 0)								// 按键按下检查静音标志位，如果静音标志为低则静音
	{
		KT_WirelessMicTx_MuteSel(AUDIO_MUTE);
		WriteFix_1621(LCD_MUTE);	
	}
	else													// 按键按下检查静音标志位，如果静音标志为高则取消静音
	{
		KT_WirelessMicTx_MuteSel(AUDIO_UNMUTE);
		WriteFix_1621(LCD_UNMUTE);	
	}
	Key_UP_flag=0;
	Key_DOWN_flag=0;
}
//------------------------------------------------------------------------------------
//函 数 名：KT_MicTX_PowerDown_Detecter
//功能描述：
//函数说明：
//调用函数：Save_Freq(); KT_WirelessMicTx_PAGain(); KT_WirelessMicTx_PASW();
//			KT_Bus_Read(); KT_Bus_Write(); Delay_ms();
//全局变量：Memery_Frequency;Load_Frequency;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-20
//修 改 者：KANG Hekai				时间：2013-03-22
//版    本：V1.3-----------	ADD  XBR1    = 0xC0;										
//								P0MDOUT	|= 0x01;				
//								while(1);
//	        V1.4-----------	ADD  power_flag
//	        V1.5-----------	去掉目端口配置和while(1)死循环的bug。
//							增加15ms延时（保证save功能），减少了VBAT_IN去抖的延时。
//------------------------------------------------------------------------------------
void KT_MicTX_PowerDown_Detecter (void)
{
	if(VBAT_IN == 0)
	{
		Delay_ms(5);
		if(VBAT_IN == 0)														//防止电容放电不干净反复上电
		{
			if(	Memery_Frequency != Load_Frequency )
				Save_Freq();

			KT_WirelessMicTx_PowerDownProcedure();
			LDO_CTRL = 0;
			Delay_ms(15);
//			XBR1    = 0xC0;											// Enable crossbar and Disable weak pull-ups
//			P0MDOUT	|= 0x01;										// 
//			while(1);

		}
	}
/*	else
	{
		if(power_flag==0)
		{
		Delay_ms(100);
		power_flag = 1;
		if(VBAT_IN == 1)														
			LDO_CTRL = 1;  											//正常工作后使LDO_CTRL状态正确
		}
	}*/
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicTX_Batter_Detecter
//功能描述：电池电压监测并显示
//函数说明：
//调用函数：KT_WirelessMicTx_BatteryMeter_Read();WriteFix_1621();
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void KT_MicTX_Batter_Detecter (void)
{
	uint xdata Batter_Voltage;

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

//------------------------------------------------------------------------------------
//函 数 名：KT_MicTX_RFSwitch
//功能描述：发射功率切换
//函数说明：
//调用函数：KT_WirelessMicTx_PAGain();	RF_Power(); Display_Frequency_and_RFPower();
//全局变量：temp_RF_POW_SW
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void KT_MicTX_RFSwitch (void)
{
	if (RF_POW_SW == 0)
	{
		Delay_ms(100);
		if (RF_POW_SW == 0)										//去抖
		{
			KT_WirelessMicTx_PAGain(PA_Pos6dBm);
			RF_Power(3);
		}
	}
	else
	{
		Delay_ms(100);
		if (RF_POW_SW == 1)										//去抖
		{
			KT_WirelessMicTx_PAGain(PA_Pos12dBm);
			RF_Power(7);
		}
	}
	temp_RF_POW_SW = RF_POW_SW;
	Display_Frequency_and_RFPower(Memery_Frequency);
}

//------------------------------------------------------------------------------------
//函 数 名：Save_Freq
//功能描述：将频率保存在MCU的FLASH里
//函数说明：
//调用函数：无
//全局变量：Memery_Frequency
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void Save_Freq (void)
{
	char i;
	uchar a[4];
	uchar xdata * data pwrite;							// 程序存储器空间的指针（FLASH），指向待写地址

	a[0]= Memery_Frequency;
	a[1]= Memery_Frequency>>8;
	a[2]= Memery_Frequency>>16;
	a[3]= Memery_Frequency>>24;

//erase
	VDM0CN = 0x80;										// enable VDD monitor
	RSTSRC = 0x02;										// enable VDD monitor as a reset source
	pwrite = 0x1C00;
	FLKEY = 0xA5;										// Key Sequence 1
	FLKEY = 0xF1;										// Key Sequence 2
	PSCTL |= 0x03;										// PSWE = 1; PSEE = 1
	VDM0CN = 0x80;										// enable VDD monitor
	RSTSRC = 0x02;										// enable VDD monitor as a reset source
	*pwrite = 0;										// initiate page erase
	PSCTL &= ~0x03;										// PSWE = 0; PSEE = 0
//erase end

//write
	PSCTL |= 0x01;										// PSWE = 1
	VDM0CN = 0x80;										// enable VDD monitor
	RSTSRC = 0x02;										// enable VDD monitor as a reset source
	pwrite = 0x1C00;
	for(i=3;i>=0;i--)
	{
		FLKEY = 0xA5;									// Key Sequence 1
		FLKEY = 0xF1;									// Key Sequence 2
		*pwrite = a[i];									// write the byte
		pwrite = pwrite + 1;
	}
	PSCTL &= ~0x01;										// PSWE = 0
//write end

}

//------------------------------------------------------------------------------------
//函 数 名：Load_Freq
//功能描述：读取保存在MCU的FLASH里的频率值
//函数说明：
//调用函数：无
//全局变量：Load_Frequency
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void Load_Freq (void)
{
	uchar code *pread; 									// 程序存储器空间的指针（FLASH），指向待写地址
	uchar i;

	pread = 0x1C00; 									// 将指针指向待擦除页
	Load_Frequency = *pread;							// MOVC指令将读对应的数据

	for(i=0;i<3;i++)
	{
		pread = pread + 1; 								// 将指针指向待擦除页
		Load_Frequency = (Load_Frequency<<8) | *pread;	// MOVC指令将读对应的数据
	}
}