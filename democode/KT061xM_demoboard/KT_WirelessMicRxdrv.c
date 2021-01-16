///**********************************************************************
//  File Name: KT_WirelessMicRxdrv.c
//  Function:  KT Wireless Mic Receiver Products Driver For Customer
///**********************************************************************
//        Revision History
//	Version	Date		Description
//	V1.0	2012-06-12	Initial draft
//	V1.1	2012-07-04	The function Calc_chanreg is modified.
//	V1.2	2012-08-02	The function KT_WirelessMicRx_Set_XTAL is added.
//	V1.3	2012-08-20	The function KT_WirelessMicRx_Init is modified.
//	V1.4	2012-11-30	Modified "KT_WirelessMicRx_UHFTune".
//	V1.5	2012-12-17	"KT_WirelessMicRx_SQUEAL_Inhibition" is added.
//	V1.6	2012-12-27	Modified "KT_WirelessMicRx_CheckAutomute"(a bug of automute).
//						Modified "KT_WirelessMicRx_CheckAUXCh".
//  V1.7	2013-03-29	A bug in function KT_WirelessMicTx_Calc_ChanReg is fixed. 
//						Modified automute configuration.
#include <KT_WirelessMicRxdrv.h>
#include <interface.h>

uchar bChannel;
extern uchar xdata K4;		

#if CHANNEL_NUMBER ==2
uchar 	CURRENT_VOL[CHANNEL_NUMBER]; //= {PRESET_VOL,PRESET_VOL};
uchar	Flag_PILOT[CHANNEL_NUMBER]		=	{ NON_PILOT,	NON_PILOT	};
uchar	Flag_PKGSYNC[CHANNEL_NUMBER]	=	{ NON_PKGSYNC,	NON_PKGSYNC	};
uchar	Flag_AUTOMUTE[CHANNEL_NUMBER]	=	{ AUTOMUTE,		AUTOMUTE	};
uchar	Flag_BURSTDATA[CHANNEL_NUMBER]	=	{ 0,			0			};
uchar	Flag_SQUEAL[CHANNEL_NUMBER]		=	{ NON_SQUEAL,	NON_SQUEAL	};
#else
uchar 	CURRENT_VOL[CHANNEL_NUMBER]; //= {PRESET_VOL,PRESET_VOL};
uchar	Flag_PILOT[CHANNEL_NUMBER]		=	NON_PILOT;
uchar	Flag_PKGSYNC[CHANNEL_NUMBER]	=	NON_PKGSYNC;
uchar	Flag_AUTOMUTE[CHANNEL_NUMBER]	=	AUTOMUTE;
uchar	Flag_BURSTDATA[CHANNEL_NUMBER]	=	0;
uchar	Flag_SQUEAL[CHANNEL_NUMBER]		=	NON_SQUEAL;
#endif
//------------------------------------------------------------------------------------
//函 数 名：KT_Bus_Write
//功能描述：总线写程序																
//函数说明：																		
//调用函数：I2C_Word_Write()														
//全局变量：无																		
//输    入：uchar Register_Address, uint Word_Data									
//返    回：无																		
//设 计 者：Kanghekai				时间：2012-06-10								
//修 改 者：Kanghekai				时间：2012-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
void KT_Bus_Write(uchar Register_Address, uint Word_Data)
{
	I2C_Word_Write(KTWirelessMicRxw_address,Register_Address,Word_Data);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_Bus_Read
//功能描述：总线读程序
//函数说明：
//调用函数：I2C_Word_Read()															
//全局变量：无
//输    入：uchar Register_Address													
//返    回：I2C_Word_Read(KTWirelessMicRxr_address, Register_Address)				
//设 计 者：Kanghekai				时间：2012-06-10								
//修 改 者：Kanghekai				时间：2012-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
uint KT_Bus_Read(uchar Register_Address)
{
	return( I2C_Word_Read(KTWirelessMicRxr_address, Register_Address) );
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_PreInit									 			
//功能描述：芯片工作始化程序														
//函数说明：判断是否芯片是否正常上电，I2C总线是否读写正常							
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：INIT_FAIL_TH															
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：Kanghekai				时间：2012-06-10								
//修 改 者：Kanghekai				时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicRx_PreInit(void)			  
{
	uint regx;
	uchar i;

	for (i=0;i<INIT_FAIL_TH;i++)
	{
		regx = KT_Bus_Read(0x01);						//Read Manufactory ID 
		Delay_ms(10);
	  	if (regx == 0x4B54)
			return(1);
	}
	return(0);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Init												 	
//功能描述：芯片初始化程序															
//函数说明：																		
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：无																		
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：YANG Pei				时间：2012-06-10								
//修 改 者：KANG Hekai				时间：2013-03-29								
//版    本：V1.6
//			V1.7	修改automute SNR门限，修改automute_sel=0;																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicRx_Init(void)
{
	uint regx;

	KT_WirelessMicRx_Volume(0);

	regx=KT_Bus_Read(0x07);
	KT_Bus_Write( 0x07, regx & 0xFFF7 );					//automute_dsp=0

	regx=KT_Bus_Read(0x10);
	KT_Bus_Write( 0x10, regx & 0xFEFF );					//pn_en=0
	regx=KT_Bus_Read(0x0D);
	KT_Bus_Write( 0x0D, regx | 0x0080 );					//vref_mon_en=1
	regx=KT_Bus_Read(0x28);
	KT_Bus_Write( 0x28, regx | 0x8000 );					//dcoc_dis=1
	regx=KT_Bus_Read(0x1D);
	KT_Bus_Write( 0x1D, (regx & 0x0F00) | ((DEEMPHASIS_DIS) << 15)| (AU_GAIN << 12) | (AFC_EN<<7) | (1<<6) | (AFC_RNG<<4) );

	regx = KT_Bus_Read(0x1F);
	KT_Bus_Write( 0x1F, (regx & 0x0FFF) | (EXP_DIS << 15) | (EXP_TC << 12)| (THD_POWER << 10) | (TREBLE_BOOST << 2) | BASS_BOOST );

#ifdef AUTOMUTE_EN
//	regx=KT_Bus_Read(0x2C);
//	KT_Bus_Write(0x2C,regx | 0x0800);					//automute_sel=1
	KT_Bus_Write(0x1C,0x6058);							//automute_snr_highth=0x60; automute_snr_lowth=0x58;
//	KT_Bus_Write(0x1C,0x5850);							//automute_snr_highth=0x50; automute_snr_lowth=0x48;
	regx=KT_Bus_Read(0x1A);
	KT_Bus_Write(0x1A,regx | 0x2000);					//automute_snr_en=1
	regx=KT_Bus_Read(0x2C);
	KT_Bus_Write(0x2C, (regx & 0x0800) | 0x8090);		//fast_rssi_en=1; fast_rssi_mute_thd=0x90
	KT_Bus_Write(0x1B,0x0CC6);							//super_rssi_cfg
	regx=KT_Bus_Read(0x1A);
	KT_Bus_Write(0x1A,regx | 0x4000);					//automute_super_rssi_en=1,automute_pilot_en=0	关导频检测
//	KT_Bus_Write(0x1A,regx | 0xC000);					//automute_super_rssi_en=1,automute_pilot_en=1  开导频检测
	regx=KT_Bus_Read(0x1A);
	KT_Bus_Write(0x1a,regx & 0xFF7F);					//automute_dis=0
	regx=KT_Bus_Read(0x18);
	KT_Bus_Write(0x18, regx | 0x0010);					//auto_mute_int_en=1
#endif

#ifdef INTERRUPT_EN
	regx=KT_Bus_Read(0x17);
	KT_Bus_Write( 0x17,(regx & 0xE7FF) | (0 << 11) );	//gpio1_set=00
	regx=KT_Bus_Read(0x18);
	KT_Bus_Write( 0x18, (regx & 0x3FFF) | (INT_EN << 15) | (INT_LVL << 14) | (PILOT_INT_EN << 2) );
#endif

#ifdef KT0616M
//AUX CHANNEL
	regx=KT_Bus_Read(0x18);
	KT_Bus_Write( 0x18, (regx & 0xFFF6) | (PKGSYNC_INT_EN << 3) | BURSTDATA_INT_EN );
	regx=KT_Bus_Read(0x20);
	KT_Bus_Write( 0x20, (regx & 0x0FFF) | (AUXDATA_EN << 15) | (AUX_FEDV << 12) );
	regx=KT_Bus_Read(0x17);
	KT_Bus_Write( 0x21, (regx & 0x00FF) | (AUX_CARRY_NUM << 12) | (AUX_CARRY_NUM_TH << 8) );
	KT_Bus_Write( 0x22, (PKG_SYNC_NUM << 4) | BYTE_LOSTSYNC_NUM );

//SQUEAL
#ifdef SQUEAL_EN
	regx=KT_Bus_Read(0x18);
	KT_Bus_Write( 0x18, (regx & 0xFFDF) | (SQUEAL_INT_EN << 5) );
	KT_Bus_Write( 0x24, (SQUEAL_DIS << 15) | (FNOTCH_NOTRESP_TH << 14) | (N_OCTAVE << 12) | (FFT_R_TH << 8) | (FRAME_NUM_TH << 4) | (NOTCH_CTRL_BY_EXT << 3) );
	KT_Bus_Write( 0x2F, (PMAX_HITH << 12) | (PMAX_LOWTH << 8) | (FDIFF_HITH << 4) | FDIFF_LOWTH );
#endif

#endif
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Standby											 	
//功能描述：待机程序																
//函数说明：																		
//调用函数：KT_Bus_Read()、KT_Bus_Write()、Delay_ms()								
//全局变量：																		
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：YANG Pei				时间：2012-06-10								
//修 改 者：YANG Pei				时间：2012-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicRx_Standby(void)
{
	uint regx;

	regx = KT_Bus_Read(0x07);
	KT_Bus_Write(0x07, regx | (1 << 15));      					//Write Standby bit to 1
	Delay_ms(20);

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_WakeUp												 	
//功能描述：唤醒程序																
//函数说明：脱离待机模式															
//调用函数：KT_Bus_Read(); KT_Bus_Write(); KT_WirelessMicRx_Init(); Delay_ms();					
//全局变量：																		
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：YANG Pei				时间：2012-06-10								
//修 改 者：YANG Pei				时间：2012-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicRx_WakeUp(void)
{
	uint regx;

	regx = KT_Bus_Read(0x07);
	KT_Bus_Write(0x07, regx & 0x7FFF );      			//Write Standby bit to 0
	Delay_ms(50);
	KT_WirelessMicRx_Init();

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Volume												 	
//功能描述：输出音量调节
//函数说明：cVolume围为0-31，共32档, 其中0为静音									
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：cVolume																
//返    回：正确：1					错误：0											
//设 计 者：YANG Pei				时间：2012-06-10								
//修 改 者：YANG Pei				时间：2012-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicRx_Volume(uchar cVolume)
{
	uint regx;

	regx = KT_Bus_Read(0x1A);
	KT_Bus_Write(0x1A, (regx & 0xFFE0) | ( (uint)cVolume ));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Calc_ChanReg									
//功能描述：晶体的频率控制字计算												
//函数说明：输入以KHz为单位的VCO震荡频率;											
//			计算结果存在*chan_ptr,*chan_frac_ptr,*chan_frac_msb_ptr中				
//调用函数：KT_Bus_Read()、KT_Bus_Write()、Delay_ms()								
//全局变量：																		
//输    入：Freq （输入以KHz为单位的VCO频率）										
//返    回：正确：1	错误：0															
//设 计 者：YANG Pei					时间：2012-04-19							
//修 改 者：KANG Hekai					时间：2013-03-29							
//版    本：V1.1																	
//			V1.7	修改余数<40或大于xtal-40的bug																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicRx_Calc_ChanReg(long Freq, uint *chan_ptr, int *chan_frac_ptr, uchar *chan_frac_msb_ptr, uint xtal_freq)
{
	*chan_ptr = Freq / xtal_freq;
	Freq = Freq % xtal_freq; 
	*chan_frac_ptr= (Freq<<16)/xtal_freq;
	if ((Freq <= 40) && (Freq >=0))
	{
		*chan_frac_ptr = 0xffff;
		*chan_frac_msb_ptr =3;
	}
	else if ((Freq < xtal_freq ) && (Freq >= xtal_freq - 40))
	{
		(*chan_ptr)++; 
		*chan_frac_ptr = 0xffff;
		*chan_frac_msb_ptr =3;
	}
	else if ((Freq>=(xtal_freq/2-40)) && (Freq<=(xtal_freq/2+40)))
	{
		*chan_frac_ptr = 0x7FFF;
		*chan_frac_msb_ptr = 0;
	}
	else if (Freq > (xtal_freq>>1))
	{
		(*chan_ptr)++; 
		*chan_frac_msb_ptr = 3;
	}
	else	
	{
		*chan_frac_msb_ptr = 0;
	}
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_UHFTune												
//功能描述：UHF接收频率设置函数														
//函数说明：输入以KHz为单位的接收频率，												
//调用函数：KT_WirelessMicRx_Calc_ChanReg(); KT_Bus_Write(); Delay_ms()；
//			KT_WirelessMicRx_SW_XTAL_Freq(); Delay_ms();							
//全局变量：																		
//输    入：Freq （输入以KHz为单位的接收频率），chan_cfg1,chan_cfg2					
//返    回：频率控制字的整数部分									
//设 计 者：YANG pei					时间：2012-06-12							
//修 改 者：Kang hekai					时间：2012-11-30							
//版    本：V1.1																	
//------------------------------------------------------------------------------------
uint KT_WirelessMicRx_UHFTune(long Freq,uchar chan_cfg1,uchar chan_cfg2)
{
	uint chan0,chan1;
	int chan_frac0,chan_frac1;
	uchar chan_frac_msb0,chan_frac_msb1;
	int mod0,mod1,mod2,mod3;

	Freq = Freq << 1;

#ifdef XTAL_24M_ONLY
	KT_WirelessMicRx_Calc_ChanReg(Freq+250, &chan0, &chan_frac0, &chan_frac_msb0,24000);
#endif
#ifdef XTAL_24P576M_ONLY
	KT_WirelessMicRx_Calc_ChanReg(Freq+250, &chan0, &chan_frac0, &chan_frac_msb0,24576);
#endif
#ifdef XTAL_DUAL
	KT_WirelessMicRx_Calc_ChanReg(Freq, &chan0, &chan_frac0, &chan_frac_msb0,24000);
	KT_WirelessMicRx_Calc_ChanReg(Freq, &chan1, &chan_frac1, &chan_frac_msb1,24576);
	mod0=chan_frac0;
	mod1=chan_frac1;
	mod2=chan_frac0<<1;
	mod3=chan_frac1<<1;
	if (mod0 < 0) 
		mod0=~mod0;			 //mod0=abs(mod0);
	if (mod1 < 0)
		mod1=~mod1;			 //mod1=abs(mod1);
	if (mod2 < 0)
		mod2=~mod2;			 //mod2=abs(mod2);
	if (mod3 < 0)
		mod3=~mod3;			 //mod3=abs(mod3);
	if (mod2 < mod0)
		mod0 = mod2;
	if (mod3 < mod1)
		mod1 = mod3;
	if (mod0<mod1)
	{
		KT_WirelessMicRx_SW_XTAL_Freq(XTAL_24P576M_FREQ);
		KT_WirelessMicRx_Calc_ChanReg(Freq+250, &chan0, &chan_frac0, &chan_frac_msb0,24576);
	}
	else
	{
		KT_WirelessMicRx_SW_XTAL_Freq(XTAL_24M_FREQ);
		KT_WirelessMicRx_Calc_ChanReg(Freq+250, &chan0, &chan_frac0, &chan_frac_msb0,24000);
	}
#endif

	KT_Bus_Write(0x0B,(chan0 << 9)| (1<<7) | (chan_cfg1 <<4) | (chan_cfg2 <<2) | chan_frac_msb0);
	KT_Bus_Write(0x0C, 0);
	KT_Bus_Write(0x0C, chan_frac0);	
	Delay_ms(50);
	return(chan0);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Tune												 	
//功能描述：接收频率设置函数														
//函数说明：输入以KHz为单位的接收频率，												
//调用函数：KT_WirelessMicRx_Volume(); KT_WirelessMicRx_UHFTune();
//			KT_WirelessMicRx_Band_Cali_Res();								
//全局变量：bChannel;Flag_PILOT; Flag_AUTOMUTE; Flag_PKGSYNC; Flag_BURSTDATA; Flag_SQUEAL															
//输    入：Freq （输入以KHz为单位的接收频率）										
//返    回：正确：1					错误：0											
//设 计 者：Yangpei					时间：2012-06-10								
//修 改 者：KANG Hekai				时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicRx_Tune(long Freq) 		//in KHz
{
	uint chan;
	uchar chan_cfg1,chan_cfg2;
	uint regx;

	Flag_PILOT[bChannel] = NON_PILOT;
#ifdef AUTOMUTE_EN
	Flag_AUTOMUTE[bChannel] = AUTOMUTE;
#endif

#ifdef KT0616M
	Flag_PKGSYNC[bChannel] = NON_PKGSYNC;
	Flag_BURSTDATA[bChannel] = 0;
#ifdef SQUEAL_EN
	Flag_SQUEAL[bChannel] = NON_SQUEAL;
#endif
#endif

	KT_WirelessMicRx_Volume(0);
	chan=KT_WirelessMicRx_UHFTune(Freq,4,1);
	regx = KT_WirelessMicRx_Band_Cali_Res();
//	if ((regx == 0)||(regx==63)) return(0);

	regx = regx * 27 + 2000;
	regx = (regx / chan)>>1;
	if (regx <13)
		{chan_cfg1 = 0; chan_cfg2 = 0;}
	else if (regx <16)
		{chan_cfg1 = 1;chan_cfg2 = 0;}
	else if (regx <20)
		{chan_cfg1 = 2;chan_cfg2 = 1;}
	else if (regx <24)
		{chan_cfg1 = 3;chan_cfg2 = 1;}
	else if (regx <30)
		{chan_cfg1 = 4;chan_cfg2 = 1;}
	else if (regx <36)
		{chan_cfg1 = 5;chan_cfg2 = 2;}
	else if (regx <44)
		{chan_cfg1 = 6;chan_cfg2 = 2;}
	else
		{chan_cfg1 = 7;chan_cfg2 = 3;}

	KT_WirelessMicRx_UHFTune(Freq,chan_cfg1,chan_cfg2);
//	KT_WirelessMicRx_Volume(PRESET_VOL);

	return(1);	
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Band_Cali_Res										 	
//功能描述：电感测试程序															
//函数说明：用于判断当前电感值是否合适												
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：无																		
//返    回：(regB & 0x01F8) >>3 （返回值范围0-63）									
//设 计 者：KANG Hekai				时间：2012-06-10								
//修 改 者：KANG Hekai				时间：2012-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicRx_Band_Cali_Res(void)
{
	uint regE;

	regE = KT_Bus_Read(0x0E);

	return( (regE & 0x01F8) >>3 );	
}
//------------------------------------------------------------------------------------
//函 数 KT_WirelessMicRx_Set_XTAL
//功能描述：设置晶体频率
//函数说明：设置晶体频率
//调用函数：KT_Bus_Read();KT_Bus_Write;KT_WirelessMicRx_Set_Pilot_Freq();
//全局变量：
//输    入：xtal_sel: 0 or 1
//返    回：成功：1； 失败：0
//设 计 者：KANG Hekai				时间：2012-08-02
//修 改 者：KANG Hekai				时间：2012-08-02
//版    本：V1.2
//------------------------------------------------------------------------------------
uchar KT_WirelessMicRx_Set_XTAL(bit bXtal_Sel)
{
	uint regx;

	KT_WirelessMicRx_Standby();
	regx=KT_Bus_Read(0x07);
	KT_Bus_Write( 0x07, (regx & 0xFFDF) | ((uchar)bXtal_Sel << 5) );
	Delay_ms(50);
	KT_WirelessMicRx_WakeUp();

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SW_XTAL_Freq										 	
//功能描述：晶体切换频率															
//函数说明：切换晶体												
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：xtal_sel: 0 or 1														
//返    回：成功：1； 失败：0 												
//设 计 者：YANG Pei			时间：2012-06-10								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicRx_SW_XTAL_Freq(bit bXtal_Sel)
{
	uint regx;

	regx = KT_Bus_Read(0x07);
	
	if ((regx & 0x0020)==0)									//bXtal_Sel=0
	{
		if (bXtal_Sel==1)
		{
			KT_WirelessMicRx_Set_XTAL(bXtal_Sel);
		}
	}
	else
	{
		if (bXtal_Sel==0)
		{
			KT_WirelessMicRx_Set_XTAL(bXtal_Sel);
		}
	}
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_CheckPilot										 	
//功能描述：导频信号状态变化监测程序															
//函数说明：												
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：xtal_sel: 0 or 1														
//返    回：成功：1； 失败：0 												
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
void  KT_WirelessMicRx_CheckPilot(void)
{
	uint regx;

	regx = KT_Bus_Read(0x19);
	if( (regx & (PILOT_INT << 2)) == (PILOT_INT << 2) )					//pilot int
	{
		if( (regx & (PILOT_STATUS <<10)) == (PILOT_STATUS <<10) )
			Flag_PILOT[bChannel] = PILOT;
		else
			Flag_PILOT[bChannel] = NON_PILOT;
		KT_Bus_Write( 0x19, regx & 0xFFFB );
	}
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_CheckAutomute										 	
//功能描述：自动静音状态变化监测程序															
//函数说明：												
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：														
//返    回： 												
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-12-26								
//版    本：V1.3																	
//------------------------------------------------------------------------------------
void  KT_WirelessMicRx_CheckAutomute(void)
{
	uint regx;

	regx = KT_Bus_Read(0x19);
	if( (regx & (AUTOMUTE_INT << 4)) == (AUTOMUTE_INT << 4) )					//automute int
	{
		KT_Bus_Write( 0x19, regx & 0xFFEF );
		regx = KT_Bus_Read(0x19);
		if( (regx & (AUTOMUTE_STATUS <<12)) == (AUTOMUTE_STATUS <<12) )
			Flag_AUTOMUTE[bChannel] = AUTOMUTE;
		else
			Flag_AUTOMUTE[bChannel] = NON_AUTOMUTE;
	}
}

#ifdef KT0616M
//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_CheckAUXCH										 	
//功能描述：辅助信道状态变化监测程序															
//函数说明：												
//调用函数：KT_Bus_Read();KT_Bus_Write();															
//全局变量：																		
//输    入：														
//返    回： 												
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-12-26								
//版    本：V1.3																	
//------------------------------------------------------------------------------------
void  KT_WirelessMicRx_CheckAUXCH(void)
{
	uint regx;

	regx = KT_Bus_Read(0x19);
	if( (regx & BURSTDATA_INT) == BURSTDATA_INT )								//burstdata int
	{
		KT_Bus_Write( 0x19, regx & 0xFFFE );
		Flag_BURSTDATA[bChannel] = 1;
	}
	regx = KT_Bus_Read(0x19);
	if( (regx & (PKGSYNC_INT << 3)) == (PKGSYNC_INT << 3) )						//pkg_sync int
	{
		KT_Bus_Write( 0x19, regx & 0xFFF7 );
		regx = KT_Bus_Read(0x19);
		if( (regx & (PKGSYNC_STATUS << 11)) == (PKGSYNC_STATUS << 11) )
			Flag_PKGSYNC[bChannel] = PKGSYNC;
		else
			Flag_PKGSYNC[bChannel] = NON_PKGSYNC;
	}
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_CheckSqueal										 	
//功能描述：啸叫抑制状态监测程序															
//函数说明：												
//调用函数：KT_Bus_Read();KT_Bus_Write();															
//全局变量：																		
//输    入：														
//返    回： 												
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
/*#ifdef SQUEAL_EN
void  KT_WirelessMicRx_CheckSqueal(void)
{
	uint regx;

	regx = KT_Bus_Read(0x19);
	if( (regx & (SQUEAL_INT << 5)) == (SQUEAL_INT << 5) )						//squeal int
	{
		KT_Bus_Write( 0x19, regx & 0xFFDF );
		if(Flag_SQUEAL[bChannel] < 4)
			Flag_SQUEAL[bChannel] ++;
	}
}
#endif
*/
#ifdef SQUEAL_EN
void  KT_WirelessMicRx_CheckSqueal(void)
{
	uint regx;

	regx = KT_Bus_Read(0x19);
	if( (regx & (SQUEAL_INT << 5)) == (SQUEAL_INT << 5) )						//squeal int
	{
		KT_Bus_Write( 0x19, regx & 0xFFDF );
		Flag_SQUEAL[bChannel] = SQUEAL;
	}
	else
		Flag_SQUEAL[bChannel] = NON_SQUEAL;
}
#endif
#endif

#ifdef INTERRUPT_EN
//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Interrupt											 	
//功能描述：中断响应程序																
//函数说明：																		
//调用函数：KT_WirelessMicRx_CheckPilot();KT_WirelessMicRx_CheckAutomute();
//			KT_WirelessMicRx_CheckAUXCH();KT_WirelessMicRx_CheckSqueal();								
//全局变量：																		
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
void KT_WirelessMicRx_Interrupt(void)
{
	KT_WirelessMicRx_CheckPilot();

#ifdef AUTOMUTE_EN
	KT_WirelessMicRx_CheckAutomute();
#endif

#ifdef KT0616M
	KT_WirelessMicRx_CheckAUXCH();

	#ifdef SQUEAL_EN
		KT_WirelessMicRx_CheckSqueal();
	#endif
#endif
}
#endif

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetBurstData										 	
//功能描述：获取辅助信道突发模式数据															
//函数说明：												
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：辅助信道突发模式数据														
//返    回：成功：1； 失败：0 												
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
uint  KT_WirelessMicRx_GetBurstData(void)
{
	Flag_BURSTDATA[bChannel] = 0;
	return(KT_Bus_Read(0x02));
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Automute										 	
//功能描述：自动静音控制															
//函数说明：												
//调用函数：KT_WirelessMicRx_CheckAutomute();KT_WirelessMicRx_Volume();															
//全局变量：																		
//输    入：														
//返    回： 												
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：ZHOU Dongfeng		时间：2012-12-17								
//版    本：V1.2	
//			V1.3																
//------------------------------------------------------------------------------------
void KT_WirelessMicRx_Automute(void)
{

#if CHANNEL_NUMBER ==2
	static uchar unmute_num[CHANNEL_NUMBER]={0,0};
	static uchar mute_num[CHANNEL_NUMBER]={0,0};
#else
	static uchar unmute_num[CHANNEL_NUMBER]=0,mute_num[CHANNEL_NUMBER]=0;
#endif

	KT_WirelessMicRx_CheckAutomute();
	if( Flag_AUTOMUTE[bChannel] == AUTOMUTE )
	{
		unmute_num[bChannel]=0;
		if (mute_num[bChannel]<=2)
		{
			mute_num[bChannel]++;
		}
		if (mute_num[bChannel]>2)
		{
			KT_WirelessMicRx_Volume(0);
//			if(bChannel == CH_A)
//				Mute_A = 1;
//			else
//				Mute_B = 1;
		}
	}
	else									 //nomute
	{
		mute_num[bChannel]=0;
		if (unmute_num[bChannel]<=4)
		{
			unmute_num[bChannel]++;
		}
		if (unmute_num[bChannel]>4)
		{
//			if( (KT_WirelessMicRx_GetAFC()<10) && (KT_WirelessMicRx_GetAFC()>-10) )
//			{
//				if(bChannel == CH_A)
//					Mute_A = 0;
//				else
//					Mute_B = 0;
				KT_WirelessMicRx_Volume(CURRENT_VOL[bChannel]);
//			}
//			else
//			{
//				KT_WirelessMicRx_Volume(0);
//				if(bChannel == CH_A)
//					Mute_A = 1;
//				else
//					Mute_B = 1;
//				Flag_AUTOMUTE[bChannel] = AUTOMUTE;
//			}
		}
	}
}
//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SetBass
//功能描述：低音增强设置程序
//函数说明：
//调用函数：KT_Bus_Read()、KT_Bus_Write()
//全局变量：
//输    入：uchar cFlt_Sel
//返    回：无
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
void KT_WirelessMicRx_SetBass(uchar cBass)
{
	uint regx;

	regx = KT_Bus_Read(0x1F);
	KT_Bus_Write(0x1F, (regx & 0xFFFC) | cBass);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SetTreble
//功能描述：高音增强设置程序
//函数说明：
//调用函数：KT_Bus_Read()、KT_Bus_Write()
//全局变量：
//输    入：uchar cFlt_Sel
//返    回：无
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
void KT_WirelessMicRx_SetTreble(uchar cTreble)
{
	uint regx;

	regx = KT_Bus_Read(0x1F);
	KT_Bus_Write(0x1F, (regx & 0xFFF3) | (cTreble << 2));
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SetTHD
//功能描述：THD设置程序
//函数说明：
//调用函数：KT_Bus_Read()、KT_Bus_Write()
//全局变量：
//输    入：uchar cTHD
//返    回：无
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
void KT_WirelessMicRx_SetTHD(uchar cTHD)
{
	uint regx;

	regx = KT_Bus_Read(0x1F);
	KT_Bus_Write(0x1F, (regx & 0xF3FF) | ((uint)cTHD << 10));
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetAF
//功能描述：获取音频幅度
//函数说明：
//调用函数：KT_Bus_Read()
//全局变量：
//输    入：无
//返    回：音频信号强度
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicRx_GetAF(void)
{
	uint regx;

	regx = KT_Bus_Read(0x37);

	return( (regx & 0x0F00) >> 8 );
}


//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetRSSI
//功能描述：获取射频信号强度
//函数说明：
//调用函数：KT_Bus_Read()
//全局变量：
//输    入：无
//返    回：射频信号强度
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicRx_GetRSSI(void)
{
	uchar cRssi;

	cRssi = 0xFF - (KT_Bus_Read(0x3B) & 0x00FF);

	return( cRssi );
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetAFC
//功能描述：
//函数说明：
//调用函数：KT_Bus_Read()
//全局变量：
//输    入：无
//返    回：射频信号强度
//设 计 者：KANG Hekai			时间：2012-08-02								
//修 改 者：KANG Hekai			时间：2012-08-02								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
char KT_WirelessMicRx_GetAFC(void)
{
	uint regx;
	regx = KT_Bus_Read(0x22)>>8;
	return( regx );
}
//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SQUEAL_Inhibition
//功能描述：啸叫抑制
//函数说明：
//调用函数：KT_Bus_Read;KT_Bus_Write;KT_WirelessMicRx_Volume; 
//全局变量：CURRENT_VOL; bChannel;K4;
//输    入：
//返    回：无
//设 计 者：zhoudongfeng			时间：2012-12-18
//修 改 者：						时间：
//版    本：V1.5
//------------------------------------------------------------------------------------
void KT_WirelessMicRx_SQUEAL_Inhibition (void)
{
	uchar j;

	if((KT_Bus_Read(0x19)>>5) & 0x0001 == SQUEAL)  			//是否有啸叫中断
	{
		KT_Bus_Write( 0x19, KT_Bus_Read(0x19) & 0xFFDF );

		CURRENT_VOL[bChannel] = CURRENT_VOL[bChannel]  - 5;
		KT_WirelessMicRx_Volume(CURRENT_VOL[bChannel]);
		Delay_ms(1088);
		K4 = 1<<3;		//显示钥匙标志，表示有啸叫				   
	}
	else												   //没有啸叫中断
	{
		if(CURRENT_VOL[bChannel]  < PRESET_VOL)
		{
			if(KT_WirelessMicRx_GetAF() > 7)
			{
#ifdef SQUEAL_RST							
				KT_Bus_Write( 0x24, (1 << 15) | (FNOTCH_NOTRESP_TH << 14) | (N_OCTAVE << 12) | (FFT_R_TH << 8) | (FRAME_NUM_TH << 4) | (NOTCH_CTRL_BY_EXT << 3) );
				KT_Bus_Write( 0x24, (SQUEAL_DIS << 15) | (FNOTCH_NOTRESP_TH << 14) | (N_OCTAVE << 12) | (FFT_R_TH << 8) | (FRAME_NUM_TH << 4) | (NOTCH_CTRL_BY_EXT << 3) );
#endif													
				for(j=0;j<6;j++)
				{
					CURRENT_VOL[bChannel] = CURRENT_VOL[bChannel]  + 6;
					if(CURRENT_VOL[bChannel]  > PRESET_VOL) CURRENT_VOL[bChannel]  = PRESET_VOL;								
					KT_WirelessMicRx_Volume(CURRENT_VOL[bChannel]);
					Delay_ms(10);
				}			
			}
			K4 = 0<<3;   //去掉钥匙标志，表示没有啸叫
		}					
	}		  
} 