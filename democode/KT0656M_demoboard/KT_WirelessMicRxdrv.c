///****************************************************************************
//  File Name: KT_WirelessMicRxdrv.c
//  Function:  KT Wireless Mic Receiver Products Driver For Customer
///****************************************************************************
//        Revision History
//  Version Date        Description
//  V0.1    2016-07-13  For KT0656M
//  V0.2    2017-02-10  规范化整理，修改了KT_WirelessMicRx_Init函数里面
//                      auto_mute control中移位的错误
//  V0.3    2017-02-17  添加KT_WirelessMicRx_CheckAUXCH(辅助信道状态变化监测程序)函数
//  V0.4    2017-03-27  在tune函数里面增加了一些操作解决温漂的问题
//  V1.3    2017-04-01  有天线分集的时候，显示现在是接收的主还是从，
//						根据当前是主路或者从路然后读取主或从的RSSI及SNR,pilot及BPSK
//						根据最新的命名规则进行了版本号修改
//  V1.4    2017-04-01  初始化中增加了修改0x1f和0x0d寄存器的值
//  V1.5    2017-05-24  初始化函数中，把comp_tc由原来的1改成了3，
//						把vtr_momitor_en配置为1使能，把ref_vtr_vth_sel配置为1
//  V1.6    2017-06-05  增加了KT_WirelessMicRx_Patch函数，用来修复一些bug
//						修改了一些宏定义的名称，增加了跟KT0616M一样的选择晶振程序
//  V1.7    2017-06-28  增加了AUTOMUTE_SNR_LOWTH和AUTOMUTE_SNR_HIGHTH宏定义
//						初始化里面把PLL_UNLOCK_EN=1注释掉，加了会有概率引起死机现象
//						在切换晶振的过程中，把ifadc也rst了
//						增加了测量电池电压函数KT_WirelessMicRx_BatteryMeter_Read
//  V1.8    2017-08-25  修改了AUTOMUTE_SNR_LOWTH和AUTOMUTE_SNR_HIGHTH的值
//						从原来的0x58和0x60改成了0x78和0x80，增加了搜台功能的宏定义
//						echo delay最多配置为23(197ms)
///****************************************************************************

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include  "I2C.h"
#include "KT_WirelessMicRxdrv.h"
#include <math.h>
#include "main.h"

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

extern UINT8 chipSel;
extern UINT8 Flag_PKGSYNC[chipNumb]; //包同步状态标识

UINT8 MorSSelect=1;//有天线分集时 1:主 0:从

//-----------------------------------------------------------------------------
//函 数 名：KT_Bus_Write
//功能描述：总线写程序                                                                
//函数说明：                                                                        
//全局变量：无                                                                        
//输    入：UINT16 Register_Address, UINT8 Byte_Data, UINT8 chipNum
//返    回：无                                                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_Bus_Write(UINT16 Register_Address, UINT8 Byte_Data,UINT8 chipNum)
{
    switch(chipNum)
    {
#ifdef DIVERSITY
    #ifdef TWOCHANNEL
        case 0: I2C_Byte_Write(KTWirelessMicRxw_addressAM,Register_Address,Byte_Data);break;
        case 1: I2C_Byte_Write(KTWirelessMicRxw_addressAS,Register_Address,Byte_Data);break;
        case 2: I2C_Byte_Write(KTWirelessMicRxw_addressBM,Register_Address,Byte_Data);break;
        case 3: I2C_Byte_Write(KTWirelessMicRxw_addressBS,Register_Address,Byte_Data);break;
    #else
        case 0: I2C_Byte_Write(KTWirelessMicRxw_addressAM,Register_Address,Byte_Data);break;
        case 1: I2C_Byte_Write(KTWirelessMicRxw_addressAS,Register_Address,Byte_Data);break;
       #endif
#else
    #ifdef TWOCHANNEL
        case 0: I2C_Byte_Write(KTWirelessMicRxw_addressAM,Register_Address,Byte_Data);break;
        case 1: I2C_Byte_Write(KTWirelessMicRxw_addressBM,Register_Address,Byte_Data);break;
       #else
        case 0: I2C_Byte_Write(KTWirelessMicRxw_addressAM,Register_Address,Byte_Data);break;
       #endif
#endif
    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_Bus_Read
//功能描述：总线读程序
//函数说明：
//全局变量：无
//输    入：UINT16 Register_Address, UINT8 chipNum                                                
//返    回：I2C_Word_Read(KTWirelessMicRxr_address, Register_Address)                
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT8 KT_Bus_Read(UINT16 Register_Address,UINT8 chipNum)
{
    switch(chipNum)
    {
#ifdef DIVERSITY
    #ifdef TWOCHANNEL
        case 0: return( I2C_Byte_Read(KTWirelessMicRxr_addressAM, Register_Address) );break;
        case 1: return( I2C_Byte_Read(KTWirelessMicRxr_addressAS, Register_Address) );break;
        case 2: return( I2C_Byte_Read(KTWirelessMicRxr_addressBM, Register_Address) );break;
        case 3: return( I2C_Byte_Read(KTWirelessMicRxr_addressBS, Register_Address) );break;
    #else
        case 0: return( I2C_Byte_Read(KTWirelessMicRxr_addressAM, Register_Address) );break;
        case 1: return( I2C_Byte_Read(KTWirelessMicRxr_addressAS, Register_Address) );break;
       #endif
#else
    #ifdef TWOCHANNEL
        case 0: return( I2C_Byte_Read(KTWirelessMicRxr_addressAM, Register_Address) );break;
        case 1: return( I2C_Byte_Read(KTWirelessMicRxr_addressBM, Register_Address) );break;
       #else
        case 0: return( I2C_Byte_Read(KTWirelessMicRxr_addressAM, Register_Address) );break;
       #endif
#endif
		default:while(1);
    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_PreInit                                                 
//功能描述：芯片工作初始化程序                                                        
//函数说明：判断芯片是否正常上电，I2C总线是否读写正常                            
//全局变量：INIT_FAIL_TH                                                            
//输    入：无                                                                        
//返    回：正确：1                 错误：0    
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
BOOL KT_WirelessMicRx_PreInit(void)              
{
    UINT8 regx;
    UINT8 i;

    for (i = 0; i < INIT_FAIL_TH; i++)
    {
        regx = KT_Bus_Read(0x0192,chipSel);        //Read Manufactory ID
        if (regx == 0x4B)
        {
            #ifdef KT0656M
			regx=KT_Bus_Read(0x002d,chipSel); //dcdc
		    KT_Bus_Write(0x002d,(regx & ~0x04)|(DCDC_EN<<2),chipSel);
			#endif
			KT_WirelessMicRx_Volume(0);
            return(1);
        }
    }
    return(0);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Init                                                     
//功能描述：芯片初始化程序                                                            
//函数说明：                                                                        
//全局变量：无
//输    入：无                                                                        
//返    回：正确：1                 错误：0    
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：WSL                     时间：2017-02-10                                 
//版    本：V0.1    For KT0656M
//版    本：V0.2    修改auto_mute control中移位的错误
//版    本：V1.4    初始化中增加了修改0x1f和0x0d寄存器的值
//版    本：V1.5    初始化函数中，把comp_tc由原来的1改成了3，把vtr_momitor_en配置为1使能，把ref_vtr_vth_sel配置为1
//-----------------------------------------------------------------------------
BOOL KT_WirelessMicRx_Init(void)
{
    UINT8 regx;

    while(!(KT_Bus_Read(0x007F,chipSel) & 0x01)); //power_on finish
    
//    regx=KT_Bus_Read(0x007F,chipSel);
//    KT_Bus_Write(0x007F,(regx & 0xfe),chipSel); // power_on finish写0,在while（1）中读到power_on finish=1时，说明芯片重启了

	regx=KT_Bus_Read(0x010f,chipSel);//FIRMWARE_VERSION
	if(regx!=0x10)
		return 0;

	regx = KT_Bus_Read(0x0108,chipSel);          
    KT_Bus_Write(0x0108,(regx&0x1f)|(XTAL_SEL1<<7)|(XTAL_SEL2<<6)|(HLSI_INV<<5),chipSel); //hlsi

	regx = KT_Bus_Read(0x001f,chipSel); 
	KT_Bus_Write(0x001f,(regx&0x1f)|0x40,chipSel); //0.4V

	KT_Bus_Write(0x000d,0xcf,chipSel); //6mA

	//可通过0x16<5:0>的变化体现，注意:由于由软件完成该过程，改变此值后要重新tune台
	KT_Bus_Write(0x0109,0x10,chipSel); //pll noise增加3db

//	regx = KT_Bus_Read(0x0224,chipSel);
//	KT_Bus_Write(0x0224,(regx&0x9f)|0x20,chipSel); //audio_gain=1 增加3.5db

#ifdef AUTOMUTE_EN

    KT_Bus_Write(0x0082,0x89,chipSel); //fast_rssi_mute_th=0x89
	//auto_mute control
    regx=KT_Bus_Read(0x0081,chipSel);
    KT_Bus_Write(0x0081,(regx & 0x8f) | (FAST_RSSI_MUTE_EN<<6) |
                 (FAST_RSSI_PD_MUTE_EN<<5) | (SUPER_RSSI_MUTE_EN<<4), chipSel); 

    KT_Bus_Write(0x0079,AUTOMUTE_SNR_LOWTH,chipSel); 
    KT_Bus_Write(0x007a,AUTOMUTE_SNR_HIGHTH,chipSel); 

	regx=KT_Bus_Read(0x0201,chipSel);
    KT_Bus_Write(0x0201,regx|0x80,chipSel); //NBW=1
//  KT_Bus_Write(0x0201,regx&~0x80,chipSel); //NBW=0

    regx=KT_Bus_Read(0x0100,chipSel);
//  KT_Bus_Write(0x0100,(regx & 0xDF) | (0<<5),chipSel); //automute_snr_en=0
    KT_Bus_Write(0x0100,(regx & 0xDF) | (1<<5),chipSel); //automute_snr_en=1

    
    regx=KT_Bus_Read(0x0100,chipSel);
#ifdef TWOCHANNEL
    if(chipSel<chipBM)	//A 通道的主或者从
#endif
    {
        if((soundA.pilotFlag&0x01)==1)
        {
            KT_Bus_Write(0x0100,regx|0x80,chipSel);    //开导频检测          
        }
        else
        {
            KT_Bus_Write(0x0100,regx&0x7f,chipSel);   //关导频检测           
        }
    }
#ifdef TWOCHANNEL
    else
    {
        if((soundB.pilotFlag&0x01)==1)
        {
            KT_Bus_Write(0x0100,regx|0x80,chipSel);              
        }
        else
        {
            KT_Bus_Write(0x0100,regx&0x7f,chipSel);              
        }
    }
#endif

//    regx=KT_Bus_Read(0x0053,chipSel);
//    KT_Bus_Write(0x0053,regx|0x10,chipSel); //BPSK_PKG_SYN_INT_EN=1
#endif

#ifdef SQUEAL_EN
    regx=KT_Bus_Read(0x026f,chipSel);
    KT_Bus_Write( 0x026f, (regx & 0xC0) | (SQUEAL_ELIM_EN << 5) |(SQEUAL_DET_EN << 3) | (FNOTCH_NOTRESP_TH << 2) | (N_OCTAVE << 0),chipSel);
    regx=KT_Bus_Read(0x0270,chipSel);
    KT_Bus_Write( 0x0270, (regx & 0x00) | (FFT_R_TH << 4) | (FRAME_NUM_TH << 0),chipSel);
    regx=KT_Bus_Read(0x0271,chipSel);
    KT_Bus_Write( 0x0271, (regx & 0x00) | (PMAX_HITH << 4) | (PMAX_LOWTH << 0),chipSel);
    regx=KT_Bus_Read(0x0272,chipSel);
    KT_Bus_Write( 0x0272, (regx & 0x00) | (FDIFF_HITH << 4) | (FDIFF_LOWTH << 0),chipSel);
#endif

    //config for bpsk
    regx = KT_Bus_Read(0x241,chipSel);          
    KT_Bus_Write(0x241,(regx & 0x3f) | (AUXDATA_EN<<7)|(BPSK_NEW_MODE<<6),chipSel); //bpsk enable bpsk new mode
    regx = KT_Bus_Read(0x243,chipSel);          
    KT_Bus_Write(0x243,(regx & 0x88) | (AUX_CARRY_NUM<<4)| CARRY_NUM_TH,chipSel); //载波个数为22 载波判决门限18 
    regx = KT_Bus_Read(0x245,chipSel);          
    KT_Bus_Write(0x245,0x22,chipSel); //解析过程中连续正确和错误（可以不连续）的数据包数都为2

    regx = KT_Bus_Read(0x0200,chipSel);          
    KT_Bus_Write(0x0200,(regx&0x8f)|(ADJUST_GAIN<<4),chipSel); //adjust gain =50kHz

    regx = KT_Bus_Read(0x0225,chipSel); //DC_NOTCH_MUTE_EN=1
    KT_Bus_Write(0x0225,regx&0xf1,chipSel);
//	KT_Bus_Write(0x0225,regx&0xf0,chipSel);
                                                
	regx = KT_Bus_Read(0x010e,chipSel);		  	 //AFC CTRL FSM使能控制位disable
	KT_Bus_Write(0x010e,regx&0xfe,chipSel);

	regx=KT_Bus_Read(0x0087,chipSel);			//SOFT_AFC_MUTE写为0
	KT_Bus_Write(0x0087,regx&~0x08,chipSel);

    regx = KT_Bus_Read(0x0217,chipSel); //
    KT_Bus_Write(0x0217,(regx&0x3f)|(AFC_RNG<<6)|0x01,chipSel);	// +/-60kHz;

    KT_Bus_Write(0x0218,0x02,chipSel); //afc_en=1  AFC_FROZEN=0
//	KT_Bus_Write(0x0218,0x00,chipSel); //afc_en=0   

    regx = KT_Bus_Read(0x0256,chipSel); //comp_tc=1
    KT_Bus_Write(0x0256,(regx&0x8f)|0x10,chipSel); 

	regx = KT_Bus_Read(0x002d,chipSel); //vtr_momitor_en=1
    KT_Bus_Write(0x002d,regx|0x08,chipSel); 

	regx = KT_Bus_Read(0x0010,chipSel); //ref_vtr_vth_sel=1
    KT_Bus_Write(0x0010,regx|0x80,chipSel);


	#ifdef DIVERSITY
		#ifdef TWOCHANNEL
		if((chipSel==chipAS)||(chipSel==chipBS))
		#else
		if(chipSel==chipAS)
		#endif
		{
			regx=KT_Bus_Read(0x0202,chipSel);
			KT_Bus_Write(0x0202,regx|0x04,chipSel);	  //SIGINV_  HLSI=1
			regx=KT_Bus_Read(0x0255,chipSel);
			KT_Bus_Write(0x0255,regx|0x10,chipSel);	  //AUDIO_INV=1
		}
	#endif

	regx=KT_Bus_Read(0x0030,chipSel);
	KT_Bus_Write(0x0030,regx & ~BIT7,chipSel);//LO_LOCK_DET_PD=0

//	regx = KT_Bus_Read(0x0133,chipSel);          
//    KT_Bus_Write(0x0133,(regx|0x20),chipSel); //PLL_UNLOCK_EN=1

    regx = KT_Bus_Read(0x0133,chipSel);          
    KT_Bus_Write(0x0133,(regx|0x40),chipSel); //dll_rst_en

//	regx = KT_Bus_Read(0x010e,chipSel);          
//    KT_Bus_Write(0x010e,(regx|0x06),chipSel); //打开watch dog

	regx = KT_Bus_Read(0x010e,chipSel); //天线分集软件状态机需要关闭（用硬件的）
    KT_Bus_Write(0x010e,regx&0xf7,chipSel);
	
	//AUDIO_SEL 输出合并信号，需要在天线分集软件状态机需要关闭后在写，因为软件开时会自动改写
	regx = KT_Bus_Read(0x021c,chipSel); 
    KT_Bus_Write(0x021c,regx&0xcf,chipSel);

	regx = KT_Bus_Read(0x0108,chipSel);          
    KT_Bus_Write(0x0108,(regx|0x10),chipSel); //LOBAND_CALI_SCAN_EN=1 for fasttune
	while(1)
	{
		Delay_ms(10);
		if(0==(0x10&KT_Bus_Read(0x0108,chipSel)))
		break;
	}
	#ifdef KT0655M
	regx=KT_Bus_Read(0x0331,chipSel);
	KT_Bus_Write(0x0331,(regx & 0xfe)|LINEIN_AGC_DIS,chipSel);//LINEIN_AGC_DIS

	regx=KT_Bus_Read(0x0333,chipSel);
	KT_Bus_Write(0x0333,(regx & ~0x30)|(COMPEN_GAIN<<4),chipSel);//COMPEN_GAIN

	regx=KT_Bus_Read(0x0334,chipSel);
	KT_Bus_Write(0x0334,(regx & ~0x18)|(PGA_GAIN_SEL<<3),chipSel);//PGA_GAIN_SEL

	regx=KT_Bus_Read(0x0336,chipSel);
	KT_Bus_Write(0x0336,(regx & 0xc0)|(SLNC_MUTE_TIME<<1)|SLNC_MUTE_DIS,chipSel);//SLNC_MUTE_DIS  SLNC_MUTE_TIME

	KT_Bus_Write(0x0337,(SLNC_MUTE_LOW_LEVEL<<4)|SLNC_MUTE_HIGH_LEVEL,chipSel);//SLNC_MUTE_LEVEL

	KT_Bus_Write(0x0339,(ALC_DIS<<7)|ALC_VMAX,chipSel);	 //	ALC_DIS ALC_VMAX

	regx=KT_Bus_Read(0x033a,chipSel);
	KT_Bus_Write(0x033a,(regx & ~0x08)|(ALC_SOFTKNEE<<3),chipSel);//ALC_SOFTKNEE
	#endif
    
    return(1);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Volume                                                     
//功能描述：输出音量调节
//函数说明：cVolume围为0-31，共32档, 其中0为静音                                    
//全局变量：                                                                        
//输    入：cVolume                                                                
//返    回：正确：1                 错误：0                                            
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
BOOL KT_WirelessMicRx_Volume(UINT8 cVolume)
{
    UINT8 regx;

    regx = KT_Bus_Read(0x0201,chipSel);
    KT_Bus_Write(0x0201, (regx & 0xE0) | (cVolume),chipSel);

    return(1);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Tune                                                     
//功能描述：接收频率设置函数                                                        
//函数说明：输入以KHz为单位的接收频率，                                                
//全局变量：无                                                            
//输    入：Freq （输入以KHz为单位的接收频率）                                        
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M 
//版    本：V1.6    增加了根据KT0616M选晶体的程序 
//版    本：V1.7    在切换晶振之前，先把PLL失锁中断关闭，tune台后再打开                                                                      
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_Tune(long Freq)
{
    UINT8 Freq_H,Freq_M,Freq_L,regx;
	UINT8 LO_LOCK_DET_PD_SAVE;
	UINT8 state;
    
//	regx = KT_Bus_Read(0x0133,chipSel);
//	KT_Bus_Write(0x0133,(regx&~0x60),chipSel); //dll_rst_en=0  I_PLL_UNLOCK_EN=0  
//	regx=KT_Bus_Read(0x0054,chipSel);
//	KT_Bus_Write(0x0054,regx&~0x18,chipSel);//PLL失锁中断 dis

#ifdef XTAL_DUAL
	#ifdef NEW_SEL_XTAL_MODE
    	caclXtal(Freq);//select xtal
	#else
		oldCaclXtal(Freq);
	#endif
#endif
    
	regx=KT_Bus_Read(0x0108,chipSel);
    KT_Bus_Write(0x0108,regx & ~BIT3,chipSel); //SCAN_MODE=0;

	Freq = Freq & 0x000FFFFF;

    Freq_H = ( Freq >> 12 );
    Freq_M = ( (Freq & 0x00000FFF) >> 4 );
    Freq_L = ( (Freq & 0x0000000F) << 4 );

    KT_Bus_Write(0x0045,Freq_H,chipSel);                
    KT_Bus_Write(0x0046,Freq_M,chipSel);

    regx=KT_Bus_Read(0x0047,chipSel);
    KT_Bus_Write(0x0047,(regx & 0x0F) | Freq_L ,chipSel);

	regx=KT_Bus_Read(0x0053,chipSel);
	KT_Bus_Write(0x0053,regx&~0x40,chipSel);  //rfamp_int_en=0

    regx=KT_Bus_Read(0x0047,chipSel);
    KT_Bus_Write(0x0047,regx | BIT0,chipSel); //chan_valid=1;

	//因为chan_valid=1以后，触发tune中断PLL done先写为0，完成后写1，如果不delay 1ms,可能PLL done读出为1的时候
	//内部cpu程序还没有把PLL done写成0，所以外部mcu和内部cpu可能会同时操作一些内部DSP等，导致死机，所以需要delay
	Delay_ms(1); 
    while (!(KT_Bus_Read(0x0061,chipSel) & 0x01)); //PLL done

	regx=KT_Bus_Read(0x0042,chipSel);
	KT_Bus_Write(0x0042,regx | BIT2,chipSel);//S_DSP_RST

	regx=KT_Bus_Read(0x0042,chipSel);
	KT_Bus_Write(0x0042,regx | BIT4,chipSel);//S_PLL_SDM_RST			

	regx=KT_Bus_Read(0x0030,chipSel);
	LO_LOCK_DET_PD_SAVE = regx&0x80;
	KT_Bus_Write(0x0030,regx | BIT7,chipSel);//LO_LOCK_DET_PD=1
   
	regx=KT_Bus_Read(0x0017,chipSel); //double+16MHz/V locoarse_var_sel
    state=regx&0x07;
    if(state >= 3)
    {
        state = 7;                                
    }
    else
    {
        state = (state<<1) + 3;
    }
    regx=(regx&0xf8)|state;                                          
    KT_Bus_Write(0x0017, regx,chipSel); //write locoarse/lofine_var_sel

	Pll_Band_Cali(0, 255);

	PLL_Reset();

	regx=KT_Bus_Read(0x0042,chipSel);
	KT_Bus_Write(0x0042,regx & ~BIT4,chipSel);//C_PLL_SDM_RST

	regx=KT_Bus_Read(0x0042,chipSel);
	KT_Bus_Write(0x0042,regx & ~BIT2,chipSel);//C_DSP_RST

	Delay_ms(10);

	regx=KT_Bus_Read(0x0030,chipSel);
	KT_Bus_Write(0x0030,(regx & ~BIT7)|LO_LOCK_DET_PD_SAVE,chipSel);//LO_LOCK_DET_PD recovery

//	regx = KT_Bus_Read(0x0133,chipSel);          
//    KT_Bus_Write(0x0133,(regx|0x60),chipSel); // dll_rst_en=1 I_PLL_UNLOCK_EN=1  

    KT_WirelessMicRx_SAIInit();

	if(KT_WirelessMicRx_GetSNR()>AUTOMUTE_SNR_LOWTH)
	{
		regx=KT_Bus_Read(0x0087,chipSel);			//SOFT_SNR_MUTE写为0
		KT_Bus_Write(0x0087,regx&~0x02,chipSel);
	}
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_FastTune                                                     
//功能描述：接收频率设置函数                                                        
//函数说明：输入以KHz为单位的接收频率，                                                
//全局变量：无                                                            
//输    入：Freq （输入以KHz为单位的接收频率）                                        
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_FastTune(long Freq)
{
    UINT8 Freq_H,Freq_M,Freq_L,regx;
    
#ifdef XTAL_DUAL
	#ifdef NEW_SEL_XTAL_MODE
    	caclXtal(Freq);//select xtal
	#else
		oldCaclXtal(Freq);
	#endif
#endif
    
	regx=KT_Bus_Read(0x0108,chipSel);
    KT_Bus_Write(0x0108,regx | BIT3,chipSel); //SCAN_MODE=1;

	Freq = Freq & 0x000FFFFF;

    Freq_H = ( Freq >> 12 );
    Freq_M = ( (Freq & 0x00000FFF) >> 4 );
    Freq_L = ( (Freq & 0x0000000F) << 4 );

    KT_Bus_Write(0x0045,Freq_H,chipSel);                
    KT_Bus_Write(0x0046,Freq_M,chipSel);

    regx=KT_Bus_Read(0x0047,chipSel);
    KT_Bus_Write(0x0047,(regx & 0x0F) | Freq_L ,chipSel);

	regx=KT_Bus_Read(0x0053,chipSel);
	KT_Bus_Write(0x0053,regx&~0x40,chipSel);  //rfamp_int_en=0

    regx=KT_Bus_Read(0x0047,chipSel);
    KT_Bus_Write(0x0047,regx | BIT0,chipSel); //chan_valid=1;

	Delay_ms(1); 
    while (!(KT_Bus_Read(0x0061,chipSel) & 0x01)); //PLL done
 
    KT_WirelessMicRx_SAIInit();

	if(KT_WirelessMicRx_GetSNR()>AUTOMUTE_SNR_LOWTH)
	{
		regx=KT_Bus_Read(0x0087,chipSel);			//SOFT_SNR_MUTE写为0
		KT_Bus_Write(0x0087,regx&~0x02,chipSel);
	}	
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SW_Echo
//功能描述：音频混响开关程序
//函数说明：
//全局变量：
//输    入：UINT8 cEcho_En
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SW_Echo(UINT8 cEcho_En)
{
    UINT8 regx;

    regx=KT_Bus_Read(0x0266,chipSel);         
    KT_Bus_Write(0x0266,(regx & 0x08) | (cEcho_En<<7) | (ECHO_STRU<<6) | 
                 (ECHO_GAIN_DOWN<<4) | (ECHO_GAIN_UP<<0),chipSel);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SetEcho
//功能描述：音频混响设置程序
//函数说明：
//全局变量：
//输    入：UINT8 cEcho_Ratio, UINT8 cEcho_Delay
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SetEcho(UINT8 cEcho_Ratio, UINT8 cEcho_Delay)
{
    UINT8 regx;

    regx=KT_Bus_Read(0x0267,chipSel);         
    KT_Bus_Write(0x0267,(regx & 0xE0) | (cEcho_Ratio<<0),chipSel);

    regx=KT_Bus_Read(0x0268,chipSel);         
    KT_Bus_Write(0x0268,(regx & 0xE0) | (cEcho_Delay<<0),chipSel);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SW_Exciter
//功能描述：音频激励开关程序
//函数说明：
//全局变量：
//输    入：UINT8 cExciter_En
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SW_Exciter(UINT8 cExciter_En)
{
    UINT8 regx;

    regx=KT_Bus_Read(0x0269,chipSel);         
    KT_Bus_Write(0x0269,(regx & 0x18) | (cExciter_En<<7) | (EXCITER_TUNE<<5) | 
                 (EXCITER_DRIVE<<0),chipSel);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SetExciter
//功能描述：音频激励设置程序
//函数说明：
//全局变量：
//输    入：UINT8 cExciter_Odd, UINT8 cExciter_Even
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SetExciter(UINT8 cExciter_Odd, UINT8 cExciter_Even)
{
    UINT8 regx;

    regx=KT_Bus_Read(0x026A,chipSel);         
    KT_Bus_Write(0x026A,(regx & 0x88) | (cExciter_Odd<<4) | (cExciter_Even<<0),chipSel);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SW_Equalizer
//功能描述：音频均衡开关程序
//函数说明：
//全局变量：
//输    入：UINT8 cEqualizer_En
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SW_Equalizer(UINT8 cEqualizer_En)
{
    UINT8 regx;

    regx=KT_Bus_Read(0x0257,chipSel);         
    KT_Bus_Write(0x0257,(regx & 0xDF) | (cEqualizer_En<<5),chipSel);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SW_Diversity
//功能描述：天线分集开关程序
//函数说明：
//全局变量：
//输    入：UINT8 cEqualizer_En
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SW_Diversity(UINT8 diversity_En)
{
    UINT8 regx;
    
    regx = KT_Bus_Read(0x021c,chipSel); //天线分集
    if(diversity_En)
    {        
        KT_Bus_Write(0x021c,regx|0x40,chipSel);
    }
    else
    {
        KT_Bus_Write(0x021c,regx&~0x40,chipSel);
    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SetEqualizer
//功能描述：音频均衡设置程序
//函数说明：
//全局变量：
//输    入：UINT8 cEq_Frq, UINT8 cEq_Gain
//返    回：无                                        
//设 计 者：Zhou Dongfeng           时间：2016-07-12                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SetEqualizer(UINT8 cEqualizer_Frq, UINT8 cEqualizer_Gain)
{ 
    UINT8 regx_1 = 0;
    
    if(0 == cEqualizer_Frq)
    {
        regx_1 = KT_Bus_Read(0x0257 + cEqualizer_Frq,chipSel);
        regx_1 &= 0x20; 
        KT_Bus_Write(0x0257 + cEqualizer_Frq, cEqualizer_Gain | regx_1,chipSel);
    }
    else
    {
        KT_Bus_Write(0x0257 + cEqualizer_Frq, cEqualizer_Gain,chipSel);
    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetAF
//功能描述：获取音频幅度
//函数说明：
//全局变量：
//输    入：无
//返    回：音频信号强度
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT8 KT_WirelessMicRx_GetAF(void)
{
    UINT16 regx;
    regx = KT_Bus_Read(0x0209,chipSel);
    return( regx & 0x0F );
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetRSSI
//功能描述：获取射频信号强度
//函数说明：
//全局变量：
//输    入：无
//返    回：射频信号强度
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT8 KT_WirelessMicRx_GetRSSI(void)
{
    UINT8 cRssi;
    if(MorSSelect)
	{
    	cRssi = KT_Bus_Read(0x020C,chipSel);
	}
	else
	{
		cRssi = KT_Bus_Read(0x0221,chipSel);
	}
    return( cRssi );
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetFastRSSI
//功能描述：获取射频信号强度
//函数说明：
//全局变量：
//输    入：无
//返    回：射频信号强度
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT8 KT_WirelessMicRx_GetFastRSSI(void)
{
    UINT8 cRssi;
    if(MorSSelect)
	{
    	cRssi = KT_Bus_Read(0x020A,chipSel);
	}
	else
	{
		cRssi = KT_Bus_Read(0x0221,chipSel);
	}
    return( cRssi );
}
//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_GetSNR
//功能描述：获取SNR
//函数说明：
//全局变量：
//输    入：无
//返    回：SNR值
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT8 KT_WirelessMicRx_GetSNR(void)
{
    UINT8 snr;
    if(MorSSelect)
	{
    	snr = KT_Bus_Read(0x020D,chipSel);
	}
	else
	{
    	snr = KT_Bus_Read(0x0223 ,chipSel);
	}
    return( snr );
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Automute
//功能描述：获取Automute状态
//函数说明：
//全局变量：
//输    入：无
//返    回：Automute状态值
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT8 KT_WirelessMicRx_Automute(void)
{
    UINT8 muteFlag;
    
    muteFlag = KT_Bus_Read(0x0088,chipSel);
    return( muteFlag&0x01);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_BatteryMeter_Read
//功能描述：获取电池电压值
//函数说明：1.2V对应最大值0x07ff
//全局变量：
//输    入：无
//返    回：电池电压code值 0v对应0 1.2V对应2047
//设 计 者：wu jinfeng              时间：2017-06-23                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT16 KT_WirelessMicRx_BatteryMeter_Read(void)
{
	INT16 batNumber;
	batNumber=(KT_Bus_Read(0x00c0,chipSel));
	batNumber<<=8;
	batNumber|=KT_Bus_Read(0x00c1,chipSel);
	return (abs(batNumber));
}
//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_CheckAUXCH
//功能描述：辅助信道状态变化监测程序
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_CheckAUXCH(void)
{
    UINT8 regx;
    if(MorSSelect)
	{
    	regx = KT_Bus_Read(0x0057,chipSel);
	    if(regx&0x10) //pkg_sync int
	    {
	        KT_Bus_Write( 0x0057, regx |0x10 ,chipSel);
	        regx = KT_Bus_Read(0x0209,chipSel);
	        if(regx&0x40)
	            Flag_PKGSYNC[chipSel] = PKGSYNC;
	        else
	            Flag_PKGSYNC[chipSel] = NON_PKGSYNC;
	    }
	}
	#ifdef DIVERSITY
	else
	{
    	regx = KT_Bus_Read(0x0057,chipSel+1);
	    if(regx&0x10) //pkg_sync int
	    {
	        KT_Bus_Write( 0x0057, regx |0x10 ,chipSel+1);
	        regx = KT_Bus_Read(0x0209,chipSel+1);
	        if(regx&0x40)
	            Flag_PKGSYNC[chipSel+1] = PKGSYNC;
	        else
	            Flag_PKGSYNC[chipSel+1] = NON_PKGSYNC;
	    }
	}
	#endif
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_CheckPilot
//功能描述：获取导频的状态
//函数说明：
//全局变量：
//输    入：无
//返    回：无导频: 0               有导频: 1
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
UINT8 KT_WirelessMicRx_CheckPilot(void)
{
    UINT8 pilotFlag;
    if(MorSSelect)
	{
    	pilotFlag = KT_Bus_Read(0x0209,chipSel);
	}
	#ifdef DIVERSITY
	else
	{
    	pilotFlag = KT_Bus_Read(0x0209,chipSel+1);
	}
	#endif
    return((pilotFlag>>7)&0x01);
}

//-----------------------------------------------------------------------------
//函 数 名：Pll_Band_Cali
//功能描述：为了解决温漂问题新加的函数，在tune过程中调用
//函数说明：
//全局变量：
//输    入：LO_VCO_BAND_SEL 起始值和最大值
//返    回：
//设 计 者：wu jinfeng              时间：2017-04-01                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
static void Pll_Band_Cali(UINT8 CLl, UINT8 CLh)
{
    INT32 data Tmp[2],I_VCOCNT_RES_2,I_VCOFREQ_REF;
	UINT8 regx,loVcoBandSelRead;

	regx=KT_Bus_Read(0x005c,chipSel);
    if(!(regx&BIT6))
    {
        return;
    }
    regx=KT_Bus_Read(0x0061,chipSel);
	regx&=0xc7;
	regx|=0x28;					//O_VCOCNT_WIN(5);
    KT_Bus_Write(0x0061, regx ,chipSel);

	regx=KT_Bus_Read(0x001c,chipSel); //S_LO_VCO_BAND_CALI_EN;
    KT_Bus_Write(0x001c, regx|BIT6 ,chipSel);

	regx=KT_Bus_Read(0x001a,chipSel);
	regx&=0xc0;
	regx|=0x20;					//O_LO_MMD_MC(32);
    KT_Bus_Write(0x001a, regx ,chipSel);

	regx=KT_Bus_Read(0x0061,chipSel); //S_LOMC_SEL;
    KT_Bus_Write(0x0061, regx|BIT1 ,chipSel);

    regx=KT_Bus_Read(0x0016,chipSel); //S_LO_DIV128_EN;
    KT_Bus_Write(0x0016, regx|BIT7 ,chipSel);
    
    
//    Delay_ms(1);
	loVcoBandSelRead = KT_Bus_Read(0x001e,chipSel);
	if(loVcoBandSelRead<5)
	{
		KT_Bus_Write(0x001e, CLl ,chipSel);//R_LO_VCO_BAND_SEL = CLl;
	}
	else
	{
		KT_Bus_Write(0x001e, loVcoBandSelRead-5 ,chipSel);//R_LO_VCO_BAND_SEL = CLl;
	}    

	regx=KT_Bus_Read(0x0042,chipSel); //C_VCOCNT_RST;
    KT_Bus_Write(0x0042, regx&~BIT5 ,chipSel);

	regx=KT_Bus_Read(0x0061,chipSel);
    KT_Bus_Write(0x0061, regx|BIT6 ,chipSel);  //S_VCOCNT_START;
    
    
    while(!(BIT7&KT_Bus_Read(0x0064,chipSel)));//while(!I_VCOCNT_RDY);

	I_VCOCNT_RES_2=(0x0f & KT_Bus_Read(0x0064,chipSel));
	I_VCOCNT_RES_2<<=8;
	I_VCOCNT_RES_2|=KT_Bus_Read(0x0065,chipSel);
	I_VCOCNT_RES_2<<=8;
	I_VCOCNT_RES_2|=KT_Bus_Read(0x0066,chipSel);
	I_VCOCNT_RES_2>>=2;

	I_VCOFREQ_REF=(0x03 & KT_Bus_Read(0x0067,chipSel));
	I_VCOFREQ_REF<<=8;
	I_VCOFREQ_REF|=KT_Bus_Read(0x0068,chipSel);
	I_VCOFREQ_REF<<=8;
	I_VCOFREQ_REF|=KT_Bus_Read(0x0069,chipSel);

    Tmp[1] = I_VCOCNT_RES_2 - I_VCOFREQ_REF;

	regx=KT_Bus_Read(0x0042,chipSel); //S_VCOCNT_RST
    KT_Bus_Write(0x0042, regx|BIT5 ,chipSel);
    
    while(1)
    {      
        regx=KT_Bus_Read(0x001e,chipSel); //R_LO_VCO_BAND_SEL++
    	KT_Bus_Write(0x001e, ++regx ,chipSel);

		regx=KT_Bus_Read(0x0042,chipSel); //C_VCOCNT_RST;
    	KT_Bus_Write(0x0042, regx&~BIT5 ,chipSel);

		regx=KT_Bus_Read(0x0061,chipSel);
    	KT_Bus_Write(0x0061, regx|BIT6 ,chipSel);  //S_VCOCNT_START;
        
        while(!(BIT7&KT_Bus_Read(0x0064,chipSel)));//while(!I_VCOCNT_RDY);
        Tmp[0] = labs(Tmp[1]);
		I_VCOCNT_RES_2=(0x0f & KT_Bus_Read(0x0064,chipSel));
		I_VCOCNT_RES_2<<=8;
		I_VCOCNT_RES_2|=KT_Bus_Read(0x0065,chipSel);
		I_VCOCNT_RES_2<<=8;
		I_VCOCNT_RES_2|=KT_Bus_Read(0x0066,chipSel);
		I_VCOCNT_RES_2>>=2;
	
		I_VCOFREQ_REF=(0x03 & KT_Bus_Read(0x0067,chipSel));
		I_VCOFREQ_REF<<=8;
		I_VCOFREQ_REF|=KT_Bus_Read(0x0068,chipSel);
		I_VCOFREQ_REF<<=8;
		I_VCOFREQ_REF|=KT_Bus_Read(0x0069,chipSel);
        Tmp[1] = I_VCOCNT_RES_2 - I_VCOFREQ_REF;
        regx=KT_Bus_Read(0x0042,chipSel); //S_VCOCNT_RST
    	KT_Bus_Write(0x0042, regx|BIT5 ,chipSel);

        if((labs(Tmp[1]) >= Tmp[0]) && (Tmp[1] <= 0))
        {
            regx=KT_Bus_Read(0x001e,chipSel); //R_LO_VCO_BAND_SEL--
    		KT_Bus_Write(0x001e, --regx ,chipSel);
            break;
        }
        else if(CLh != KT_Bus_Read(0x001e,chipSel))
        {
            continue;
        }
        else
        {
            break;
        }
    }
    regx=KT_Bus_Read(0x001c,chipSel); //C_LO_VCO_BAND_CALI_EN
    KT_Bus_Write(0x001c, regx&~BIT6 ,chipSel);

	regx=KT_Bus_Read(0x0016,chipSel); //C_LO_DIV128_EN;
    KT_Bus_Write(0x0016, regx&~BIT7 ,chipSel);

	regx=KT_Bus_Read(0x0061,chipSel); //C_LOMC_SEL;
    KT_Bus_Write(0x0061, regx&~BIT1 ,chipSel);
    
    return;
}

//-----------------------------------------------------------------------------
//函 数 名：PLL_Reset
//功能描述：为了解决温漂问题新加的函数，在tune过程中调用
//函数说明：
//全局变量：
//输    入：无
//返    回：无
//设 计 者：wu jinfeng              时间：2017-04-01                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
static void PLL_Reset(void)
{
    UINT8 Tmp;
    UINT8 VrefSel;
	UINT8 regx,regy;
    
    if(BIT3&KT_Bus_Read(0x00133,chipSel))	  //I_PLL_RESET_EN
    {
        regx=KT_Bus_Read(0x0016,chipSel);  //Tmp = I_LO_KVCO_CALI_EN;
		Tmp = (regx>>6)&BIT0;

        VrefSel = 0x07&KT_Bus_Read(0x0016,chipSel);//I_LO_KVCO_COARSE_VREF_SEL;

		regy=KT_Bus_Read(0x0133,chipSel);
		regy&=0x07;

		regx=KT_Bus_Read(0x0018,chipSel);
		regx&=~0x07;
		regx|=regy;
		KT_Bus_Write(0x0018, regx ,chipSel);  //O_LO_KVCO_COARSE_VREF_SEL(I_LO_KVCO_COARSE_VREF_SEL_RST);
		
		regx=KT_Bus_Read(0x0016,chipSel);
		KT_Bus_Write(0x0016, regx|BIT6 ,chipSel); 	//S_LO_KVCO_CALI_EN;      
        
        Delay_ms(10);

        if(Tmp)
        {
            regx=KT_Bus_Read(0x0016,chipSel);
			KT_Bus_Write(0x0016, regx|BIT6 ,chipSel); 	//S_LO_KVCO_CALI_EN; 
        }
        else
        {
            regx=KT_Bus_Read(0x0016,chipSel);
			KT_Bus_Write(0x0016, regx&~BIT6 ,chipSel); 	//C_LO_KVCO_CALI_EN; 
        }

		regx=KT_Bus_Read(0x0018,chipSel);
		KT_Bus_Write(0x0018, (regx&0xf8)|VrefSel ,chipSel);  //O_LO_KVCO_COARSE_VREF_SEL(VrefSel);        
    }
}

//-----------------------------------------------------------------------------
//函 数 名：selectMS
//功能描述：根据fast rssi的值决定天线分集用的是主路还是从路
//函数说明：
//全局变量：
//输    入：无
//返    回：无
//设 计 者：wu jinfeng              时间：2017-04-01                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                                                                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SelectMS(void)
{
	#ifdef DIVERSITY
	UINT8 fastRssiSlave,fastRssiMaster;
	fastRssiMaster=KT_Bus_Read(0x020a,chipSel);
	fastRssiSlave=KT_Bus_Read(0x0222,chipSel);
	if(fastRssiMaster>=fastRssiSlave)
	{
		MorSSelect=1;
	}
	else
	{
		MorSSelect=0;
	}
	#else
		MorSSelect=1;
	#endif

}

//-----------------------------------------------------------------------------
//函 数 名：rfIntCtl
//功能描述：rfIntCtl
//函数说明：在主循环里面控制RF中断使能开关
//全局变量：无
//输    入：无
//返    回：无   
//设 计 者：wu jinfeng              时间：2017-03-27
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void rfIntCtl(void)
{
	UINT8 regx;
	for(chipSel=0;chipSel<chipNumb;chipSel++)
	{
		if(!(0x7f&KT_Bus_Read(0x005b,chipSel)))
		{
			regx=KT_Bus_Read(0x0053,chipSel);
			KT_Bus_Write(0x0053,regx&~0x40,chipSel);
		}
		else
		{
			regx=KT_Bus_Read(0x0053,chipSel);
			KT_Bus_Write(0x0053,regx|0x40,chipSel);
		}
	}
}

//-----------------------------------------------------------------------------
//函 数 名：pilotMuteRefresh
//功能描述：pilotMuteRefresh
//函数说明：修复导频mute的bug
//全局变量：无
//输    入：无
//返    回：无   
//设 计 者：wu jinfeng              时间：2017-06-02
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
static void  pilotMuteRefresh(void)
{
	UINT8 regx;
	for(chipSel=0;chipSel<chipNumb;chipSel++)
	{
		if(((0x80&KT_Bus_Read(0x0100,chipSel))==0x80)&&((0x80&KT_Bus_Read(0x0209,chipSel))==0x00))
	    {
	        regx=KT_Bus_Read(0x0087,chipSel);
	        KT_Bus_Write(0x0087,(regx|0x04),chipSel);
	    }
		else
	    {
			regx=KT_Bus_Read(0x0087,chipSel);
	        KT_Bus_Write(0x0087,(regx&~0x04),chipSel);
	    }
	}
}

//-----------------------------------------------------------------------------
//函 数 名：snrMuteRefresh
//功能描述：snrMuteRefresh
//函数说明：修复snr mute的bug
//全局变量：无
//输    入：无
//返    回：无   
//设 计 者：wu jinfeng              时间：2017-06-02
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
static void  snrMuteRefresh(void)
{
	UINT8 regx;
	for(chipSel=0;chipSel<chipNumb;chipSel++)
	{
		if(((KT_Bus_Read(0x0100,chipSel)&0x20)==0x20)&&(KT_Bus_Read(0x020D,chipSel)<=AUTOMUTE_SNR_LOWTH))
	    {
	        regx=KT_Bus_Read(0x0087,chipSel);
	        KT_Bus_Write(0x0087,(regx|0x02),chipSel);
	    }
		else if((KT_Bus_Read(0x020D,chipSel)>=AUTOMUTE_SNR_HIGHTH)||((KT_Bus_Read(0x0100,chipSel)&0x20)==0x00))
	    {
			regx=KT_Bus_Read(0x0087,chipSel);
	        KT_Bus_Write(0x0087,(regx&~0x02),chipSel);
	    }
		else
		{
		}
	}
}

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_Patch
//功能描述：KT_WirelessMicRx_Patch
//函数说明：修复一些KT0656M的bug
//全局变量：无
//输    入：无
//返    回：无   
//设 计 者：wu jinfeng              时间：2017-06-02
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_Patch(void)
{
	rfIntCtl();
	pilotMuteRefresh();
	snrMuteRefresh();
}

void Seek_Freq_FastTune(long Freq)
{
    UINT8 Freq_H,Freq_M,Freq_L,regx;
	regx=KT_Bus_Read(0x0053,chipSel);
	KT_Bus_Write(0x0053,regx&~0x40,chipSel);  //rfamp_int_en=0  
#ifdef XTAL_DUAL
	#ifdef NEW_SEL_XTAL_MODE
    	caclXtal(Freq);//select xtal
	#else
		oldCaclXtal(Freq);
	#endif
#endif 
	regx=KT_Bus_Read(0x0108,chipSel);
    KT_Bus_Write(0x0108,regx | BIT3,chipSel); //SCAN_MODE=1;

	Freq = Freq & 0x000FFFFF;

    Freq_H = ( Freq >> 12 );
    Freq_M = ( (Freq & 0x00000FFF) >> 4 );
    Freq_L = ( (Freq & 0x0000000F) << 4 );

    KT_Bus_Write(0x0045,Freq_H,chipSel);                
    KT_Bus_Write(0x0046,Freq_M,chipSel);

    regx=KT_Bus_Read(0x0047,chipSel);
    KT_Bus_Write(0x0047,(regx & 0x0F) | Freq_L ,chipSel);

    regx=KT_Bus_Read(0x0047,chipSel);
    KT_Bus_Write(0x0047,regx | BIT0,chipSel); //chan_valid=1;
	Delay_ms(1); 
}

#ifdef XTAL_DUAL
//-----------------------------------------------------------------------------
//函 数 名：selXtal
//功能描述：Xtal选择
//函数说明：
//全局变量：
//输    入：选择24MHz: 0            选择24.576MHz: 1
//返    回：无
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M 
//版    本：V1.7    在切换晶振的过程中，把ifadc也rst一下                                         
//-----------------------------------------------------------------------------
static void selXtal(bit xtalSel)
{
     UINT8 regx,state;
     
     xtalSel ^= XTAL_SEL1;
     regx = KT_Bus_Read(0x0012,chipSel);
     if(((regx&0x80)>>7)!=xtalSel) //当前的选择是xtal1
     {
         if(xtalSel==1)
        {
            KT_Bus_Write(0x0012,regx|0x80,chipSel);
        }
        else
        {
            KT_Bus_Write(0x0012,regx&~0x80,chipSel);
        }
        do
        {
            regx=KT_Bus_Read(0x00a0,chipSel);
        }while(!(regx&0x02));
		regx=KT_Bus_Read(0x00a0,chipSel);
        KT_Bus_Write(0x00a0,regx|0x02,chipSel);

        regx = KT_Bus_Read(0x0015,chipSel); //rst dll
        KT_Bus_Write(0x0015,regx|0x40,chipSel);
		regx = KT_Bus_Read(0x002a,chipSel); //ifadc rst
        KT_Bus_Write(0x002a,regx|0x80,chipSel);
		Delay_ms(10);
        regx = KT_Bus_Read(0x0015,chipSel);      
        KT_Bus_Write(0x0015,regx&~0x40,chipSel);
        do
        {
            regx = KT_Bus_Read(0x0180,chipSel);     
            state=(regx&0x08)>>3;
        }while(!state);
		regx = KT_Bus_Read(0x002a,chipSel); //ifadc rst
        KT_Bus_Write(0x002a,regx&~0x80,chipSel);

//		regx=KT_Bus_Read(0x0042,chipSel);	//dsp_rst
//		KT_Bus_Write(0x0042,regx | 0x04,chipSel);
//		Delay_ms(1);
//		regx=KT_Bus_Read(0x0042,chipSel);
//		KT_Bus_Write(0x0042,regx &~ 0x04,chipSel); 
     }
}
#ifdef NEW_SEL_XTAL_MODE
//-----------------------------------------------------------------------------
//函 数 名：caclFreqFrac
//功能描述：根据频点和晶振的频率来计算参数
//函数说明：
//全局变量：
//输    入：INT32 Freq,INT32 xtal_freq
//返    回：UINT32的参数值
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                    
//-----------------------------------------------------------------------------
static UINT32 caclFreqFrac(INT32 Freq,INT32 xtal_freq)
{
    UINT32 temp,tempMin=xtal_freq;
    UINT8 i;
    
    for(i=1;i<4;i++)
    {
        temp = ((xtal_freq/(2*i))-abs(((Freq%(xtal_freq/i))-(xtal_freq/(2*i)))))*i;
        if(temp<tempMin)
        {
            tempMin=temp;
        }
    }
    i=8;
    temp = ((xtal_freq/(2*i))-abs(((Freq%(xtal_freq/i))-(xtal_freq/(2*i)))))*i;
    if(temp<tempMin)
    {
        tempMin=temp;
    }
    return(tempMin*(3072000/xtal_freq));
}

//使用24MHz晶振的频点
UINT32 code use24M[26] = 
{   
    490500,492000,516000,541500,556000,565500,566000,590000,614000,615000,639000,651250,688000,
    688500,712000,712250,712500,722500,736500,760500,762000,787500,810000,811500,835500,859500
};

//使用24.56MHz晶振的频点
UINT32 code use24576M[14] = 
{
    7500,9000,10000,10500,12000,13500,14000,15000,16000,16500,18000,19500,20000,22000
};


//-----------------------------------------------------------------------------
//函 数 名：caclXtal
//功能描述：根据频点选择要使用的晶体
//函数说明：
//全局变量：
//输    入：INT32 Freq
//返    回：无
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                    
//-----------------------------------------------------------------------------
static void caclXtal(INT32 Freq)
{
    UINT8 use24M_flag=0,use24576M_flag=0,i;
    UINT32 state_tmp,chan_frac_temp0,chan_frac_temp1;
    
    for(i=0;i<26;i++)
    {
        if(Freq==use24M[i])    
        {
            use24M_flag=1;
            break;
        }
    }
    state_tmp = Freq%24000;
    for(i=0;i<14;i++)
    {
        if(state_tmp==use24576M[i])
        {
            use24576M_flag=1;
            break;
        }
    }
    if (use24M_flag)
    {
        selXtal(XTAL_24M_FREQ);
    }
    else if(use24576M_flag)
    {
        selXtal(XTAL_24P576M_FREQ);
    }
    else 
    {    
        chan_frac_temp0= caclFreqFrac(Freq,24000);
        chan_frac_temp1= caclFreqFrac(Freq,24576);
        if(chan_frac_temp0>chan_frac_temp1)
        {
            selXtal(XTAL_24M_FREQ);    
        }
        else
        {
            selXtal(XTAL_24P576M_FREQ);
        }
    }
}
#endif

#ifdef OLD_SEL_XTAL_MODE
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
BOOL KT_WirelessMicRx_Calc_ChanReg(INT32 Freq, UINT16 *chan_ptr, INT16 *chan_frac_ptr, UINT8 *chan_frac_msb_ptr, UINT16 xtal_freq)
{
	*chan_ptr = Freq / xtal_freq;
	Freq = Freq % xtal_freq; 
	*chan_frac_ptr = (Freq << 16) / xtal_freq;
	if ((Freq <= 40) && (Freq >= 0))
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
	else if ((Freq >= (xtal_freq / 2 - 40)) && (Freq <= (xtal_freq / 2 + 40)))
	{
		*chan_frac_ptr = 0x7FFF;
		*chan_frac_msb_ptr = 0;
	}
	else if (Freq > (xtal_freq >> 1))
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

//-----------------------------------------------------------------------------
//函 数 名：oldCaclXtal
//功能描述：根据频点选择要使用的晶体(跟KT0616M选择的晶体的方法一致)
//函数说明：
//全局变量：
//输    入：INT32 Freq
//返    回：无
//设 计 者：wu jinfeng              时间：2017-06-05                                
//修 改 者：                        时间：                                
//版    本：V0.1    新增加函数                   
//-----------------------------------------------------------------------------
static void oldCaclXtal(INT32 Freq)
{
	UINT16 chan0,chan1;
	INT16 chan_frac0,chan_frac1;
	UINT8 chan_frac_msb0,chan_frac_msb1;
	INT16 mod0,mod1,mod2,mod3;
	Freq = Freq << 1;
	KT_WirelessMicRx_Calc_ChanReg(Freq, &chan0, &chan_frac0, &chan_frac_msb0,24000);
	KT_WirelessMicRx_Calc_ChanReg(Freq, &chan1, &chan_frac1, &chan_frac_msb1,24576);
	mod0 = chan_frac0;
	mod1 = chan_frac1;
	mod2 = chan_frac0 << 1;
	mod3 = chan_frac1 << 1;
	if(mod0 < 0) 
		mod0 = ~mod0;			 //mod0=abs(mod0);
	if(mod1 < 0)
		mod1 = ~mod1;			 //mod1=abs(mod1);
	if(mod2 < 0)
		mod2 = ~mod2;			 //mod2=abs(mod2);
	if(mod3 < 0)
		mod3 = ~mod3;			 //mod3=abs(mod3);
	if(mod2 < mod0)
		mod0 = mod2;
	if(mod3 < mod1)
		mod1 = mod3;
	if(mod0 < mod1)
	{
		selXtal(XTAL_24P576M_FREQ);
	}
	else
	{
		selXtal(XTAL_24M_FREQ); 
	}
}
#endif
#endif

//I2S 主右从左 可以
#ifdef I2S_EN

//-----------------------------------------------------------------------------
//函 数 名：KT_WirelessMicRx_SAIInit
//功能描述：SAI工作初始化程序(serial audio interface 数字音频接口)
//函数说明：
//全局变量：
//输    入：INT32 Freq
//返    回：无
//设 计 者：Zhou Dongfeng           时间：2016-07-13                                
//修 改 者：                        时间：                                
//版    本：V0.1    For KT0656M                    
//-----------------------------------------------------------------------------
void KT_WirelessMicRx_SAIInit(void)
{
    UINT8 regx;
#ifdef DIVERSITY
    if(chipSel>chipAS) //B通道
#else
    if(chipSel>chipAM) //B通道
#endif
    {
        KT_Bus_Write(0x0050, 0x20,chipSel); //stereo_mono_sel=1 LR_SEL=0   //1 left  0 right
        KT_Bus_Write(0x0051, 0x10,chipSel); //默认情况下，mclk=12m,mclk/lrclk=128,i2s标准，对应dac double模式
        KT_Bus_Write(0x0052, 0x02,chipSel); //i2s_master_en= 1
    }
    else
    {            
        KT_Bus_Write(0x0052,0x01,chipSel); //i2s_slave_en = 1
        KT_Bus_Write(0x004d,0x01,chipSel); //自动估算，快速模式，phase=0,unlock_tw=0
        regx = KT_Bus_Read(0x004d,chipSel);
        KT_Bus_Write(0x004d,regx|0x80,chipSel); //unlock_tw_cfg_rdy = 1
        KT_Bus_Write(0x0050,0x36,chipSel); //立体声，左声道，i2s标准以及24bit data
        regx = KT_Bus_Read(0x0050,chipSel);
        KT_Bus_Write(0x0050,regx|0x40,chipSel); //i2s_slave_sync_en=1
        regx = KT_Bus_Read(0x0052,chipSel);
        KT_Bus_Write(0x0052,regx|0x08,chipSel); //i2ss_pad_sdout_oen=1    
    }
}
#endif

//I2S 主左从右 主有从无
//#ifdef I2S_EN
//void KT_WirelessMicRx_SAIInit(void)
//{
//    UINT8 regx;
//    #ifdef DIVERSITY
//    if(chipSel>chipAS)  //B通道
//    #else
//    if(chipSel>chipAM)  //B通道
//    #endif
//    {
//        KT_Bus_Write(0x0050, 0x30,chipSel);    //stereo_mono_sel=1 LR_SEL=1   //1 left  0 right
//        KT_Bus_Write(0x0051, 0x10,chipSel);//默认情况下，mclk=12m,mclk/lrclk=128,i2s标准，对应dac double模式
//        KT_Bus_Write(0x0052, 0x02,chipSel);//i2s_master_en= 1
//    }
//    else
//    {            
//        KT_Bus_Write(0x0052,0x01,chipSel);//i2s_slave_en = 1
//        KT_Bus_Write(0x004d,0x01,chipSel);    //自动估算，快速模式，phase=0,unlock_tw=0
//        regx = KT_Bus_Read(0x004d,chipSel);
//        KT_Bus_Write(0x004d,regx|0x80,chipSel);//unlock_tw_cfg_rdy = 1
//        KT_Bus_Write(0x0050,0x26,chipSel);//立体声，右声道，i2s标准以及24bit data
//        regx = KT_Bus_Read(0x0050,chipSel);
//        KT_Bus_Write(0x0050,regx|0x40,chipSel);//i2s_slave_sync_en=1
//        regx = KT_Bus_Read(0x0052,chipSel);
//        KT_Bus_Write(0x0052,regx|0x08,chipSel);//i2ss_pad_sdout_oen=1    
//    }
//}
//#endif

//左对齐 主左从右 可以
//#ifdef I2S_EN
//void KT_WirelessMicRx_SAIInit(void)
//{
//    UINT8 regx;
//    #ifdef DIVERSITY
//    if(chipSel>chipAS)  //B通道
//    #else
//    if(chipSel>chipAM)  //B通道
//    #endif
//    {
//        KT_Bus_Write(0x0050, 0x30,chipSel);    //stereo_mono_sel=1 LR_SEL=1   //1 left  0 right
//        KT_Bus_Write(0x0051, 0x12,chipSel);//默认情况下，mclk=12m,mclk/lrclk=128,左对齐，对应dac double模式
//        KT_Bus_Write(0x0052, 0x02,chipSel);//i2s_master_en= 1
//    }
//    else
//    {            
//        KT_Bus_Write(0x0052,0x01,chipSel);//i2s_slave_en = 1
//        KT_Bus_Write(0x004d,0x01,chipSel);    //自动估算，快速模式，phase=0,unlock_tw=0
//        regx = KT_Bus_Read(0x004d,chipSel);
//        KT_Bus_Write(0x004d,regx|0x80,chipSel);//unlock_tw_cfg_rdy = 1
//        KT_Bus_Write(0x0050,0x2a,chipSel);//立体声，右声道，左对齐以及24bit data
//        regx = KT_Bus_Read(0x0050,chipSel);
//        KT_Bus_Write(0x0050,regx|0x40,chipSel);//i2s_slave_sync_en=1
//        regx = KT_Bus_Read(0x0052,chipSel);
//        KT_Bus_Write(0x0052,regx|0x08,chipSel);//i2ss_pad_sdout_oen=1    
//    }
//}
//#endif

//左对齐 主右从左 主有从无
//#ifdef I2S_EN
//void KT_WirelessMicRx_SAIInit(void)
//{
//    UINT8 regx;
//    #ifdef DIVERSITY
//    if(chipSel>chipAS)  //B通道
//    #else
//    if(chipSel>chipAM)  //B通道
//    #endif
//    {
//        KT_Bus_Write(0x0050, 0x20,chipSel);    //stereo_mono_sel=1 LR_SEL=0   //1 left  0 right
//        KT_Bus_Write(0x0051, 0x12,chipSel);//默认情况下，mclk=12m,mclk/lrclk=128,左对齐，对应dac double模式
//        KT_Bus_Write(0x0052, 0x02,chipSel);//i2s_master_en= 1
//    }
//    else
//    {            
//        KT_Bus_Write(0x0052,0x01,chipSel);//i2s_slave_en = 1
//        KT_Bus_Write(0x004d,0x01,chipSel);    //自动估算，快速模式，phase=0,unlock_tw=0
//        regx = KT_Bus_Read(0x004d,chipSel);
//        KT_Bus_Write(0x004d,regx|0x80,chipSel);//unlock_tw_cfg_rdy = 1
//        KT_Bus_Write(0x0050,0x3a,chipSel);//立体声，左声道，左对齐以及24bit data
//        regx = KT_Bus_Read(0x0050,chipSel);
//        KT_Bus_Write(0x0050,regx|0x40,chipSel);//i2s_slave_sync_en=1
//        regx = KT_Bus_Read(0x0052,chipSel);
//        KT_Bus_Write(0x0052,regx|0x08,chipSel);//i2ss_pad_sdout_oen=1    
//    }
//}
//#endif

//右对齐 主右从左 可以
//#ifdef I2S_EN
//void KT_WirelessMicRx_SAIInit(void)
//{
//    UINT8 regx;
//    #ifdef DIVERSITY
//    if(chipSel>chipAS)  //B通道
//    #else
//    if(chipSel>chipAM)  //B通道
//    #endif
//    {
//        KT_Bus_Write(0x0050, 0x20,chipSel);    //stereo_mono_sel=1 LR_SEL=0   //1 left  0 right
//        KT_Bus_Write(0x0051, 0x13,chipSel);//默认情况下，mclk=12m,mclk/lrclk=128,右对齐，对应dac double模式
//        KT_Bus_Write(0x0052, 0x02,chipSel);//i2s_master_en= 1
//    }
//    else
//    {            
//        KT_Bus_Write(0x0052,0x01,chipSel);//i2s_slave_en = 1
//        KT_Bus_Write(0x004d,0x01,chipSel);    //自动估算，快速模式，phase=0,unlock_tw=0
//        regx = KT_Bus_Read(0x004d,chipSel);
//        KT_Bus_Write(0x004d,regx|0x80,chipSel);//unlock_tw_cfg_rdy = 1
//        KT_Bus_Write(0x0050,0x3e,chipSel);//立体声，左声道，右对齐以及24bit data
//        regx = KT_Bus_Read(0x0050,chipSel);
//        KT_Bus_Write(0x0050,regx|0x40,chipSel);//i2s_slave_sync_en=1
//        regx = KT_Bus_Read(0x0052,chipSel);
//        KT_Bus_Write(0x0052,regx|0x08,chipSel);//i2ss_pad_sdout_oen=1    
//    }
//}
//#endif

//右对齐 主左从右 主有从无 
//#ifdef I2S_EN
//void KT_WirelessMicRx_SAIInit(void)
//{
//    UINT8 regx;
//    #ifdef DIVERSITY
//    if(chipSel>chipAS)  //B通道
//    #else
//    if(chipSel>chipAM)  //B通道
//    #endif
//    {
//        KT_Bus_Write(0x0050, 0x30,chipSel);    //stereo_mono_sel=1 LR_SEL=1   //1 left  0 right
//        KT_Bus_Write(0x0051, 0x13,chipSel);//默认情况下，mclk=12m,mclk/lrclk=128,右对齐，对应dac double模式
//        KT_Bus_Write(0x0052, 0x02,chipSel);//i2s_master_en= 1
//    }
//    else
//    {            
//        KT_Bus_Write(0x0052,0x01,chipSel);//i2s_slave_en = 1
//        KT_Bus_Write(0x004d,0x01,chipSel);    //自动估算，快速模式，phase=0,unlock_tw=0
//        regx = KT_Bus_Read(0x004d,chipSel);
//        KT_Bus_Write(0x004d,regx|0x80,chipSel);//unlock_tw_cfg_rdy = 1
//        KT_Bus_Write(0x0050,0x2e,chipSel);//立体声，右声道，右对齐以及24bit data
//        regx = KT_Bus_Read(0x0050,chipSel);
//        KT_Bus_Write(0x0050,regx|0x40,chipSel);//i2s_slave_sync_en=1
//        regx = KT_Bus_Read(0x0052,chipSel);
//        KT_Bus_Write(0x0052,regx|0x08,chipSel);//i2ss_pad_sdout_oen=1    
//    }
//}
//#endif
