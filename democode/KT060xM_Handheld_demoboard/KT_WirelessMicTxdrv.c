//*********************************************************************
//  File Name: KT_WirelessMicTxdrv.c
//  Function:  KT Wireless Mic Transmitter Products Driver For Customer
//*********************************************************************
//        Revision History
//  Version	Date		Description
//  V1.0	2011-06-10	Initial draft
//	V1.1	2011-08-03	Tune function is modified.
// 	V1.2	2011-09-08	Standby function is modified.
//	V1.3	2011-09-09	Support for 24.576M crystal is added.
//	V1.4	2011-11-29	Remove negative.
//	V1.5	2012-04-06	pre-compile option of TUNE_V1 is added.
//	V1.6	2012-04-19	pre-compile option of XTAL_DUAL, XTAL_24M_ONLY and XTAL_24P576M_ONLY are added.
//	V1.7	2012-05-31	A bug in function Calc_ChanReg_24M and Calc_ChanReg_24P576M is fixed.
//	V1.8	2012-06-21	The function UHFTune and SW_xtal_freq are refactored.
//	V1.9	2012-06-21	The function Calc_chanreg is modified.
//	V2.0	2012-06-30	A bug in function Calc_chanreg is fixed.
//	V2.1	2012-07-10	The function of Init and UHFTune is modified.
//	V2.2	2012-07-13	A bug in UHFTune is fixed.
//	V2.3	2012-08-01	Support KT0606M(Auxchannel).
//						A bug in function KT_WirelessMicTx_Set_XTAL is fixed.
//						Added a pre-compile option of KT_RX. 
//						Added a pre-compile option of OTHER_RX. 
//						Added a pre-compile option of KT0605M. 
//						Added a pre-compile option of KT0606M. 
//						Modified the KT_WirelessMicTx_PreInit function. 
//  V2.4	2012-08-20	The function KT_WirelessMicTx_PowerDownProcedure is added. 
//  V2.5	2013-03-29	A bug in function KT_WirelessMicTx_Calc_ChanReg is fixed. 

#include <KT_WirelessMicTxdrv.h>
#include <interface.h>

//------------------------------------------------------------------------------------
//函 数 名：KT_Bus_Write														 	
//功能描述：总线写程序																
//函数说明：																		
//调用函数：I2C_Word_Write()														
//全局变量：无																		
//输    入：uchar Register_Address, uint Word_Data									
//返    回：无																		
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
void KT_Bus_Write(uchar Register_Address, uint Word_Data)
{
	I2C_Word_Write(KTWirelessMicTxw_address,Register_Address,Word_Data);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_Bus_Read															 	
//功能描述：总线读程序																
//函数说明：																		
//调用函数：I2C_Word_Read()															
//全局变量：无																		
//输    入：uchar Register_Address													
//返    回：I2C_Word_Read(KTWirelessMicTxr_address, Register_Address)				
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
uint KT_Bus_Read(uchar Register_Address)
{
	return( I2C_Word_Read(KTWirelessMicTxr_address, Register_Address) );
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_PreInit									 			
//功能描述：芯片工作始化程序														
//函数说明：判断是否芯片是否正常上电，I2C总线是否读写正常							
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：INIT_FAIL_TH															
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2012-08-01								
//版    本：V2.3																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_PreInit(void)			  
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
//函 数 名：KT_WirelessMicTx_Init												 	
//功能描述：芯片初始化程序															
//函数说明：																		
//调用函数：KT_Bus_Read()、KT_Bus_Write()、KT_WirelessMicTx_Comp_TC();
//			KT_WirelessMicTx_Set_Pilot_Freq();				
//全局变量：无																		
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2012-08-01								
//版    本：V2.3																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Init(void)
{
	uint regx;

	KT_Bus_Write(0x03, 0x0000);      									//Write Standby bit to 0
	Delay_ms(50);

//	regx = KT_Bus_Read(0x1b);
//	KT_Bus_Write(0x1b, regx |0x0800);									//au_topldo_vsel=1, 2.3V
	regx = KT_Bus_Read(0x29);
	KT_Bus_Write(0x29, regx |0x0020);									//lo_topldo_vsel=1, 2.3V

	regx = KT_Bus_Read(0x2a);
	KT_Bus_Write(0x2a, (regx & 0x1fff) | (5<<13));						//lo_sldo_isel=100;

	regx = KT_Bus_Read(0x0d);
	KT_Bus_Write(0x0d, regx & 0xffdf);									//pn_en=0

	KT_WirelessMicTx_Comp_TC(COMPANDOR_TC_12ms);
#ifdef OTHER_RX
	KT_WirelessMicTx_Set_Pilot_Freq(XTAL_24M_FREQ);
#endif

#ifdef KT0606M
	KT_Bus_Write(0x10,0xAAAA);
	KT_Bus_Write(0x11,0x5555);
	KT_Bus_Write(0x17,0x1234);
	KT_Bus_Write(0x20,(AUX_ADDRB << 8) | AUX_ADDRA);
	KT_Bus_Write(0x21,(AUX_ADDRD << 8) | AUX_ADDRC);

	regx=KT_Bus_Read(0x1F);	
	KT_Bus_Write(0x1F, (regx & 0x007F) | REG1F);
#endif
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Standby											 	
//功能描述：待机程序																
//函数说明：																		
//调用函数：KT_Bus_Read()、KT_Bus_Write()、KT_WirelessMicTx_PASW()、Delay_ms()		
//全局变量：																		
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：YANG Pei				时间：2011-09-08								
//版    本：V1.2																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Standby(void)
{
	KT_WirelessMicTx_PowerDownProcedure();
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_WakeUp												 	
//功能描述：唤醒程序																
//函数说明：脱离待机模式															
//调用函数：KT_Bus_Read()、KT_Bus_Write()、KT_WirelessMicTx_Init()					
//全局变量：																		
//输    入：无																		
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-11-29								
//版    本：V1.4																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_WakeUp(void)
{
	uint reg3;

	reg3 = KT_Bus_Read(0x03);
	KT_Bus_Write(0x03, reg3 & 0x7FFF | (WAKEUP << 15));      			//Write Standby bit to 0
	Delay_ms(500);
	KT_WirelessMicTx_Init();

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_PASW												 	
//功能描述：PA打开、关闭程序														
//函数说明：用来控制芯片打开或者关闭PA												
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：bPA_Switch																
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_PASW(bit bPA_Switch)
{
	uint regF;

	regF = KT_Bus_Read(0x0F);
	KT_Bus_Write(0x0F, regF & 0xFFEF | ( (uchar)bPA_Switch << 4 ));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_PAGain												 	
//功能描述：PA增益调整																
//函数说明：cPaGain范围为0-12，共13档												
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：cPaGain																	
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_PAGain(uchar cPaGain)
{
	uint reg3;

	reg3 = KT_Bus_Read(0x03);
	KT_Bus_Write(0x03, reg3 & 0xFFE0 | (cPaGain << 1) | PA_SEL);		

	return(1);	
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Fdev_Monitor										 	
//功能描述：实时频偏读取程序														
//函数说明：读取当前状态的频偏大小													
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：无																		
//返    回：(reg1C & 0xF000) >> 12 （为0-15的整数）									
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicTx_Fdev_Monitor(void)
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);

	return( (reg1C & 0xF000) >> 12 );
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Fdev_Monitor_TC									 	
//功能描述：频偏检测时间常数配置程序												
//函数说明：cFdev_Mon_TC围为0-3，共4档												
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：cFdev_Mon_TC															
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Fdev_Monitor_TC(uchar cFdev_Mon_TC)
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);
	KT_Bus_Write(0x1C, (reg1C & 0xF3FF) | ( (uint)cFdev_Mon_TC << 10 ));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_PreEmpasis_SW										 	
//功能描述：预加重功能打开、关闭程序												
//函数说明：用来控制芯片打开或者关闭预加重功能										
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：cPreEmpasis_Dis															
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_PreEmpasis_SW(uchar cPreEmpasis_Dis)
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);
	KT_Bus_Write(0x1C, (reg1C & 0xFDFF) | ( (uint)cPreEmpasis_Dis << 9 ));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Mic_Sens											 	
//功能描述：Mic灵敏度调整															
//函数说明：cMicSens围为0-15，共16档												
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：cMicSens																
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Mic_Sens(uchar cMicSens)
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);
	KT_Bus_Write(0x1C, (reg1C & 0xFE1F) | ( (uint)cMicSens << 5 ));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Comp_Dis											 	
//功能描述：压扩功能打开、关闭程序													
//函数说明：用来控制芯片打开或者关闭压扩功能										
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：bComp_Dis																
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Comp_Dis(bit bComp_Dis)							
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);
	KT_Bus_Write(0x1C, (reg1C & 0xFFEF) | ( (uchar)bComp_Dis << 4 ));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Comp_TC											 	
//功能描述：压扩时间常数调整														
//函数说明：cComp_TC范围为0-7，共8档												
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：cComp_TC																
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Comp_TC(uchar cComp_TC)
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);
	KT_Bus_Write(0x1C, (reg1C & 0xFFF1) | (cComp_TC << 1));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_MuteSel											 	
//功能描述：静音功能打开、关闭程序													
//函数说明：用来选择打开或者关闭静音功能											
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：bMute_Sel																
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_MuteSel(bit bMute_Sel)
{
	uint reg1C;

	reg1C = KT_Bus_Read(0x1C);
	KT_Bus_Write(0x1C, (reg1C & 0xFFFE) | bMute_Sel);      				//Write Mute bit

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Pilot												 	
//功能描述：32.768KHz倒频信号打开、关闭程序											
//函数说明：用来选择是否使用32.768KHz倒频信号										
//调用函数：KT_Bus_Read()、KT_Bus_Write()、Delay_ms()								
//全局变量：																		
//输    入：bPilot_Dis																
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-08-01								
//版    本：V2.3																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Pilot(bit bPilot_Dis)
{
	uint reg1F;

	reg1F = KT_Bus_Read(0x1F);
#ifdef OTHER_RX
	KT_Bus_Write(0x1F, (reg1F & 0xFFBF) | ( (uchar)bPilot_Dis << 6 ));
#endif
#ifdef KT_RX
	KT_Bus_Write(0x1F, (reg1F & 0x7FFF) | ( (uint)bPilot_Dis << 15 ));
#endif
	Delay_ms(20);

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Pilot_Fdev											 	
//功能描述：32.768KHz倒频信号调制频偏大小设置程序									
//函数说明：cPilot_Fdev围为0-3，共4档												
//调用函数：KT_Bus_Read()、KT_Bus_Write()、Delay_ms()								
//全局变量：																		
//输    入：cPilot_Fdev																
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-08-01								
//版    本：V2.3																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Pilot_Fdev(uchar cPilot_Fdev)
{
	uint reg1F;

	reg1F = KT_Bus_Read(0x1F);
#ifdef OTHER_RX
	KT_Bus_Write(0x1F, (reg1F & 0xFFCF) | (cPilot_Fdev << 4));
#endif
#ifdef KT_RX
	KT_Bus_Write(0x1F, (reg1F & 0xFE7F) | ((uint)cPilot_Fdev << 7));
#endif
	Delay_ms(20);

	return(1);
}

#ifdef OTHER_RX
//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Set_Pilot_Freq										 	
//功能描述：设置导频频率															
//函数说明：设置导频频率												
//调用函数：KT_Bus_Write();															
//全局变量：																		
//输    入：xtal_sel: 0 or 1														
//返    回：成功：1； 失败：0 												
//设 计 者：YANG Pei				时间：2012-04-10								
//修 改 者：YANG Pei				时间：2012-04-10								
//版    本：V1.6																	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicTx_Set_Pilot_Freq(bit bXtal_Sel)
{
	uint regx;
	if (bXtal_Sel==XTAL_24M_FREQ)
	{
		KT_Bus_Write(0x2c, ((PILOT_FREQ<<13) / 9375));
//		regx=KT_Bus_Read(0x2c);
	}
	else
	{
		KT_Bus_Write(0x2c, ((PILOT_FREQ<<13) / 9600));
//		regx=KT_Bus_Read(0x2c);
	}
	return(1);
}
#endif

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_BatteryMeter_SW									 	
//功能描述：电池电压检测功能打开、关闭程序											
//函数说明：选择打开或者关闭电池电压检测功能										
//调用函数：KT_Bus_Read()、KT_Bus_Write()											
//全局变量：																		
//输    入：bBatteryMeter_En														
//返    回：正确：1					错误：0											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_BatteryMeter_SW(bit bBatteryMeter_En)
{
	uint reg24;

	reg24 = KT_Bus_Read(0x24);
	KT_Bus_Write(0x24, (reg24 & 0xEFFF) | ( (uint)bBatteryMeter_En << 12));

	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_BatteryMeter_Read									 	
//功能描述：电池电压读取程序														
//函数说明：读取当前的电池电压，返回值为0-2047的整数								
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：无																		
//返    回：reg7 & 0x07FF（为0-2047的整数）											
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
uint KT_WirelessMicTx_BatteryMeter_Read(void)
{
	uint reg7;

	reg7 = KT_Bus_Read(0x07);

	return( reg7 & 0x07FF );
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_LowVoltageIndicator								 	
//功能描述：低电压指示程序															
//函数说明：将读取到的电压值与LOWVOLTAGE_TH进行比较，低于门限返回1					
//调用函数：KT_WirelessMicTx_BatteryMeter_Read()									
//全局变量：																		
//输    入：无																		
//返    回：电压低：1				电压正常：0										
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_LowVoltageIndicator(void)
{
	if( KT_WirelessMicTx_BatteryMeter_Read() < LOWVOLTAGE_TH )
		return(1);
	else
		return(0);
}
//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Calc_ChanReg									
//功能描述：晶体的频率控制字计算												
//函数说明：输入以KHz为单位的VCO震荡频率;											
//			计算结果存在*chan_ptr,*chan_frac_ptr,*chan_frac_msb_ptr中				
//调用函数：KT_Bus_Read()、KT_Bus_Write()、Delay_ms()								
//全局变量：																		
//输    入：Freq （输入以KHz为单位的VCO频率）										
//返    回：正确：1	错误：0															
//设 计 者：YANG Pei					时间：2012-04-19							
//修 改 者：KANG Hekai					时间：2013-03-29							
//版    本：V2.0																	
//			V2.5	修改余数<40或大于xtal-40的bug																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Calc_ChanReg(long Freq, uint *chan_ptr, int *chan_frac_ptr, uchar *chan_frac_msb_ptr, uint xtal_freq)
{
	*chan_ptr = Freq / xtal_freq;
	Freq = Freq % xtal_freq; 
	*chan_frac_ptr = (Freq << 16) / xtal_freq;
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
	else if ( (Freq >= (xtal_freq / 2 - 40)) && (Freq <= (xtal_freq / 2 + 40)) )
	{
		*chan_frac_ptr = 0x7fff;
		*chan_frac_msb_ptr = 0;
	}
	else if ( Freq > (xtal_freq >> 1) )
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
//函 数 名：KT_WirelessMicTx_UHFTune												
//功能描述：UHF发射频率设置函数														
//函数说明：输入以KHz为单位的发射频率，												
//调用函数：KT_Bus_Read()、KT_Bus_Write()、Delay_ms()								
//全局变量：																		
//输    入：Freq （输入以KHz为单位的发射频率），chan_cfg1,chan_cfg2					
//返    回：正确：频率控制字的整数部分	错误：0										
//设 计 者：Yangpei						时间：2011-08-03							
//修 改 者：YANG Pei					时间：2012-07-13							
//版    本：V2.2																	
//------------------------------------------------------------------------------------
uint KT_WirelessMicTx_UHFTune(long Freq,uchar chan_cfg1,uchar chan_cfg2)
{
	uint chan0,chan1;
	int chan_frac0,chan_frac1;
	uchar chan_frac_msb0,chan_frac_msb1;
	int mod0,mod1,mod2,mod3;

	Freq = Freq << 1;

#ifdef XTAL_24M_ONLY
	KT_WirelessMicTx_Calc_ChanReg(Freq, &chan0, &chan_frac0, &chan_frac_msb0,24000);
#endif
#ifdef XTAL_24P576M_ONLY
	KT_WirelessMicTx_Calc_ChanReg(Freq, &chan0, &chan_frac0, &chan_frac_msb0,24576);
#endif
#ifdef XTAL_DUAL
	KT_WirelessMicTx_Calc_ChanReg(Freq, &chan0, &chan_frac0, &chan_frac_msb0,24000);
	KT_WirelessMicTx_Calc_ChanReg(Freq, &chan1, &chan_frac1, &chan_frac_msb1,24576);
	mod0=chan_frac0;mod1=chan_frac1;
	mod2=chan_frac0<<1;mod3=chan_frac1<<1;
	if (mod0 < 0) mod0=~mod0;			 //mod0=abs(mod0);
	if (mod1 < 0) mod1=~mod1;			 //mod1=abs(mod1);
	if (mod2 < 0) mod2=~mod2;			 //mod2=abs(mod2);
	if (mod3 < 0) mod3=~mod3;			 //mod3=abs(mod3);
	if (mod2 < mod0) mod0 = mod2;
	if (mod3 < mod1) mod1 = mod3;
	if (mod0<mod1)
	{
		KT_WirelessMicTx_SW_XTAL_Freq(XTAL_24P576M_FREQ);
		chan0 = chan1;
		chan_frac0 = chan_frac1;
		chan_frac_msb0 = chan_frac_msb1;
	}
	else 
		KT_WirelessMicTx_SW_XTAL_Freq(XTAL_24M_FREQ);
#endif

	KT_Bus_Write(0x08,(chan0 << 9)| (1<<7) | (chan_cfg1 <<4) | (chan_cfg2 <<2) | chan_frac_msb0); 	//
	KT_Bus_Write(0x09, 0);
	KT_Bus_Write(0x09, chan_frac0);	
	Delay_ms(50);
	return(chan0);
}
//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Tune												 	
//功能描述：发射频率设置函数														
//函数说明：输入以KHz为单位的发射频率，												
//调用函数：KT_Bus_Read()、KT_Bus_Write()、Delay_ms()								
//全局变量：																		
//输    入：Freq （输入以KHz为单位的发射频率）										
//返    回：正确：1					错误：0											
//设 计 者：Yangpei					时间：2011-06-10								
//修 改 者：Yangpei					时间：2011-08-03								
//版    本：V1.1																	
//------------------------------------------------------------------------------------
bit KT_WirelessMicTx_Tune(long Freq) 		//in KHz
{
	uint chan;
	uchar chan_cfg1,chan_cfg2;
	uint regx;
#ifdef TUNE_V1	
	uchar band_sel1,band_sel2,band_sel3;
#endif		
	
	chan=KT_WirelessMicTx_UHFTune(Freq,4,1);
	regx = KT_WirelessMicTx_Band_Cali_Res();
//	if ((regx == 0)||(regx==63)) return(0);

	regx = regx * 27 + 2000;
	regx = (regx / chan)>>1;
	if (regx <13)
	{
		chan_cfg1 = 0; chan_cfg2 = 0;
	}
	else if (regx <16)
	{
		chan_cfg1 = 1;chan_cfg2 = 0;
	}
	else if (regx <20)
	{
		chan_cfg1 = 2;chan_cfg2 = 1;
	}
	else if (regx <24)
	{
		chan_cfg1 = 3;chan_cfg2 = 1;
	}
	else if (regx <30)
	{
		chan_cfg1 = 4;chan_cfg2 = 1;
	}
	else if (regx <36)
	{
		chan_cfg1 = 5;chan_cfg2 = 2;
	}
	else if (regx <44)
	{
		chan_cfg1 = 6;chan_cfg2 = 2;}
	else
	{
		chan_cfg1 = 7;chan_cfg2 = 3;
	}

	KT_WirelessMicTx_UHFTune(Freq,chan_cfg1,chan_cfg2);

#ifdef TUNE_V1	
	KT_WirelessMicTx_UHFTune(Freq-200,chan_cfg1,chan_cfg2);
	band_sel1=KT_WirelessMicTx_Band_Cali_Res();
	KT_WirelessMicTx_UHFTune(Freq+200,chan_cfg1,chan_cfg2);
	band_sel3=KT_WirelessMicTx_Band_Cali_Res();

	if (band_sel1<band_sel3)
		band_sel3=band_sel1-1;
	if (band_sel1-band_sel3>1)
		band_sel1=band_sel3+1;
	for (;;)
	{
		KT_WirelessMicTx_UHFTune(Freq,chan_cfg1,chan_cfg2);
		band_sel2=KT_WirelessMicTx_Band_Cali_Res();
		if ((band_sel2<=band_sel1)&&(band_sel2>=band_sel3))
			break;
	}
#endif		
	return(1);	
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Band_Cali_Res										 	
//功能描述：电感测试程序															
//函数说明：用于判断当前电感值是否合适												
//调用函数：KT_Bus_Read()															
//全局变量：																		
//输    入：无																		
//返    回：(regB & 0x01F8) >>3 （返回值范围0-63）									
//设 计 者：KANG Hekai				时间：2011-06-10								
//修 改 者：KANG Hekai				时间：2011-06-10								
//版    本：V1.0																	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicTx_Band_Cali_Res(void)
{
	uint regB;

	regB = KT_Bus_Read(0x0B);

	return( (regB & 0x01F8) >>3 );	
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_Set_XTAL
//功能描述：设置晶体频率
//函数说明：设置晶体频率
//调用函数：KT_Bus_Read();KT_Bus_Write;KT_WirelessMicTx_Set_Pilot_Freq();
//全局变量：
//输    入：xtal_sel: 0 or 1
//返    回：成功：1； 失败：0
//设 计 者：YANG Pei				时间：2012-04-10
//修 改 者：KANG Hekai				时间：2012-08-01
//版    本：V2.3
//------------------------------------------------------------------------------------
uchar KT_WirelessMicTx_Set_XTAL(bit bXtal_Sel)
{
	uint regx,regy;

	KT_Bus_Write(0x3E,0x8000);				//soft_rst=1
	KT_Bus_Write(0x0E, 0x0002);				//au_rst_bypass=1
	regy = KT_Bus_Read(0x1B);
	KT_Bus_Write(0x1B, regy | 0x0004);		//au_dig_rst=1
	
	regx = KT_Bus_Read(0x3F);
	KT_Bus_Write( 0x3F, (regx & 0xFF7F) | ((uchar)bXtal_Sel << 7) );		//bXtal_Sel=0

	Delay_ms(50);
	KT_Bus_Write(0x1B, regy);				//au_dig_rst=0
	KT_Bus_Write(0x0E, 0x0000);				//au_rst_bypass=0
	KT_Bus_Write(0x3E, 0x0000);				//soft_rst=0
#ifdef OTHER_RX
	KT_WirelessMicTx_Set_Pilot_Freq(bXtal_Sel);		//set pilot frequency
#endif
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_SW_XTAL_Freq											 	 	
//功能描述：切换晶体频率														 	
//函数说明：																 	
//调用函数：KT_Bus_Read;KT_WirelessMicTx_Set_XTAL;												 	
//全局变量：无																 	
//输    入：xtal_sel;														 	
//返    回：无																 	
//设 计 者：KANG Hekai				时间：									 	
//修 改 者：						时间：									 	
//版    本：																 	
//------------------------------------------------------------------------------------
uchar KT_WirelessMicTx_SW_XTAL_Freq(bit bXtal_Sel)
{
	uint regx;

	regx = KT_Bus_Read(0x3F);
	if (bXtal_Sel==1)
	{
		if ((regx & 0x0080)==0)						//bXtal_Sel=0
			KT_WirelessMicTx_Set_XTAL(bXtal_Sel);
	}
	else
	{
		if ((regx & 0x0080)!=0)						//bXtal_Sel=1
			KT_WirelessMicTx_Set_XTAL(bXtal_Sel);
	}
	return(1);
}

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_AUXCH_SendBurstData											 	 	
//功能描述：辅助信道突发模式发送数据														 	
//函数说明：																 	
//调用函数：KT_Bus_Write();												 	
//全局变量：无																 	
//输    入：iBurstData;														 	
//返    回：无																 	
//设 计 者：KANG Hekai				时间：2012-08-01									 	
//修 改 者：						时间：									 	
//版    本：V2.3																 	
//------------------------------------------------------------------------------------
#ifdef KT0606M
void KT_WirelessMicTx_AUXCH_SendBurstData(uint iBurstData)
{
	KT_Bus_Write(0x02, iBurstData);
}
#endif

//------------------------------------------------------------------------------------
//函 数 名：KT_WirelessMicTx_AUXCH_SendBurstData											 	 	
//功能描述：辅助信道突发模式发送数据														 	
//函数说明：																 	
//调用函数：KT_WirelessMicTx_MuteSel();KT_WirelessMicTx_Pilot();KT_WirelessMicTx_PAGain();
//			KT_WirelessMicTx_PASW();KT_Bus_Read();KT_Bus_Write();Delay_ms();												 	
//全局变量：无																 	
//输    入：无;														 	
//返    回：无																 	
//设 计 者：KANG Hekai				时间：2012-08-20									 	
//修 改 者：						时间：									 	
//版    本：V2.4																 	
//------------------------------------------------------------------------------------
void KT_WirelessMicTx_PowerDownProcedure(void)
{
	uint regx;

	KT_WirelessMicTx_MuteSel(AUDIO_MUTE);
//	Delay_ms(50);
	KT_WirelessMicTx_Pilot(PILOT_DISABLE);
//	Delay_ms(500);

	KT_WirelessMicTx_PAGain(0);
	KT_WirelessMicTx_PASW(PA_OFF);
	regx = KT_Bus_Read(0x08);
	KT_Bus_Write(0x08, regx + (1<<9));						
//	Delay_ms(5);

	regx = KT_Bus_Read(0x03);
	KT_Bus_Write(0x03, regx | (STANDBY << 15));      					//Write Standby bit to 1
//	Delay_ms(20);
}