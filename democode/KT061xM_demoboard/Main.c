//*********************************************************************
//  File Name: main.c
//  Function:  KT Wireless Mic Receiver Products Demoboard
//*********************************************************************
//        Revision History
//  Version	Date		Description
//  V1.0	2012-08-28	Initial draft
//	V1.1	2012-12-17	SQUEAL is added.
//	V1.2	2012-12-18	Simplify main.
//	V1.3	2013-03-29	修正了开关机死机和关机反复上电的BUG。
//	V1.4	2013-04-24	将A、B两路的频率范围区分开。
//						将前面板中间的按键作为导频检测的开关。
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <intrins.h>
#include <stdio.h>
#include <KT_WirelessMicRxdrv.h>
#include <interface.h>

//------------------------------------------------------------------------------------
// Global VARIABLES
//------------------------------------------------------------------------------------
extern uchar bChannel;
extern uchar xdata K4;

extern uchar	CURRENT_VOL[CHANNEL_NUMBER];
extern uchar	Flag_PILOT[CHANNEL_NUMBER];
extern uchar	Flag_PKGSYNC[CHANNEL_NUMBER];
extern uchar	Flag_AUTOMUTE[CHANNEL_NUMBER];
extern uchar	Flag_BURSTDATA[CHANNEL_NUMBER];
extern uchar	Flag_SQUEAL[CHANNEL_NUMBER];

#if (CHANNEL_NUMBER ==2)
long xdata Memery_Frequency[CHANNEL_NUMBER] = {662000,662000};
long xdata Load_Frequency[CHANNEL_NUMBER] = {662000,662000};
#else
long xdata Memery_Frequency[CHANNEL_NUMBER] = 662000;
long xdata Load_Frequency[CHANNEL_NUMBER] =0;
#endif

uchar Key_UP_flag=0;
uchar Key_DOWN_flag=0;
//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
void SYSCLK_Init (void);
void PORT_Init (void);

void KT_MicRX_Init (void);									// KT_MicRX初始化
void KT_MicRX_Next_Fre (void);								// KT_MicRX加100KHz
void KT_MicRX_Previous_Fre (void);							// KT_MicRX减100KHz
//void KT_MicRX_Mute (void);									// KT_MicRX静音
void KT_MicRX_PowerDown_Detecter (void);
void KT_MicRX_Batter_Detecter (void);
uchar Vol_Display(void);
uchar RSSI_Display(void);
void LCD_Refresh (void);
void DEMO_Init (void);
void Save_Freq (void);
void Load_Freq (void);

void KT_MicRX_Automute_Pilot(void);

//------------------------------------------------------------------------------------
//函 数 名：main
//功能描述：主程序
//函数说明：
//调用函数：SYSCLK_Init(); PORT_Init(); DEMO_Init(); LCD_Refresh(); 
//			KT_MicRX_PowerDown_Detecter(); Key_Scan();
//			KT_MicRX_Next_Fre(); KT_MicRX_Previous_Fre();
//全局变量：bChannel; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-03-29
//版    本：V1.0
//			V1.1	2012-12-17	SQUEAL is added.
//			V1.2	2012-12-18	Simplify main.
//			V1.3	2013-03-29	增加LDO_CTRL = 1解决关机后反复上电和死机问题;
//			V1.4	2013-04-24	将前面板中间的按键作为导频检测的开关;
//------------------------------------------------------------------------------------
void main (void)
{
	uchar ckey=0;
	
	PCA0MD &= ~0x40;                       					// WDTE = 0 (clear watchdog timer enable)
	SYSCLK_Init ();                        					// Initialize system clock to 24.5MHz
	PORT_Init ();                          					// Initialize crossbar and GPIO
	DEMO_Init();
//	LDO_CTRL = 1;  											//正常工作后使LDO_CTRL状态正确

	while (1)
	{
		LCD_Refresh();
		KT_MicRX_PowerDown_Detecter();

		ckey = Key_Scan();
		switch(ckey)
		{
			case 1	:	LCD_LED_A = 0;	bChannel = CH_A;	KT_MicRX_Next_Fre();		break;
			case 2	:	LCD_LED_A = 0;	bChannel = CH_A;	KT_MicRX_Previous_Fre();	break;
			case 3	:	LCD_LED_A = 0;	bChannel = CH_A;	KT_MicRX_Automute_Pilot();	break;
//			case 3	:	LCD_LED_A = 0;	bChannel = CH_A;	KT_MicRX_Mute();			break;
			case 4	:	LCD_LED_B = 0;	bChannel = CH_B;	KT_MicRX_Next_Fre();		break;
			case 5	:	LCD_LED_B = 0;	bChannel = CH_B;	KT_MicRX_Previous_Fre();	break;
			case 6	:	LCD_LED_B = 0;	bChannel = CH_B;	KT_MicRX_Automute_Pilot();	break;
//			case 6	:	LCD_LED_B = 0;	bChannel = CH_B;	KT_MicRX_Mute();			break;
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
//版    本：V1.0
//			V1.3----打开电池电压检测复位功能   
//------------------------------------------------------------------------------------
void SYSCLK_Init (void)
{
	OSCICN |= 0x03;   //0xc3								// configure internal oscillator for 24.5MHz

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
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void PORT_Init (void)
{
	XBR0    = 0x00;											// Disable all
	XBR1    = 0x40;											// Enable crossbar and Disable weak pull-ups
	P0MDOUT	= 0x08;											// 
//	P0MDIN	|= 0xF7;										// 
//	P0		|= 0xF7;										// 

	P1MDOUT	= 0xF1;	
//	P1MDIN	|= 0x0E;										// 
//	P1		|= 0x0E;										// 

	P2MDOUT	= 0x8F;	
//	P2MDIN	|= 0x70;										// 
//	P2		|= 0x70;										// 

	P3MDOUT	= 0x18;
//	P3MDIN	|= 0x04;										// 
//	P3		|= 0x04;										// 
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicRX_Init
//功能描述：芯片初始化
//函数说明：
//调用函数：Load_Freq(); KT_WirelessMicRx_PreInit(); KT_WirelessMicRx_Init(); 
//			KT_WirelessMicRx_Tune(); Display_Frequency_and_RFPower();
//全局变量：Memery_Frequency[]; Load_Frequency[];bChannel;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void KT_MicRX_Init(void)
{
	Load_Freq();
	while(!KT_WirelessMicRx_PreInit());
	while(!KT_WirelessMicRx_Init());
	Memery_Frequency[bChannel] = Load_Frequency[bChannel];
	KT_WirelessMicRx_Tune(Memery_Frequency[bChannel]);
	Display_Frequency_and_RFPower(Memery_Frequency[bChannel]);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicRX_Next_Fre
//功能描述：频率加BAND_STEP，长按加4倍的BAND_STEP，超长按加40倍的BAND_STEP
//函数说明：
//调用函数：KT_WirelessMicRx_Tune(); Display_Frequency_and_RFPower(); WriteFix_1621();
//			RF_Power(); Audio_Lever(); Delay_ms();
//全局变量：Memery_Frequency[i]; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-04-24
//版    本：V1.0
//			V1.4	2013-04-24	将A、B两路的频率范围区分开
//------------------------------------------------------------------------------------
void KT_MicRX_Next_Fre (void)									// 加250KHz
{
	Key_DOWN_flag = 0;
	if(bChannel == CH_A)
		Mute_A = 1;
	else
		Mute_B = 1;
	Delay_ms(50);
	if(Key_UP_flag < 20)
	{
		if(Key_DOWN_flag < 10)
			Memery_Frequency[bChannel] = Memery_Frequency[bChannel] + BAND_STEP;
		else
			Memery_Frequency[bChannel] = Memery_Frequency[bChannel] + BAND_STEP*4;

		Key_UP_flag = Key_UP_flag + 1;
	}
	else
	{
		Memery_Frequency[bChannel] = Memery_Frequency[bChannel] + BAND_STEP*20;
		Key_UP_flag = 20;
	}
	if(bChannel == CH_A)
	{
		if( (Memery_Frequency[bChannel] > BAND_TOP_CHA) || (Memery_Frequency[bChannel] < BAND_BOTTOM_CHA) )
			Memery_Frequency[bChannel] = BAND_BOTTOM_CHA;
	}
	else
	{
		if( (Memery_Frequency[bChannel] > BAND_TOP_CHB) || (Memery_Frequency[bChannel] < BAND_BOTTOM_CHB) )
			Memery_Frequency[bChannel] = BAND_BOTTOM_CHB;
	}
	KT_WirelessMicRx_Tune(Memery_Frequency[bChannel]);
//	if(bChannel == CH_A)
//		Mute_A = 0;
//	else
//		Mute_B = 0;
	WriteFix_1621(LCD_BATZ);
	WriteFix_1621(LCD_PILOT_DIS);
	RF_Power(0);
	Audio_Lever(0);				
	Display_Frequency_and_RFPower(Memery_Frequency[bChannel]);	 
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicRX_Previous_Fre
//功能描述：频率减BAND_STEP，长按减4倍的BAND_STEP，超长按减20倍的BAND_STEP
//函数说明：
//调用函数：KT_WirelessMicRx_Tune(); Display_Frequency_and_RFPower(); WriteFix_1621();
//			RF_Power(); Audio_Lever(); Delay_ms();
//全局变量：Memery_Frequency[i]; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：KANG Hekai				时间：2013-04-24
//版    本：V1.0
//			V1.3	2013-04-24	将A、B两路的频率范围区分开
//------------------------------------------------------------------------------------
void KT_MicRX_Previous_Fre (void)								// 减250KHz
{
	Key_UP_flag = 0;
	if(bChannel == CH_A)
		Mute_A = 1;
	else
		Mute_B = 1;
	Delay_ms(50);
	if(Key_DOWN_flag < 20)
	{
		if(Key_DOWN_flag < 10)
			Memery_Frequency[bChannel] = Memery_Frequency[bChannel] - BAND_STEP;
		else
			Memery_Frequency[bChannel] = Memery_Frequency[bChannel] - BAND_STEP*4;

		Key_DOWN_flag = Key_DOWN_flag + 1;
	}
	else
	{
		Memery_Frequency[bChannel] = Memery_Frequency[bChannel] - BAND_STEP*20;
		Key_DOWN_flag = 20;
	}
	if(bChannel == CH_A)
	{
		if( (Memery_Frequency[bChannel] > BAND_TOP_CHA) || (Memery_Frequency[bChannel] < BAND_BOTTOM_CHA) )
			Memery_Frequency[bChannel] = BAND_BOTTOM_CHA;
	}
	else
	{
		if( (Memery_Frequency[bChannel] > BAND_TOP_CHB) || (Memery_Frequency[bChannel] < BAND_BOTTOM_CHB) )
			Memery_Frequency[bChannel] = BAND_BOTTOM_CHB;
	}

	KT_WirelessMicRx_Tune(Memery_Frequency[bChannel]);
//	if(bChannel == CH_A)
//		Mute_A = 0;
//	else
//		Mute_B = 0;
	WriteFix_1621(LCD_BATZ);
	WriteFix_1621(LCD_PILOT_DIS);
	RF_Power(0);							 
	Audio_Lever(0);
	Display_Frequency_and_RFPower(Memery_Frequency[bChannel]);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicRX_PowerDown_Detecter
//功能描述：
//函数说明：
//调用函数：Delay_ms(); KT_WirelessMicRx_Volume(); Save_Freq();
//全局变量：bChannel; Memery_Frequency[]; Load_Frequency[];
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-20
//修 改 者：						时间：
//版    本：V1.1
//------------------------------------------------------------------------------------
void KT_MicRX_PowerDown_Detecter (void)
{
	if(VBAT_IN == 0)
	{
		Delay_ms(500);
		if(VBAT_IN == 0)														//防止电容放电不干净反复上电
		{
		                                                                                                                            
			bChannel = 0;
			KT_WirelessMicRx_Volume(0);
			bChannel = 1;
			KT_WirelessMicRx_Volume(0);
			Delay_ms(200);
			Mute_A = 1;
			Mute_B = 1;
			Delay_ms(200);

			if( (	Memery_Frequency[CH_A] != Load_Frequency[CH_A] ) || ( Memery_Frequency[CH_B] != Load_Frequency[CH_B] ) )
				Save_Freq();
			Delay_ms(100);
			LDO_CTRL = 0;

		}
	}
	else
	{
		Delay_ms(100);
		if(VBAT_IN == 1)														//防止电容放电不干净反复上电
			LDO_CTRL = 1;
		//	Mute_A = 1;
		//	Mute_B = 1;
	}
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicRX_Batter_Detecter
//功能描述：电池电压监测并显示
//函数说明：
//调用函数：KT_Bus_Read();WriteFix_1621();
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void KT_MicRX_Batter_Detecter (void)
{

	uint xdata Batter_Voltage;

	Batter_Voltage=KT_Bus_Read(0x04);

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
//函 数 名：Vol_Display
//功能描述：电池电压监测并显示
//函数说明：
//调用函数：KT_WirelessMicRx_Automute();WriteFix_1621();Audio_Lever();KT_WirelessMicRx_GetAF();
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-11-30
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
uchar Vol_Display(void)
{
	uchar cAF;

	KT_WirelessMicRx_Automute();
	if( Flag_AUTOMUTE[bChannel] == AUTOMUTE )
	{
		if(bChannel == CH_A)
			Mute_A = 1;
		else
			Mute_B = 1;
		WriteFix_1621(LCD_MUTE);
		Audio_Lever(0);
	}
	else
	{
		WriteFix_1621(LCD_UNMUTE);
		cAF=KT_WirelessMicRx_GetAF();
		if( (cAF <= 15) && (cAF >= 13) )
		{
			Audio_Lever(7);
		}
		else if( (cAF <= 12) && (cAF >= 11) )
		{
			Audio_Lever(6);
		}
		else if( cAF == 10 )
		{
			Audio_Lever(5);
		}
		else if( cAF == 9 )
		{
			Audio_Lever(4);
		}
		else if( (cAF <= 8) && (cAF >= 7) )
		{
			Audio_Lever(3);
		}
		else if( (cAF <= 6) && (cAF > 5) )
		{
			Audio_Lever(2);
		}
		else if( (cAF <= 4) && (cAF >2) )
		{
			Audio_Lever(1);
		}
		else
		{
			Audio_Lever(0);
		}
		if(bChannel == CH_A)
			Mute_A = 0;
		else
			Mute_B = 0;
	}
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：RSSI_Display
//功能描述：信号强度显示
//函数说明：
//调用函数：KT_WirelessMicRx_GetRSSI();RF_Power();Display_Frequency_and_RFPower();
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-11-30
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
uchar RSSI_Display(void)
{
	uchar cRssi;

	cRssi=KT_WirelessMicRx_GetRSSI();

	if( (cRssi <= 255) && (cRssi > 235) )
	{
		RF_Power(7);
	}
	else if( (cRssi <= 235) && (cRssi > 215) )
	{
		RF_Power(6);
	}
	else if( (cRssi <= 215) && (cRssi > 195) )
	{
		RF_Power(5);
	}
	else if( (cRssi <= 195) && (cRssi > 175) )
	{
		RF_Power(4);
	}
	else if( (cRssi <= 175) && (cRssi > 155) )
	{
		RF_Power(3);
	}
	else if( (cRssi <= 155) && (cRssi > 135) )
	{
		RF_Power(2);
	}
	else if( (cRssi <= 135) && (cRssi > 115) )
	{
		RF_Power(1);
	}
	else
	{
		RF_Power(0);
	}
//	Display_Frequency_and_RFPower(Memery_Frequency[bChannel]);
	return(1);
}
//------------------------------------------------------------------------------------
//函 数 名：Save_Freq
//功能描述：将频率保存在MCU的FLASH里
//函数说明：
//调用函数：无
//全局变量：Memery_Frequency[i]
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void Save_Freq (void)
{
	uchar xdata i,j;
	uchar xdata a[8];
	uchar xdata * data pwrite;							// 程序存储器空间的指针（FLASH），指向待写地址

	for(i=0;i<2;i++)
	{
		for(j=0;j<4;j++)
			a[i*4+j]= Memery_Frequency[i] >> (j*8);
	}

//erase
	VDM0CN = 0x80;										// enable VDD monitor
	RSTSRC = 0x02;										// enable VDD monitor as a reset source
	pwrite = 0x3A00;
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
	pwrite = 0x3A00;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[3];									// write the byte
	pwrite = pwrite + 1;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[2];									// write the byte
	pwrite = pwrite + 1;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[1];									// write the byte
	pwrite = pwrite + 1;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[0];									// write the byte
	pwrite = pwrite + 1;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[7];									// write the byte
	pwrite = pwrite + 1;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[6];									// write the byte
	pwrite = pwrite + 1;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[5];									// write the byte
	pwrite = pwrite + 1;

	FLKEY = 0xA5;									// Key Sequence 1
	FLKEY = 0xF1;									// Key Sequence 2
	*pwrite = a[4];									// write the byte
	pwrite = pwrite + 1;

/*
	for(i=0;i<2;i++)
	{
		for(j=3;j>=0;j--)
		{
			FLKEY = 0xA5;									// Key Sequence 1
			FLKEY = 0xF1;									// Key Sequence 2
			*pwrite = a[j+i*4];									// write the byte
			pwrite = pwrite + 1;
		}
	}
*/
	PSCTL &= ~0x01;										// PSWE = 0
//write end

}

//------------------------------------------------------------------------------------
//函 数 名：Load_Freq
//功能描述：读取保存在MCU的FLASH里的频率值
//函数说明：
//调用函数：无
//全局变量：Load_Frequency; bChannel;
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-08-01
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void Load_Freq (void)
{
	uchar code *pread; 									// 程序存储器空间的指针（FLASH），指向待写地址
	uchar xdata j;

	pread = 0x3A00; 									// 将指针指向待擦除页
	Load_Frequency[bChannel] = *(pread+bChannel*4);							// MOVC指令将读对应的数据

	for(j=1;j<4;j++)
	{
		Load_Frequency[bChannel] = (Load_Frequency[bChannel]<<8) | *(pread+bChannel*4+j);	// MOVC指令将读对应的数据
	}
}
//------------------------------------------------------------------------------------
//函 数 名：LCD_Refresh
//功能描述：刷新LCD屏幕显示
//函数说明：
//调用函数：KT_WirelessMicRx_SQUEAL_Inhibition;Vol_Display;RSSI_Display;
//			KT_WirelessMicRx_CheckPilot;WriteFix_1621;KT_WirelessMicRx_CheckAUXCH;
//			KT_MicRX_Batter_Detecter;Display_Frequency_and_RFPower;
//全局变量：CURRENT_VOL; bChannel;
//输    入：
//返    回：无
//设 计 者：zhoudongfeng			时间：2012-12-18
//修 改 者：						时间：
//版    本：V1.2
//------------------------------------------------------------------------------------
void LCD_Refresh (void)
{
	uchar i;
//	uint data burstdata;
			
	for(i=0;i<CHANNEL_NUMBER;i++)
	{  
		bChannel = i;
#ifdef SQUEAL_EN
		KT_WirelessMicRx_SQUEAL_Inhibition(); 
#endif 	
#ifdef AUTOMUTE_EN				

		Vol_Display();
#endif				
		RSSI_Display();
	
		KT_WirelessMicRx_CheckPilot();
		if(Flag_PILOT[bChannel] == PILOT)
		{
			WriteFix_1621(LCD_PILOT);	
		}
		else
		{
			WriteFix_1621(LCD_PILOT_DIS);	
		}
		
#ifdef KT0616M
		KT_WirelessMicRx_CheckAUXCH();
//		if(Flag_BURSTDATA[bChannel] == 1)
//		{
//			burstdata = KT_WirelessMicRx_GetBurstData();
//		}
		if(Flag_PKGSYNC[bChannel] == PKGSYNC)
		{
			KT_MicRX_Batter_Detecter();
		}
		else
		{
			WriteFix_1621(LCD_BATZ);
		}
#endif
		Display_Frequency_and_RFPower(Memery_Frequency[bChannel]);
	}
}
//------------------------------------------------------------------------------------
//函 数 名：DEMO_Init
//功能描述：整个DEMO初始化
//函数说明：
//调用函数：LCD_Init();Scan_segs;KT_MicRX_Init();KT_WirelessMicRx_Volume;
//全局变量：bChannel;CURRENT_VOL;
//输    入：
//返    回：无
//设 计 者：zhoudongfeng			时间：2012-12-18
//修 改 者：						时间：
//版    本：V1.2
//------------------------------------------------------------------------------------
void DEMO_Init (void)
{
	uchar i;

	Mute_A = 1;
	Mute_B = 1;

	for(i=0;i<CHANNEL_NUMBER;i++)
	{
		bChannel = i;
		LCD_Init();
	}
	Delay_ms(600);
	Scan_segs(0x00,0);
	bChannel = CH_A;
	Scan_segs(0x00,0);
	KT_MicRX_Init();
	bChannel = CH_B;
	KT_MicRX_Init();	

	Delay_ms(200);
	for(i=0;i<CHANNEL_NUMBER;i++)
	{
		bChannel = i;
		CURRENT_VOL[bChannel] = PRESET_VOL;
		KT_WirelessMicRx_Volume(CURRENT_VOL[bChannel]);
	}
}

//------------------------------------------------------------------------------------
//函 数 名：KT_MicRX_Automute_Pilot
//功能描述：导频检测作为Automute的条件的开关程序
//函数说明：
//调用函数：KT_Bus_Read();KT_Bus_Write();
//全局变量：
//输    入：
//返    回：无
//设 计 者：Kang hekai				时间：2013-04-24
//修 改 者：						时间：
//版    本：V1.0
//------------------------------------------------------------------------------------
void KT_MicRX_Automute_Pilot(void)
{
	uint regx;

	regx=KT_Bus_Read(0x1A);
	if( (regx & 0x4000) == 0x4000 )
	{
		KT_Bus_Write(0x1A,regx & 0xBFFF);					//automute_super_rssi_en=1,automute_pilot_en=0  关导频检测
		Display_Ch_Num(13,0);
	}
	else
	{
		KT_Bus_Write(0x1A,regx | 0x4000);					//automute_super_rssi_en=1,automute_pilot_en=1  开导频检测
		Display_Ch_Num(13,1);
	}
}