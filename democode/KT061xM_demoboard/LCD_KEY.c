//*********************************************************************
//  File Name: LCD_KEY.c
//  Function:  KT Wireless Mic Receiver Products Demoboard
//*********************************************************************
//        Revision History
//  Version	Date		Description
//  V1.0	2012-08-28	Initial draft

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <interface.h>										// SFR declarations
#include <intrins.h>
#include <stdio.h>
#include <KT_WirelessMicRxdrv.h>

uchar code DATAL[] ={0x0A,0x0A,0x06,0x0E,0x0E,	0x0C,0x0C,0x0A,0x0E,0x0E}; 
uchar code DATAH[] ={0x0F,0x00,0x0D,0x09,0x02,	0x0B,0x0F,0x01,0x0F,0x0B};
uchar code FREQ_NUM[] ={	FREQ_ZERO_A,	FREQ_ZERO_B,	FREQ_ZERO_C,	FREQ_ZERO_D,
							FREQ_ONE_A,		FREQ_ONE_B,		FREQ_ONE_C,		FREQ_ONE_D, 	
							FREQ_TWO_A,		FREQ_TWO_B,		FREQ_TWO_C,		FREQ_TWO_D, 	
							FREQ_THREE_A,	FREQ_THREE_B,	FREQ_THREE_C,	FREQ_THREE_D,
							FREQ_FOUR_A,	FREQ_FOUR_B,	FREQ_FOUR_C,	FREQ_FOUR_D,	
							FREQ_FIVE_A,	FREQ_FIVE_B,	FREQ_FIVE_C,	FREQ_FIVE_D,
							FREQ_SIX_A,		FREQ_SIX_B,		FREQ_SIX_C,		FREQ_SIX_D,  
							FREQ_SEVEN_A,	FREQ_SEVEN_B,	FREQ_SEVEN_C,	FREQ_SEVEN_D,
							FREQ_EIGHT_A,	FREQ_EIGHT_B,	FREQ_EIGHT_C,	FREQ_EIGHT_D,
							FREQ_NINE_A,	FREQ_NINE_B,	FREQ_NINE_C,	FREQ_NINE_D
						};

uchar code CH_NUM[] ={	CH_ZERO_A,	CH_ZERO_B, 
						CH_ONE_A,	CH_ONE_B, 	
						CH_TWO_A,	CH_TWO_B, 	
						CH_THREE_A,	CH_THREE_B,
						CH_FOUR_A,	CH_FOUR_B ,
						CH_FIVE_A,	CH_FIVE_B ,
						CH_SIX_A,	CH_SIX_B ,	
						CH_SEVEN_A,	CH_SEVEN_B,
						CH_EIGHT_A,	CH_EIGHT_B,
						CH_NINE_A,	CH_NINE_B
					};

uchar xdata RF	= 0<<3,RF_S1 = 0<<2,RF_S2 = 0<<3,RF_S3 = 0<<3,RF_S4 = 0<<3,RF_S5 = 0<<3,RF_S6 = 0<<2,RF_S7 = 0<<3;
uchar xdata Dot	= 1<<2,K4 = 0<<3,MHz = 1<<3;
uchar xdata AF	= 0<<3,AF_S8 = 0<<2,AF_S9 = 0<<2,AF_S10 = 0<<1,AF_S11 = 0<<0,AF_S12 = 0<<3,AF_S13 = 0<<2,AF_S14 = 0<<3;

extern uchar bChannel;
//------------------------------------------------------------------------------------
//函 数 名：Key_Scan
//功能描述：键盘扫描程序
//函数说明：
//调用函数：无
//全局变量：无
//输    入：Tus
//返    回：key_value
//设 计 者：Kanghekai				时间：
//修 改 者：						时间：
//版    本：
//------------------------------------------------------------------------------------
uchar Key_Scan (void)
{
	char key_value=0;
   if(Key_UP_A==0)							// 判断是否有键按下
	{
		Delay_ms(100);												// 延时去抖动
		if(Key_UP_A==0)
			key_value=1;
	}
	else if(Key_DOWN_A==0)							// 判断是否有键按下
	{
		Delay_ms(100);												// 延时去抖动
		if (Key_DOWN_A==0)	
			key_value=2;
	}
	else if(Key_SET_A==0)							// 判断是否有键按下
	{
		Delay_ms(100);												// 延时去抖动
		if (Key_SET_A==0)	
			key_value=3;
	}
	else if(Key_UP_B==0)							// 判断是否有键按下
	{
		Delay_ms(100);												// 延时去抖动
		if(Key_UP_B==0)
			key_value=4;
	}
	else if(Key_DOWN_B==0)							// 判断是否有键按下
	{
		Delay_ms(100);												// 延时去抖动
		if (Key_DOWN_B==0)	
			key_value=5;
	}
	else if(Key_SET_B==0)							// 判断是否有键按下
	{
		Delay_ms(100);												// 延时去抖动
		if (Key_SET_B==0)	
			key_value=6;
	}
	else
		key_value=0;
	return(key_value);
}

//------------------------------------------------------------------------------------
//函 数 名：Delay_ms
//功能描述：ms级延时
//函数说明：
//调用函数：无
//全局变量：无
//输    入：iTime_Number
//返    回：无
//设 计 者：Kanghekai						时间：
//修 改 者：						时间：
//版    本：
//------------------------------------------------------------------------------------
void Delay_ms(uint iTime_Number)
{
	uint i;
	uint j;
	for (j=0;j<iTime_Number;j++)
	{
		for (i=0;i<0x900;i++)
		{
		}
	}
}

//------------------------------------------------------------------------------------
//函 数 名：Delay_us													 	 	
//功能描述：Us级延时														 	
//函数说明：																 	
//调用函数：无																 	
//全局变量：无																 	
//输    入：iTime_Number																 	
//返    回：无																 	
//设 计 者：ZDF						时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
void Delay_us(uchar iTime_Number)
{
	uchar i;
	for(i=0;i<=iTime_Number;i++)
	{
		_nop_();
	}
}

//------------------------------------------------------------------------------------
//函 数 名：LCD_Init													 	 	
//功能描述：LCD初始化														 	
//函数说明：																 	
//调用函数：SendBit_1621(),Write_1621(),Display_Freq_Num()
//全局变量：无																 	
//输    入：无																 	
//返    回：无																 	
//设 计 者：Kanghekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
void LCD_Init(void)
{
	if(bChannel ==  CH_A)
	{
		LCD_LED_A=0;									// 开背光

		LCD_CS_A = 1;
		Delay_us(50);
		LCD_CS_A = 0;
	}
	else
	{
		LCD_LED_B=0;									// 开背光

		LCD_CS_B = 1;
		Delay_us(50);
		LCD_CS_B = 0;
	}
	Delay_us(10);
	SendBit_1621(CMD_MODE,	3);
	SendBit_1621(SYS_EN,	9);
	SendBit_1621(INT_256K,	9);
	SendBit_1621(LCD_BIAS,	9);
	SendBit_1621(LCD_ON,	9);

	Scan_segs(0x00,0);
//K
	Write_1621(0,0x0B);
	Write_1621(1,0);
	Write_1621(2,5);
	Write_1621(3,0);
//T
	Write_1621(4,0);
	Write_1621(5,5);
	Write_1621(6,0);
	Write_1621(7,0);

#ifdef KT0606M
//0606
	Display_Freq_Num(2,0);
	Display_Freq_Num(3,6);
	Display_Freq_Num(4,0);
	Display_Freq_Num(5,6);
#endif

#ifdef KT0605M
//0605
	Display_Freq_Num(2,0);
	Display_Freq_Num(3,6);
	Display_Freq_Num(4,0);
	Display_Freq_Num(5,5);
#endif

#ifdef KT0616M
//0616
	Display_Freq_Num(2,0);
	Display_Freq_Num(3,6);
	Display_Freq_Num(4,1);
	Display_Freq_Num(5,6);
#endif

//	Delay_ms(600);
//	Scan_segs(0x00,0);
}

//------------------------------------------------------------------------------------
//函 数 名：SendBit_1621												 	 	
//功能描述：设置模式														 	
//函数说明：																 	
//调用函数：无																 	
//全局变量：无																 	
//输    入：cSenddata;cCounter															 	
//返    回：无																 	
//设 计 者：Kanghekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
void SendBit_1621(uchar cSenddata, uchar cCounter)//data 的高cnt 位元寫入HT1621，高位在前
{
	uchar i;
	if(bChannel ==  CH_A)
	{
		LCD_WR_A = 0;
		for (i = 0; i < cCounter; i++)
		{
			LCD_DATA_A = cSenddata & 0x80;
			cSenddata = cSenddata << 1;
			Delay_us(20);
			LCD_WR_A = 1;
			Delay_us(20);
			LCD_WR_A = 0;
		}
	}
	else
	{
		LCD_WR_B = 0;
		for (i = 0; i < cCounter; i++)
		{
			LCD_DATA_B = cSenddata & 0x80;
			cSenddata = cSenddata << 1;
			Delay_us(20);
			LCD_WR_B = 1;
			Delay_us(20);
			LCD_WR_B = 0;
		}
	}
	Delay_us(20);

}

//------------------------------------------------------------------------------------
//函 数 名：SendComBit_1621													 	 	
//功能描述：写数据															 	
//函数说明：																 	
//调用函数：无																 	
//全局变量：无																 	
//输    入：dByte															 	
//返    回：无																 	
//设 计 者：Kanghekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
void SendComBit_1621(uchar cSenddata) //data 的低cnt 位寫入HT1621，低位在前
{
	uchar i;

	if(bChannel ==  CH_A)
	{
		for(i =0; i <4; i ++)
		{
			LCD_DATA_A = cSenddata & 0x01;
			cSenddata = cSenddata >> 1;
			Delay_us(20);
			LCD_WR_A = 1;
			Delay_us(20);
			LCD_WR_A = 0;
			Delay_us(20);
		}
	}
	else
	{
		for(i =0; i <4; i ++)
		{
			LCD_DATA_B = cSenddata & 0x01;
			cSenddata = cSenddata >> 1;
			Delay_us(20);
			LCD_WR_B = 1;
			Delay_us(20);
			LCD_WR_B = 0;
			Delay_us(20);
		}
	}
}

//------------------------------------------------------------------------------------
//函 数 名：SendComBit_1621													 	 	
//功能描述：写数据															 	
//函数说明：																 	
//调用函数：无																 	
//全局变量：无																 	
//输    入：dByte															 	
//返    回：无																 	
//设 计 者：Kanghekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------

void WriteFix_1621(uint iSenddata)
{
	Write_1621(iSenddata>>4,iSenddata);
}

//------------------------------------------------------------------------------------
//函 数 名：Write_1621														 	 	
//功能描述：写地址															 	
//函数说明：																 	
//调用函数：无																 	
//全局变量：无																 	
//输    入：CAddr															 	
//返    回：无																 	
//设 计 者：Kanghekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
void Write_1621(uchar cAddr,uchar cCom)
{
	if(bChannel ==  CH_A)
	{
		LCD_CS_A = 1;
		Delay_us(50);
		LCD_CS_A = 0;
		Delay_us(10);
		cAddr = cAddr <<  2;
		SendBit_1621(WR_MODE,3);
		SendBit_1621(cAddr,6);
		SendComBit_1621(cCom);
		LCD_CS_A=1;
	}
	else
	{
		LCD_CS_B = 1;
		Delay_us(50);
		LCD_CS_B = 0;
		Delay_us(10);
		cAddr = cAddr <<  2;
		SendBit_1621(WR_MODE,3);
		SendBit_1621(cAddr,6);
		SendComBit_1621(cCom);
		LCD_CS_B=1;
	}
	Delay_us(50);
}

//------------------------------------------------------------------------------------
//函 数 名：Scan_segs													 	 	
//功能描述：连续显示														 	
//函数说明：																 	
//调用函数：SendComBit_1621;Write_1621;Delay_ms										 	
//全局变量：无																 	
//输    入：dByte;Tscan														 	
//返    回：无																 	
//设 计 者：ZDF						时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
void Scan_segs(uchar dByte, uint Tscan)
{
	uchar segs;

	if(bChannel ==  CH_A)
	{
		LCD_CS_A = 1;
		Delay_us(50);
		LCD_CS_A = 0;
		Delay_us(10);
		SendBit_1621(WR_MODE,3);
		SendBit_1621(0,6);
		for (segs = 0; segs < LCD_SEG; segs++)
		{
			SendComBit_1621(dByte);
			Delay_ms(Tscan);
		}
		LCD_CS_A=1;
		Delay_us(50);
	}
	else
	{
		LCD_CS_B = 1;
		Delay_us(50);
		LCD_CS_B = 0;
		Delay_us(10);
		SendBit_1621(WR_MODE,3);
		SendBit_1621(0,6);
		for (segs = 0; segs < LCD_SEG; segs++)
		{
			SendComBit_1621(dByte);
			Delay_ms(Tscan);
		}
		LCD_CS_B=1;
		Delay_us(50);
	}

}

//------------------------------------------------------------------------------------
//函 数 名：Display_Frequency_and_RFPower											 	 	
//功能描述：频率显示														 	
//函数说明：																 	
//调用函数：SendComBit_1621;Write_1621;												 	
//全局变量：无																 	
//输    入：M_Fre;K_Fre														 	
//返    回：无																 	
//设 计 者：Kanghekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
void Display_Frequency_and_RFPower(long lFreq)  
{
	uint M_Fre,K_Fre;
	M_Fre = lFreq/1000;
	K_Fre = lFreq%1000;
	
	if( (M_Fre<1000) && (M_Fre>=100) )
	{
		Display_Freq_Num(0,M_Fre/100);
		M_Fre = M_Fre%100;
		Display_Freq_Num(1,M_Fre/10);
		Display_Freq_Num(2,M_Fre%10);
	}

	if (K_Fre==0) 
	{
		Display_Freq_Num(3,0);
		Display_Freq_Num(4,0);
		Display_Freq_Num(5,0);
	}
	else if( (K_Fre < 10)	&& (K_Fre > 0) )
	{
		Display_Freq_Num(3,0);
		Display_Freq_Num(4,0);
		Display_Freq_Num(5,K_Fre);
	}
	else if( (K_Fre < 100) && (K_Fre >= 10) )
	{
		Display_Freq_Num(3,0);
		Display_Freq_Num(4,K_Fre / 10);
		Display_Freq_Num(5,K_Fre % 10);
	}			
	else if( (K_Fre < 1000) && (K_Fre >= 100) )
	{
		Display_Freq_Num(3,K_Fre / 100);
		K_Fre = K_Fre % 100;
		Display_Freq_Num(4,K_Fre / 10);
		Display_Freq_Num(5,K_Fre % 10);
	}

	Write_1621(31,0 | AF_S12 | AF_S11 | AF_S10 | AF_S9);
}

//------------------------------------------------------------------------------------
//函 数 名：Display_LBAT												 	 	
//功能描述：低电量显示														 	
//函数说明：
//调用函数：无
//全局变量：无
//输    入：无																 	
//返    回：无																 	
//设 计 者：Kanghekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------

void Display_Ch_Num(uchar address, uchar num)
{
	uchar i;
	for(i=0;i<2;i++)
	{
		Write_1621(address*2+i,CH_NUM[num*2+i]);
	}
}

//------------------------------------------------------------------------------------
//函 数 名：RF_Power
//功能描述：ms级延时
//函数说明：
//调用函数：无
//全局变量：无
//输    入：cPower_Level
//返    回：无
//设 计 者：Kanghekai				时间：
//修 改 者：						时间：
//版    本：
//------------------------------------------------------------------------------------
void RF_Power(uchar cPower_Level)
{
	switch(cPower_Level)
	{
		case 0:{ RF	= 1<<3;	RF_S1 =	0<<2;	RF_S2 =	0<<3;	RF_S3 =	0<<3;	RF_S4 = 0<<3;	RF_S5 =	0<<3;	RF_S6 =	0<<2;	RF_S7 =	0<<3; }break;
		case 1:{ RF	= 1<<3;	RF_S1 =	1<<2;	RF_S2 =	0<<3;	RF_S3 =	0<<3;	RF_S4 = 0<<3;	RF_S5 =	0<<3;	RF_S6 =	0<<2;	RF_S7 =	0<<3; }break;
		case 2:{ RF	= 1<<3;	RF_S1 =	1<<2;	RF_S2 =	1<<3;	RF_S3 =	0<<3;	RF_S4 = 0<<3;	RF_S5 =	0<<3;	RF_S6 =	0<<2;	RF_S7 =	0<<3; }break;
		case 3:{ RF	= 1<<3;	RF_S1 =	1<<2;	RF_S2 =	1<<3;	RF_S3 =	1<<3;	RF_S4 = 0<<3;	RF_S5 =	0<<3;	RF_S6 =	0<<2;	RF_S7 =	0<<3; }break;
		case 4:{ RF	= 1<<3;	RF_S1 =	1<<2;	RF_S2 =	1<<3;	RF_S3 =	1<<3;	RF_S4 = 1<<3;	RF_S5 =	0<<3;	RF_S6 =	0<<2;	RF_S7 =	0<<3; }break;
		case 5:{ RF	= 1<<3;	RF_S1 =	1<<2;	RF_S2 =	1<<3;	RF_S3 =	1<<3;	RF_S4 = 1<<3;	RF_S5 =	1<<3;	RF_S6 =	0<<2;	RF_S7 =	0<<3; }break;
		case 6:{ RF	= 1<<3;	RF_S1 =	1<<2;	RF_S2 =	1<<3;	RF_S3 =	1<<3;	RF_S4 = 1<<3;	RF_S5 =	1<<3;	RF_S6 =	1<<2;	RF_S7 =	0<<3; }break;
		case 7:{ RF	= 1<<3;	RF_S1 =	1<<2;	RF_S2 =	1<<3;	RF_S3 =	1<<3;	RF_S4 = 1<<3;	RF_S5 =	1<<3;	RF_S6 =	1<<2;	RF_S7 =	1<<3; }break;
		default	:	break;
	} 
}
//------------------------------------------------------------------------------------
//函 数 名：RF_Power
//功能描述：ms级延时
//函数说明：
//调用函数：无
//全局变量：无
//输    入：cPower_Level
//返    回：无
//设 计 者：Kanghekai				时间：
//修 改 者：						时间：
//版    本：
//------------------------------------------------------------------------------------
void Audio_Lever(uchar cAF_Level)
{
	switch(cAF_Level)
	{
		case 0:{ AF	= 1<<3;	AF_S14 = 0<<3;	AF_S13 = 0<<2;	AF_S12 = 0<<3;	AF_S11 = 0<<0;	AF_S10 = 0<<1;	AF_S9 =	0<<2;	AF_S8 =	0<<2; }break;
		case 1:{ AF	= 1<<3;	AF_S14 = 1<<3;	AF_S13 = 0<<2;	AF_S12 = 0<<3;	AF_S11 = 0<<0;	AF_S10 = 0<<1;	AF_S9 =	0<<2;	AF_S8 =	0<<2; }break;
		case 2:{ AF	= 1<<3;	AF_S14 = 1<<3;	AF_S13 = 1<<2;	AF_S12 = 0<<3;	AF_S11 = 0<<0;	AF_S10 = 0<<1;	AF_S9 =	0<<2;	AF_S8 =	0<<2; }break;
		case 3:{ AF	= 1<<3;	AF_S14 = 1<<3;	AF_S13 = 1<<2;	AF_S12 = 1<<3;	AF_S11 = 0<<0;	AF_S10 = 0<<1;	AF_S9 =	0<<2;	AF_S8 =	0<<2; }break;
		case 4:{ AF	= 1<<3;	AF_S14 = 1<<3;	AF_S13 = 1<<2;	AF_S12 = 1<<3;	AF_S11 = 1<<0;	AF_S10 = 0<<1;	AF_S9 =	0<<2;	AF_S8 =	0<<2; }break;
		case 5:{ AF	= 1<<3;	AF_S14 = 1<<3;	AF_S13 = 1<<2;	AF_S12 = 1<<3;	AF_S11 = 1<<0;	AF_S10 = 1<<1;	AF_S9 =	0<<2;	AF_S8 =	0<<2; }break;
		case 6:{ AF	= 1<<3;	AF_S14 = 1<<3;	AF_S13 = 1<<2;	AF_S12 = 1<<3;	AF_S11 = 1<<0;	AF_S10 = 1<<1;	AF_S9 =	1<<2;	AF_S8 =	0<<2; }break;
		case 7:{ AF	= 1<<3;	AF_S14 = 1<<3;	AF_S13 = 1<<2;	AF_S12 = 1<<3;	AF_S11 = 1<<0;	AF_S10 = 1<<1;	AF_S9 =	1<<2;	AF_S8 =	1<<2; }break;
		default	:	break;
	} 
}
//------------------------------------------------------------------------------------
//函 数 名：Display_Freq_Num
//功能描述：ms级延时
//函数说明：
//调用函数：无
//全局变量：无
//输    入：Tus
//返    回：无
//设 计 者：Kanghekai				时间：
//修 改 者：						时间：
//版    本：
//------------------------------------------------------------------------------------
void Display_Freq_Num(uchar cAddress, uchar cNum)
{
	uchar i;
	if(cAddress == 0)
	{
/*		for(i=0;i<4;i++)
		{
			if(i == 2)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | RF);
			else if(i == 3)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | RF_S1 |RF_S2);
			else
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i]);
		}
*/
		Write_1621(cAddress*4+0, FREQ_NUM[cNum*4+0]);
		Write_1621(cAddress*4+1, FREQ_NUM[cNum*4+1]);
		Write_1621(cAddress*4+2, FREQ_NUM[cNum*4+2] | RF);
		Write_1621(cAddress*4+3, FREQ_NUM[cNum*4+3] | RF_S1 |RF_S2);

	}
	else if(cAddress == 2)
	{
/*		for(i=0;i<4;i++)
		{
			if(i == 2)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | RF_S3);
			else if(i == 3)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | RF_S4 | Dot);
			else
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i]);
		}
*/
		Write_1621(cAddress*4+0,FREQ_NUM[cNum*4+0]);
		Write_1621(cAddress*4+1,FREQ_NUM[cNum*4+1]);
		Write_1621(cAddress*4+2,FREQ_NUM[cNum*4+2] | RF_S3);
		Write_1621(cAddress*4+3,FREQ_NUM[cNum*4+3] | RF_S4 | Dot);
	}
	else if(cAddress == 3)
	{
/*		for(i=0;i<4;i++)
		{
			if(i == 2)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | RF_S5);
			else if(i == 3)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | RF_S6 | RF_S7);
			else
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i]);
		}
*/
		Write_1621(cAddress*4+0,FREQ_NUM[cNum*4+0]);
		Write_1621(cAddress*4+1,FREQ_NUM[cNum*4+1]);
		Write_1621(cAddress*4+2,FREQ_NUM[cNum*4+2] | RF_S5);
		Write_1621(cAddress*4+3,FREQ_NUM[cNum*4+3] | RF_S6 | RF_S7);
	}
	else if(cAddress == 4)
	{
/*		for(i=0;i<4;i++)
		{
			if(i == 2)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | K4);
			else if(i == 3)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | AF_S13 | AF_S14);
			else
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i]);
		}
*/
		Write_1621(cAddress*4+0,FREQ_NUM[cNum*4+0]);
		Write_1621(cAddress*4+1,FREQ_NUM[cNum*4+1]);
		Write_1621(cAddress*4+2,FREQ_NUM[cNum*4+2] | K4);
		Write_1621(cAddress*4+3,FREQ_NUM[cNum*4+3] | AF_S13 | AF_S14);
	}
	else if(cAddress == 5)
	{
/*		for(i=0;i<4;i++)
		{
			if(i == 2)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | MHz);
			else if(i == 3)
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i] | AF_S8	| AF);
			else
				Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i]);
		}
*/
		Write_1621(cAddress*4+0,FREQ_NUM[cNum*4+0]);
		Write_1621(cAddress*4+1,FREQ_NUM[cNum*4+1]);
		Write_1621(cAddress*4+2,FREQ_NUM[cNum*4+2] | MHz);
		Write_1621(cAddress*4+3,FREQ_NUM[cNum*4+3] | AF_S8	| AF);

	}
/*	else if(cAddress == 6)
	{
				Write_1621(24,0 | MHz);
				Write_1621(25,0 | AF_S8	| AF);
	}
	else if(cAddress == 7)
	{
				Write_1621(26,0 | MHz);
				Write_1621(27,0 | AF_S8	| AF);
	}
	else if(cAddress == 8)
	{
				Write_1621(28,0 | MHz);
				Write_1621(29,0 | AF_S8	| AF);
	}
*/	else if(cAddress == 9)
	{
//		Write_1621(30,0 | MHz);
		Write_1621(31,0 | AF_S12 | AF_S11 | AF_S10 | AF_S9);
	}
	else
	{
		for(i=0;i<4;i++)
		{
			Write_1621(cAddress*4+i,FREQ_NUM[cNum*4+i]);
		}
	}
}