//*****************************************************************************
//  File Name: main.c
//  Function:  KT Wireless Mic Receiver Products Demoboard
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V1.0    2012-08-28  Initial draft
//  V1.7    2017-02-10  删除KT_WirelessMicRx_CheckAUXCH(辅助信道状态变化监测程序)函数
//                  修改KT_MicRX_Batter_Detecter函数，使其读取电池电量信息的时候与
//                  发射端（KT0646M）程序位置相对应
//  V1.4N    2017-04-01 根据当前是主路或者从路显示相对应的RSSI及SNR,pilot及BPSK值(Main.c)
//						上电的时候先把chipen拉低再拉高(Main.c)
//						把I2C的速度改快了(I2C.c)
//						根据最新的命名规则进行了版本号修改
//  V1.5     2017-04-21 在主循环里面一直读取导频和SNR的值，并根据最新的值决定是否mute(Main.c)
//  V1.6     2017-06-05 原来读写电池电压的寄存器为0x029,应该为0x0249
//						把rfIntCtl();pilotMuteRefresh();snrMuteRefresh();移到了驱动文件(Main.c)
//  V1.7     2017-06-28 增加了BATTERY_Display函数，用来显示接收机的电池电压(Main.c)
//  V1.8     2017-09-18 增加了根据搜台功能宏定义开关来决定一些代码
//*****************************************************************************

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <KT_WirelessMicRxdrv.h>
#include <intrins.h>
#include <stdio.h>
#include "LCD_KEY.h"
#include "I2C.h"
#include "main.h"
#include "math.h"


#define debug

#ifdef debug
UINT8 DisplayRegx(UINT16 regx,UINT8 x,UINT8 y);
#endif

void displayChanFreq(UINT8 channel);

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
//UINT8 bChannel;

extern UINT8 MorSSelect;

enum statusName currentStatus=normal; //Lcd显示的功能状态
UINT8 chipSel; //芯片选择指示，表示当前选择的哪颗芯片
UINT8 currentChannel; //通道指示

UINT8 Flag_AUTOMUTE[chipNumb]; //MUTE状态标识
UINT8 Flag_PKGSYNC[chipNumb]; //包同步状态标识
UINT8 Flag_PILOT[chipNumb]; //导频状态标识

UINT8 Key_UP_flag=0; //按键长按快速往上加标志状态
UINT8 Key_DOWN_flag=0; //按键长按快速往下减标志状态

UINT8 setNum=0; // 设置菜单之显示选择

UINT8 echoStatus=0; //菜单显示之echo状态指示
UINT8 echoSel=0; //echo菜单显示选择

UINT8 equalierStatus=0; //菜单显示之equalier状态指示
UINT8 equalierSel=0; //equalier菜单显示选择

UINT8 exciterStatus=0; //菜单显示之exciter状态指示
UINT8 exciterSel=0; //exciter菜单显示选择

//通道A 频率及音效信息表
soundEffect data soundA = 
{
    740150,0,0,0,1,1,10,23,{12,12,12,12,12,12,12,12,12,12,12,12,12,12,12},1,1
};

#ifdef TWOCHANNEL
//通道B 频率及音效信息表
soundEffect data soundB = 
{
    755150,0,0,0,1,1,10,23,{12,12,12,12,12,12,12,12,12,12,12,12,12,12,12},1,1
};
#endif

//均衡器频段划分，中心频率常量表
float code equalizerFreq[15] = 
{
    25.0,40.0,63.0,100.0,160.0,250.0,400.0,630.0,1.0,1.6,2.5,4.0,6.3,10.0,16.0
};

//通道信息表指针变量
pSoundEffect pChangeSound;

//临时存储缓冲区
UINT8 data strTemp[20];

StateMachine idata StateMachineA = 
{
	READ_RSSI,0,3,0,0,0,0,0,0,0,0,0,	
};

#ifdef TWOCHANNEL
StateMachine idata StateMachineB = 
{
	READ_RSSI,0,3,0,0,0,0,0,0,0,0,0,	
};
#endif
StateMachine idata * data pState;
//pStateMachine data pState;
UINT16 refreshTimer=0;
UINT16 timerCounter=0;
//-----------------------------------------------------------------------------
// 芯片调试阶段使用
//-----------------------------------------------------------------------------
//#define BURNOTP
#ifdef BURNOTP

//-----------------------------------------------------------------------------
//函 数 名：KT_Otp_Write
//功能描述：KT_Otp_Write
//函数说明：KT_Otp_Write
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_Otp_Write(UINT16 Register_Address, UINT8 Byte_Data)
{
    I2C_Byte_Write(KTWirelessMicRxw_addressOTP,Register_Address,Byte_Data);
}

//-----------------------------------------------------------------------------
//函 数 名：KT_Otp_Read
//功能描述：KT_Otp_Read
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 KT_Otp_Read(UINT16 Register_Address)
{
    return(I2C_Byte_Read(KTWirelessMicRxr_addressOTP, Register_Address));
}

//-----------------------------------------------------------------------------
//函 数 名：Otp_Prog_One
//功能描述：Otp_Prog_One
//函数说明：
//全局变量：无
//输    入：UINT16 Otp_Addr,UINT8 WReg
//返    回：0: OK                   其他: 错误
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 Otp_Prog_One(UINT16 Otp_Addr,UINT8 WReg)
{
    static UINT8 regx,regy,regz,RReg;
    UINT8 flag_fail,num;
    UINT16 addr;
    
    //prog_read1
    KT_Otp_Write(0x03a2,0x09);
    KT_Otp_Write(0x03a3,0xc4);
    KT_Otp_Write(0x03a0,0x40);
    regz = KT_Otp_Read(0xf840);
    flag_fail=0;
    if(1==(Otp_Addr%2))
    {
        addr = Otp_Addr*2 - 1 + 0xf800;
        regy = KT_Otp_Read(addr);
        if(    (Otp_Addr>=0 && Otp_Addr<=0x03 && (0x01 == (regz&0x01)))
            || (Otp_Addr>=0x04 && Otp_Addr<=0x05 && (0x02 == (regz&0x02))) 
            || (Otp_Addr>=0x06 && Otp_Addr<=0x0f && (0x04 == (regz&0x04)))
            || (Otp_Addr>=0xe3 && Otp_Addr<=0x3ff && (0x08 == (regz&0x08)))
            || (Otp_Addr>=0x10 && Otp_Addr<=0x13 && (0x10 == (regz&0x10)))
            || (Otp_Addr>=0x14 && Otp_Addr<=0x17 && (0x20 == (regz&0x20)))
            || (Otp_Addr>=0x18 && Otp_Addr<=0x1b && (0x40 == (regz&0x40)))
            || (Otp_Addr>=0x1c && Otp_Addr<=0x1f && (0x80 == (regz&0x80)))
        )
            RReg = regy;
        else
            RReg = WReg & regy;

    }
    else
    {
        addr = Otp_Addr*2 + 0xf800;
        regy = KT_Otp_Read(addr);
        if(    (Otp_Addr>=0 && Otp_Addr<=0x03 && (0x01 == (regz&0x01)))
            || (Otp_Addr>=0x04 && Otp_Addr<=0x05 && (0x02 == (regz&0x02))) 
            || (Otp_Addr>=0x06 && Otp_Addr<=0x0f && (0x04 == (regz&0x04)))
            || (Otp_Addr>=0xe3 && Otp_Addr<=0x3ff && (0x08 == (regz&0x08)))
            || (Otp_Addr>=0x10 && Otp_Addr<=0x13 && (0x10 == (regz&0x10)))
            || (Otp_Addr>=0x14 && Otp_Addr<=0x17 && (0x20 == (regz&0x20)))
            || (Otp_Addr>=0x18 && Otp_Addr<=0x1b && (0x40 == (regz&0x40)))
            || (Otp_Addr>=0x1c && Otp_Addr<=0x1f && (0x80 == (regz&0x80)))
        )
            RReg = regy;
        else
            RReg = WReg | regy;
    }
    
    KT_Otp_Write(addr,WReg);
    Delay_ms(2);
    regx= KT_Otp_Read(addr);
    if(regx != RReg)
    {
        //prog read2
        KT_Otp_Write(0x03a2,0x30);
        KT_Otp_Write(0x03a3,0xd4);
        KT_Otp_Write(0x03a0,0x20);
        for(num=0;num<16;num++)
        {
            KT_Otp_Write(addr,WReg);
            Delay_ms(5);
            regx = KT_Otp_Read(addr);
            if(regx ==  RReg)
                break;
        }
        if(num == 16)// prog_read2 fail 16 times
        {
            flag_fail=1;
            printf("prog fail at %.4x(%.4x) with%bx when init=%bx,hope=%bx\n",
                    addr,Otp_Addr,regx,regy,RReg);
        }
    }
    regx = KT_Otp_Read(addr+2);
    if(regx != RReg)
    {
        //prog read2
        KT_Otp_Write(0x03a2,0x30);
        KT_Otp_Write(0x03a3,0xd4);
        KT_Otp_Write(0x03a0,0x20);
        for(num=0;num<16;num++)
        {
            KT_Otp_Write(addr+2,WReg);
            Delay_ms(5);
            regx = KT_Otp_Read(addr+2);
            if(regx ==  RReg)
                break;
        }
        if(num == 16)// prog_read2 fail 16 times
        {
            flag_fail=2;
            printf("prog fail at %.4x(%.4x) with%bx when init=%bx,hope=%bx\n",
                    addr+2,Otp_Addr,regx,regy,RReg);
        }
    }
    return flag_fail;
}

//-----------------------------------------------------------------------------
//函 数 名：burnOtp
//功能描述：burnOtp
//函数说明：烧otp的时候只有一个芯片的chip_en上电
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：wu jinfeng              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void burnOtp(void)
{
    UINT8 state=0;
    UINT8 regx;
    CHIP_EN_AM  = 0;
	CHIP_EN_AS  = 0;
	CHIP_EN_BM  = 0;
	CHIP_EN_BS  = 0;
	Delay_ms(10);
	Delay_us(250);
	CHIP_EN_AM  = 1;
//	CHIP_EN_AS  = 1;
//	CHIP_EN_BM  = 1;
//	CHIP_EN_BS  = 1;
	Delay_ms(15);
    do
    {
        KT_Otp_Write(0x07ff, 0x5a); //WRITE PROG REQ
        regx= KT_Otp_Read(0x03a4);
        state = (regx&0x10)>>4;
    }while(!(state)); 

	regx= KT_Otp_Read(0xf800);	   //read bondotp 0x00
	regx= KT_Otp_Read(0xf801);	   //read bondotp 0x01
	regx= KT_Otp_Read(0xf804);	   //read bondotp 0x02

    //Otp_Prog_One(0x00,0xff);//kt0656m
    //Otp_Prog_One(0x01,0xb0);//kt0656m

	//Otp_Prog_One(0x00,0xff);//kt0655m
	//Otp_Prog_One(0x01,0xb2);//kt0655m

	//Otp_Prog_One(0x00,0x3e);//kt0652
	//Otp_Prog_One(0x01,0x36);//kt0652
	//
	//Otp_Prog_One(0x00,0x7e);//kt0653D
	//Otp_Prog_One(0x01,0x32);//kt0653D

//    Otp_Prog_One(0x00,0xe0);//kt0650
//    Otp_Prog_One(0x01,0x44);//kt0650
//    Otp_Prog_One(0x02,0x80);//kt0650
//
//    Otp_Prog_One(0x20,0x01);

    LCD_Init();
    while(1);
}
#endif

//-----------------------------------------------------------------------------
//函 数 名：i2sInit
//功能描述：I2S初始化
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void i2sInit(void)
{
    #ifdef I2S_EN
    for(chipSel=0;chipSel<chipNumb;chipSel++)
    {
        KT_WirelessMicRx_SAIInit();    
    }
    #endif
}

void freqSearch(UINT32 startFreq,UINT32 stopFreq)
{
	UINT32 freqTemp,rssiMinFreq,rssiMaxFreq;
	UINT8 rssiMin=0xff,rssiMax=0,rssi;
	display_ascii_string(0,2,"start");
	for(freqTemp=startFreq;freqTemp<stopFreq;freqTemp+=250)
	{
		KT_WirelessMicRx_FastTune(freqTemp);
		rssi=KT_WirelessMicRx_GetRSSI();
		if(rssi<rssiMin)
		{
			rssiMin=rssi;
			rssiMinFreq=freqTemp;
		}
		if(rssi>rssiMax)
		{
			rssiMax=rssi;
			rssiMaxFreq=freqTemp;
		}
	}
	KT_WirelessMicRx_Tune(rssiMaxFreq);
	sprintf(strTemp,"%ld",rssiMaxFreq);
    display_ascii_string(0,2,strTemp);
}

//-----------------------------------------------------------------------------
//函 数 名：main
//功能描述：主程序
//函数说明：
//全局变量：bChannel; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：KANG Hekai              时间：2013-03-29
//版    本：V1.0
//          V1.1    2012-12-17    SQUEAL is added.
//          V1.2    2012-12-18    Simplify main.
//          V1.3    2013-03-29    增加LDO_CTRL = 1解决关机后反复上电和死机问题;
//          V1.4    2013-04-24    将前面板中间的按键作为导频检测的开关;
//-----------------------------------------------------------------------------
void main (void)
{
	PCA0MD &= ~0x40; // WDTE = 0 (clear watchdog timer enable)
    SYSCLK_Init (); // Initialize system clock to 12MHz
    PORT_Init (); // Initialize crossbar and GPIO
//  burnOtp();
    DEMO_Init();
    dacInit();
    i2sInit();
	Timer0_Init(); // Initialize the Timer0
    while (1)
    {
		LCD_Refresh();
        KT_MicRX_PowerDown_Detecter();
        keyWork(Key_Scan());
	#ifdef AUTO_SEARCH_FREQ
		#ifdef TWOCHANNEL
		if((StateMachineA.seek_mute==0)&&(StateMachineB.seek_mute==0))
		#else
		if(StateMachineA.seek_mute==0)
		#endif
	#endif
		{
			KT_WirelessMicRx_Patch();
		}
	#ifdef AUTO_SEARCH_FREQ
		rfIntCtl();		
		SeekFreqStateMachine(CH_A);
		SeekFreqStateMachine(CH_B);
	#endif
    }
}

void rfIntCtlChip(UINT8 chip)
{
	UINT8 regx;
	if(!(0x7f&KT_Bus_Read(0x005b,chip)))
	{
		regx=KT_Bus_Read(0x0053,chip);
		KT_Bus_Write(0x0053,regx&~0x40,chip);
	}
	else
	{
		regx=KT_Bus_Read(0x0053,chip);
		KT_Bus_Write(0x0053,regx|0x40,chip);
	}
}

void pilotMuteRefreshChip(UINT8 chip)
{
	UINT8 regx;
	chipSel=chip;
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

void snrMuteRefreshChip(UINT8 chip)
{
	UINT8 regx;
	chipSel=chip;
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

void KT_WirelessMicRx_PatchChip(UINT8 chip)
{
	rfIntCtlChip(chip);
	pilotMuteRefreshChip(chip);
	snrMuteRefreshChip(chip);
}
//-----------------------------------------------------------------------------
//函 数 名：CS4398Init
//功能描述：CS4398芯片初始化
//函数说明：
//全局变量：无
//输    入：UINT8 address: device addr
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void CS4398Init(UINT8 address)
{
    UINT8 regx;
    regx=I2S_Byte_Read(address,0x08); //CPEN=1
    regx|=0x40;
    I2S_Byte_Write(address,0x08,regx);
    regx&=~0x80;
    I2S_Byte_Write(address,0x08,regx); //PDN=0
    I2S_Byte_Write(address,0x02,0x11); //I2S mode
//    I2S_Byte_Write(address,0x02,0x01);//左对齐 mode
//    I2S_Byte_Write(address,0x02,0x31);//右对齐 mode
//    I2S_Byte_Write(address,0x05,0x12); //vol=-9db
//    I2S_Byte_Write(address,0x06,0x12); //vol=-9db
}

//-----------------------------------------------------------------------------
//函 数 名：dacInit
//功能描述：dacInit
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void dacInit(void)
{
    CS4398Init(CS4398AddrA);
}

//-----------------------------------------------------------------------------
//函 数 名：menuReturn
//功能描述：menuReturn
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void menuReturn(void)
{
    if(currentStatus==setExciter)
    {
        if(exciterStatus!=0)
        {
            exciterStatus=0;
        }
        else
        {
            currentStatus=selectSet;
        }
    }
    else if(currentStatus==setEcho)
    {
        if(echoStatus!=0)
        {
            echoStatus=0;
        }
        else
        {
            currentStatus=selectSet;
        }
    }
    else if(currentStatus==setEqualizer)
    {
        if(equalierStatus!=0)
        {
            equalierStatus=0;
        }
        else
        {
            currentStatus=selectSet;
        }
    }
    else
    {
        currentStatus=normal;
        toNormal();
    }
    if(currentStatus==selectSet)
    {
        clear_screen(); //清屏
    }
}

//-----------------------------------------------------------------------------
//函 数 名：keyWork
//功能描述：keyWork
//函数说明：
//全局变量：无
//输    入：UINT8 keyName: 键码
//返    回：无
//设 计 者：WJF                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void keyWork(UINT8 keyName)
{
    if(keyName==0)
		return;
	if(currentStatus==normal)
    {
        if(keyName<=3)
        {
            chipSel = chipAM;
            pChangeSound= &soundA;
            currentChannel=channelA;
        }
    #ifdef TWOCHANNEL
        else
        {        
            keyName-=3;            
            chipSel = chipBM;
            pChangeSound= &soundB;            
            currentChannel=channelB;
        }
    #endif        
        switch(keyName)
        {
			case 1: KT_MicRX_Next_Fre();            break;
			case 2: KT_MicRX_Previous_Fre();        break;
            case 3: setMenu();                      break;
            default: Key_DOWN_flag=0;Key_UP_flag=0; break; 
        }
    }
    else
    {
        if(currentChannel==channelA)
        {
            pChangeSound= &soundA;
			chipSel = chipAM;
			if(keyName>=4)
            {
                if(keyName==6)
                {
                    menuReturn();
                }
                return;
            }
        }
        else
        {
            #ifdef TWOCHANNEL
			pChangeSound= &soundB;
			chipSel = chipBM;
			#endif 
			if(keyName<=3)
            {
                if(keyName==3)
                {
                    menuReturn();
                }
                return;
            }
            keyName-=3;
        }
        switch(keyName)
        {
            case 2: MenuAdd();                        break;
            case 1: MenuDec();                        break;
            case 3: MenuSelect();                     break;
            default: Key_DOWN_flag=0;Key_UP_flag=0;   break; 
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：setMenu
//功能描述：setMenu
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void setMenu(void)
{
    currentStatus = selectSet;
    clear_screen();
    displayMenu();    
}

//lcd显示的设置菜单状态字符串
char code * code pStr[3]={"setEcho","setEqua","setExci"};

//-----------------------------------------------------------------------------
//函 数 名：displayMenu
//功能描述：菜单显示
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void displayMenu(void)
{
    UINT8 i;
    UINT8 data * pTemp;
    
    if(currentChannel==channelA)
    {
        display_ascii_string(0,15,"A");
		pChangeSound = &soundA;
    }
    else
    {
        #ifdef TWOCHANNEL
		pChangeSound= &soundB;
		#endif 
		display_ascii_string(0,15,"B");
    }
    pTemp=&(pChangeSound->echoFlag);
    for(i=0;i<3;i++)
    {
        display_ascii_string(i,0,pStr[i]);
    }
    
    for(i=0;i<3;i++)
    {
        if(*(pTemp+i))
        {
            display_ascii_string(i,8,"ON");
        }
        else
        {
            display_ascii_string(i,8,"OFF");
        }
    }
    
    if(pChangeSound->diversityFlag)
    {
        display_ascii_string(3,0,"diverON ");
    }
    else
    {
        display_ascii_string(3,0,"diverOFF");
    }
    
    if(pChangeSound->pilotFlag)
    {
        display_ascii_string(3,8,"pilotON ");
    }
    else
    {
        display_ascii_string(3,8,"pilotOFF");
    }
}

//-----------------------------------------------------------------------------
//函 数 名：displayMenu
//功能描述：freshMenu
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void freshMenu()
{
    static bit showFlag;
    
    showFlag^=1;
    if(showFlag)
    {
        display_ascii_string(setNum/2,(setNum%2)*8,"        ");    
    }
    else
    {
        displayMenu();
    }
}

//-----------------------------------------------------------------------------
//函 数 名：MenuAdd
//功能描述：MenuAdd
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void MenuAdd()
{
    if(currentStatus == selectSet)
    {
        if(++setNum>7)
            setNum=0;
    }
    if(currentStatus == setEcho)
    {
        if(echoStatus==0)
        {
            if(++echoSel>1)
                echoSel=0;
        }
        else 
        {
            if(echoStatus==1)
            {
                if(++(pChangeSound->echoDelay)>23)
                    pChangeSound->echoDelay=0;
            }
            else
            {
                if(++(pChangeSound->echoDepth)>25)
                    pChangeSound->echoDepth=0;
            }
            KT_WirelessMicRx_SetEcho(pChangeSound->echoDepth,pChangeSound->echoDelay);
        #ifdef DIVERSITY            
            chipSel++;
            KT_WirelessMicRx_SetEcho(pChangeSound->echoDepth,pChangeSound->echoDelay);
            chipSel--;
        #endif
        }
    }
    if(currentStatus==setEqualizer)
    {
        if(equalierStatus==0)
        {
            if(++equalierSel>14)
                equalierSel=0;
        }
        else
        {
            if(++(pChangeSound->equalier16Sel[equalierSel])>24)
            {
                pChangeSound->equalier16Sel[equalierSel]=0;
            }
            KT_WirelessMicRx_SetEqualizer(equalierSel,(pChangeSound->equalier16Sel)[equalierSel]);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetEqualizer(equalierSel,(pChangeSound->equalier16Sel)[equalierSel]);
            chipSel--;
        #endif
        }
    }
    if(currentStatus == setExciter)
    {
        if(exciterStatus==0)
        {
            if(++exciterSel>1)
                exciterSel=0;
        }
        else 
        {
            if(exciterSel==0)
            {
                if(++(pChangeSound->exciterOdd)>6)
                    pChangeSound->exciterOdd=0;
            }
            else
            {
                if(++(pChangeSound->exciterEven)>6)
                    pChangeSound->exciterEven=0;
            }
            KT_WirelessMicRx_SetExciter(pChangeSound->exciterOdd,pChangeSound->exciterEven);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetExciter(pChangeSound->exciterOdd,pChangeSound->exciterEven);
            chipSel--;
        #endif
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：MenuDec
//功能描述：MenuDec
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void MenuDec()
{
    if(currentStatus == selectSet)
    {
        if(setNum--<1)
            setNum=7;
    }
    if(currentStatus == setEcho)
    {
        if(echoStatus==0)
        {
            if(echoSel--<1)
                echoSel=1;
        }
        else 
        {
            if(echoStatus==1)
            {
                if(pChangeSound->echoDelay--<1)
                    pChangeSound->echoDelay=23;
            }
            else
            {
                if(pChangeSound->echoDepth--<1)
                    pChangeSound->echoDepth=25;
            }
            KT_WirelessMicRx_SetEcho(pChangeSound->echoDepth,pChangeSound->echoDelay);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetEcho(pChangeSound->echoDepth,pChangeSound->echoDelay);
            chipSel--;
        #endif
        }
    }
    if(currentStatus==setEqualizer)
    {
        if(equalierStatus==0)
        {
            if(equalierSel--<1)
                equalierSel=14;
        }
        else
        {
            if((pChangeSound->equalier16Sel)[equalierSel]--<1)
            {
                (pChangeSound->equalier16Sel)[equalierSel]=24;
            }
            KT_WirelessMicRx_SetEqualizer(equalierSel,(pChangeSound->equalier16Sel)[equalierSel]);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetEqualizer(equalierSel,(pChangeSound->equalier16Sel)[equalierSel]);
            chipSel--;
        #endif
        }
    }
    if(currentStatus == setExciter)
    {
        if(exciterStatus==0)
        {
            if(exciterSel--<1)
                exciterSel=1;
        }
        else 
        {
            if(exciterSel==0)
            {
                if(pChangeSound->exciterOdd--<1)
                    pChangeSound->exciterOdd=6;
            }
            else
            {
                if(pChangeSound->exciterEven--<1)
                    pChangeSound->exciterEven=6;
            }
            KT_WirelessMicRx_SetExciter(pChangeSound->exciterOdd,pChangeSound->exciterEven);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetExciter(pChangeSound->exciterOdd,pChangeSound->exciterEven);
            chipSel--;
        #endif
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：toNormal
//功能描述：toNormal
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void toNormal()
{
    currentStatus = normal;
    clear_screen(); //清屏
    display_ascii_string(0,0,"F:      F:");    //频率
    display_ascii_string(1,0,"V:      V:");    //电量
    display_ascii_string(2,0,"S:      S:");    //RSSI
    display_ascii_string(3,0,"A:      A:");    //音量
    if(StateMachineA.seek_mute==0)
	{
		displayChanFreq(channelA);
	}
	#ifdef TWOCHANNEL
    if(StateMachineB.seek_mute==0)
	{
		displayChanFreq(channelB);
	}
	#endif
	refreshTimer=5000;
//	displayFreq();
}

//ECHO Delay值常数表
UINT8 code EchoDelyTab[] = 
{
    22,24,27,29, 32,35,39,43,
    47,52,57,63, 69,76,84,92,
    101,111,122,135, 148,163,179,197
};

//-----------------------------------------------------------------------------
//函 数 名：MenuSelect
//功能描述：MenuSelect
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void MenuSelect()
{
    UINT8 regx;
    
    if(currentStatus == selectSet)
    {
//        if(setNum%2)
        {
            clear_screen();
        }
        
        if(setNum==0)
        {
            currentStatus = setEcho;
            echoStatus=0;
            display_ascii_string(0,4,"setEcho");
            display_ascii_string(1,0,"Delay   Ratio");
            sprintf(strTemp,"%03bums",EchoDelyTab[pChangeSound->echoDelay]);
            display_ascii_string(2,0,strTemp);
            if(pChangeSound->echoDepth)
            {
                sprintf(strTemp,"%02bd/32",pChangeSound->echoDepth);
            }
            else
            {
                sprintf(strTemp,"%02bd   ",pChangeSound->echoDepth);
            }
            display_ascii_string(2,8,strTemp);
        }
        else if(setNum==1)
        {
            pChangeSound->echoFlag^=0x01;
            if(pChangeSound->echoFlag)
            {
                display_ascii_string(0,8,"ON ");
            }
            else
            {
                display_ascii_string(0,8,"OFF");
            }
            KT_WirelessMicRx_SW_Echo(pChangeSound->echoFlag);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SW_Echo(pChangeSound->echoFlag);
            chipSel--;
        #endif
        }
        else if(setNum==2)
        {
            currentStatus = setEqualizer;
            equalierStatus=0;
            display_ascii_string(0,2,"setEqualier");
        }
        else if(setNum==3)
        {
            pChangeSound->equalierFlag^=0x01;
            if(pChangeSound->equalierFlag)
            {
                display_ascii_string(1,8,"ON ");
            }
            else
            {
                display_ascii_string(1,8,"OFF");
            }
            KT_WirelessMicRx_SW_Equalizer(pChangeSound->equalierFlag);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SW_Equalizer(pChangeSound->equalierFlag);
            chipSel--;
        #endif
        }
        else if(setNum==4)
        {
            currentStatus = setExciter;
            exciterStatus=0;
            display_ascii_string(0,3,"setExciter");
        }
        else if(setNum==5)
        {
            pChangeSound->exciterFlag^=0x01;
            if(pChangeSound->exciterFlag)
            {
                display_ascii_string(2,8,"ON ");
            }
            else
            {
                display_ascii_string(2,8,"OFF");
            }
            KT_WirelessMicRx_SW_Exciter(pChangeSound->exciterFlag);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SW_Exciter(pChangeSound->exciterFlag);
            chipSel--;
        #endif
        }
        else if(setNum==6)
        {
            pChangeSound->diversityFlag^=0x01;
            if(pChangeSound->diversityFlag)
            {
                display_ascii_string(3,0,"diverON ");
            }
            else
            {
                display_ascii_string(3,0,"diverOFF");
            }
            KT_WirelessMicRx_SW_Diversity(pChangeSound->diversityFlag);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SW_Diversity(pChangeSound->diversityFlag);
            chipSel--;
        #endif
        }
//        else if(setNum==6)             //这是开关亚扩的，为了测试PLL SNR
//        {
//            regx = KT_Bus_Read(0x0256,chipSel);    
//            regx^=0x04;
//            if(regx&0x04)
//            {
//                display_ascii_string(3,0,"diverON ");
//            }
//            else
//            {
//                display_ascii_string(3,0,"diverOFF");
//            }
//            KT_Bus_Write(0x0256,regx,chipSel);
//            #ifdef DIVERSITY
//            chipSel++;
//            KT_Bus_Write(0x0256,regx,chipSel);
//            chipSel--;
//            #endif
//        }
        else
        {
            pChangeSound->pilotFlag^=0x01;
            KT_MicRX_Automute_Pilot_SW(pChangeSound->pilotFlag);
        #ifdef DIVERSITY
            chipSel++;
            KT_MicRX_Automute_Pilot_SW(pChangeSound->pilotFlag);
            chipSel--;
        #endif
            if(pChangeSound->pilotFlag)
            {
                display_ascii_string(3,8,"pilotON ");
                if(0x80&KT_Bus_Read(0x0209,chipSel))
                {
                    regx=KT_Bus_Read(0x0087,chipSel);
                    KT_Bus_Write(0x0087,(regx&~0x04),chipSel);
                #ifdef DIVERSITY
                    chipSel++;
                    regx=KT_Bus_Read(0x0087,chipSel);
                    KT_Bus_Write(0x0087,(regx&~0x04),chipSel);
                    chipSel--;
                #endif
                }
                else
                {
                    regx=KT_Bus_Read(0x0087,chipSel);
                    KT_Bus_Write(0x0087,(regx|0x04),chipSel);
                #ifdef DIVERSITY
                    chipSel++;
                    regx=KT_Bus_Read(0x0087,chipSel);
                    KT_Bus_Write(0x0087,(regx|0x04),chipSel);
                    chipSel--;
                #endif
                }
            }
            else
            {
                display_ascii_string(3,8,"pilotOFF");
				regx=KT_Bus_Read(0x0087,chipSel);
                    KT_Bus_Write(0x0087,(regx&~0x04),chipSel);
                #ifdef DIVERSITY
                    chipSel++;
                    regx=KT_Bus_Read(0x0087,chipSel);
                    KT_Bus_Write(0x0087,(regx&~0x04),chipSel);
                    chipSel--;
                #endif
            }
        }
        if(chipSel == chipAM)
        {
            display_ascii_string(0,15,"A");
        }
        else
        {
            display_ascii_string(0,15,"B");
        }
    }
    else if(currentStatus==setEcho)
    {
        if(echoStatus==0)
        {
            echoStatus = 1+echoSel;
            display_ascii_string(1,0,"Delay   Ratio   ");
        }
        else
        {
            KT_WirelessMicRx_SetEcho(pChangeSound->echoDepth,pChangeSound->echoDelay);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetEcho(pChangeSound->echoDepth,pChangeSound->echoDelay);
            chipSel--;
        #endif
            echoStatus=0;
            sprintf(strTemp,"%03bums   %02bd/32",EchoDelyTab[pChangeSound->echoDelay],
            pChangeSound->echoDepth);
            display_ascii_string(2,0,strTemp);
        }
    }
    else if(currentStatus==setEqualizer)
    {
        if(equalierStatus==0)
        {
            equalierStatus = 1;
            if(equalizerFreq[equalierSel]>=25)//显示Hz
            {
                sprintf(strTemp,"%5uHz:",(UINT16)equalizerFreq[equalierSel]);
                display_ascii_string((equalierSel%3)+1,0,strTemp);
            }
            else
            {
                sprintf(strTemp,"%4.1fkHz:",(float)equalizerFreq[equalierSel]);
                display_ascii_string((equalierSel%3)+1,0,strTemp);
            }
        }
        else
        {
            KT_WirelessMicRx_SetEqualizer(equalierSel,(pChangeSound->equalier16Sel)[equalierSel]);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetEqualizer(equalierSel,(pChangeSound->equalier16Sel)[equalierSel]);
            chipSel--;
        #endif
            equalierStatus=0;
//            toNormal();
        }
    }
    else if(currentStatus==setExciter)
    {
        if(exciterStatus==0)
        {
            exciterStatus = 1;
            display_ascii_string(1,0,"Odd     Even    ");
        }
        else
        {
            KT_WirelessMicRx_SetExciter(pChangeSound->exciterOdd,pChangeSound->exciterEven);
        #ifdef DIVERSITY
            chipSel++;
            KT_WirelessMicRx_SetExciter(pChangeSound->exciterOdd,pChangeSound->exciterEven);
            chipSel--;
        #endif
            exciterStatus=0;
            //            toNormal();
         }
    }
}

//激励器配置常数表
UINT8 code g_ExciterTab[] = {-100,-30, -24,-18,-12,-6,0};

//-----------------------------------------------------------------------------
//函 数 名：freshSetExciter
//功能描述：freshSetExciter
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void freshSetExciter()
{
    static bit i;
    
    i^=1;
    if(exciterStatus==0)
    {
        if(i)
        {
            display_ascii_string(1,exciterSel*8,"        ");    
        }
        else
        {
            display_ascii_string(1,0,"Odd     Even    ");
            if(0 == pChangeSound->exciterOdd)
            {
                sprintf(strTemp,"mindB   ");
            }
            else
            {
                sprintf(strTemp,"%02bddB    ",g_ExciterTab[pChangeSound->exciterOdd]);
            }
            display_ascii_string(2,0,strTemp);
            if(0 == pChangeSound->exciterEven)
            {
                sprintf(strTemp,"mindB   ");
            }
            else
            {
                sprintf(strTemp,"%02bddB    ",g_ExciterTab[pChangeSound->exciterEven]);
            }
            display_ascii_string(2,8,strTemp);
        }
    }
    else
    {
        if(i)
        {
            display_ascii_string(2,exciterSel*8,"        ");    
        }
        else
        {            
            if(0 == pChangeSound->exciterOdd)
            {
                sprintf(strTemp,"mindB   ");
            }
            else
            {
                sprintf(strTemp,"%02bddB    ",g_ExciterTab[pChangeSound->exciterOdd]);
            }
            display_ascii_string(2,0,strTemp);
            if(0 == pChangeSound->exciterEven)
            {
                sprintf(strTemp,"mindB   ");
            }
            else
            {
                sprintf(strTemp,"%02bddB    ",g_ExciterTab[pChangeSound->exciterEven]);
            }
            display_ascii_string(2,8,strTemp);
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：freshSetEqualizer
//功能描述：freshSetEqualizer
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void freshSetEqualizer()
{
    static bit i;
    UINT8 temp,j;
    
    i^=1;
    if(equalierStatus==0)
    {
        if(i)
        {
            display_ascii_string((equalierSel%3)+1,0,"        ");    
        }
        else
        {
            temp = equalierSel/3;
            for(j=0;j<3;j++)
            { 
                if(equalizerFreq[temp*3+j]>=25)//显示Hz
                {
                    sprintf(strTemp,"%5uHz:%+03bddB",(UINT16)equalizerFreq[j+temp*3],
                            (INT8)(pChangeSound->equalier16Sel)[j+temp*3]-12);
                    display_ascii_string(j+1,0,strTemp);
                }
                else
                {
                    sprintf(strTemp,"%4.1fkHz:%+03bddB",(float)equalizerFreq[j+temp*3],
                            (INT8)(pChangeSound->equalier16Sel)[j+temp*3]-12);
                    display_ascii_string(j+1,0,strTemp);
                }
            }
        }
    }
    else
    {
        if(i)
        {
            display_ascii_string((equalierSel%3)+1,8,"        ");    
        }
        else
        {
            sprintf(strTemp,"%+03bddB",(INT8)(pChangeSound->equalier16Sel)[equalierSel]-12);
            display_ascii_string((equalierSel%3)+1,8,strTemp);
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：freshSetEcho
//功能描述：freshSetEcho
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：WJF                     时间：2016-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void freshSetEcho()
{
    static bit i;
    
    i^=1;
    if(echoStatus==0)
    {
        if(i)
        {
            display_ascii_string(1,echoSel*8,"        ");    
        }
        else
        {
            display_ascii_string(1,0,"Delay   Ratio");
        }
    }
    else if(echoStatus==1) //delay
    {
        if(i)
        {
            display_ascii_string(2,0,"   ");    
        }
        else
        {
            sprintf(strTemp,"%03bums",EchoDelyTab[pChangeSound->echoDelay]);
            display_ascii_string(2,0,strTemp);
        }
    }
    else //depth
    {
        if(i)
        {
            display_ascii_string(2,8,"     ");    
        }
        else
        {
            sprintf(strTemp,"%02bd/32",pChangeSound->echoDepth);
            display_ascii_string(2,8,strTemp);
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：SYSCLK_Init
//功能描述：MCU时钟初始化
//函数说明：
// This routine initializes the system clock to use the internal 24.5MHz / 8 
// oscillator as its clock source.  Also enables missing clock detector reset.
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：KANG Hekai                时间：2013-03-29
//版    本：V1.0
//            V1.3----打开电池电压检测复位功能   
//-----------------------------------------------------------------------------
void SYSCLK_Init (void)
{
    OSCICN |= 0x03; //0xc3 // configure internal oscillator for 12MHz

    VDM0CN = 0x80; // enable VDD monitor
    Delay_ms(1);
    RSTSRC = 0x02; // enable VDD monitor as a reset source
}

//-----------------------------------------------------------------------------
//函 数 名：PORT_Init
//功能描述：MCU端口初始化
//函数说明：Configure the Crossbar and GPIO ports.使用弱上拉
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void PORT_Init (void)
{
    XBR0    = 0x00; // Disable all
    XBR1    = 0x40;    // Enable crossbar and enable weak pull-ups
    P0MDOUT    = 0x00;    // 0:漏极开路 1:推挽输出  默认为0.
//    P0MDIN    = 0xff;    // 0:模拟输入 1:数字输入  默认为1.
//    P0        = 0xff;    // 默认为1

    P1MDOUT    |= 0xf8; //P1.3-P1.7是LCD相关的GPIO口
//    P1MDIN    |= 0x0E; // 
//    P1        |= 0x0E; // 

    P2MDOUT    = 0x00;    
//    P2MDIN    |= 0x70; // 
//    P2        |= 0x70; // 

    P3MDOUT    = 0x4c;
//    P3MDIN    |= 0x04; // 
//    P3        |= 0x4c; //
    
//    P4MDOUT    |= 0x40;
//    P4        |= 0x40;
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicRX_Init
//功能描述：芯片初始化
//函数说明：
//全局变量：Memery_Frequency[]; Load_Frequency[];bChannel;
//输    入：
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void  KT_MicRX_Init(void)
{
    while(!KT_WirelessMicRx_Init());
#ifdef TWOCHANNEL
    if(chipSel<chipBM)	 //A 通道的主或者从
    {
        pChangeSound= &soundA;
        currentChannel=channelA;
    }
    else
    {
        pChangeSound= &soundB;
        currentChannel=channelB;
    }
#else
    pChangeSound= &soundA;
    currentChannel=channelA;    
#endif
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
    KT_WirelessMicRx_Volume(PRESET_VOL);
    displayFreq();    
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicRX_Next_Fre
//功能描述：频率加BAND_STEP，长按加4倍的BAND_STEP，超长按加40倍的BAND_STEP
//函数说明：
//全局变量：Memery_Frequency[i]; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：KANG Hekai                时间：2013-04-24
//版    本：V1.0
//            V1.4    2013-04-24    将A、B两路的频率范围区分开
//-----------------------------------------------------------------------------
/*
void KT_MicRX_Next_Fre (void) // 加250KHz
{
    Key_DOWN_flag = 0;
    if(currentChannel==channelA)
        Mute_A = 1;
    else
        Mute_B = 1;
    Delay_ms(50);
    if(Key_UP_flag < 20)
    {
        if(Key_UP_flag < 10)
            pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  + BAND_STEP;
        else
            pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  + BAND_STEP*4;

        Key_UP_flag = Key_UP_flag + 1;
    }
    else
    {
        pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  + BAND_STEP*20;
        Key_UP_flag = 20;
    }
    if(currentChannel==channelA)
    {
        if( (pChangeSound->Memery_Frequency  > BAND_TOP_CHA) || 
            (pChangeSound->Memery_Frequency  < BAND_BOTTOM_CHA) )
            pChangeSound->Memery_Frequency  = BAND_BOTTOM_CHA;
    }
    else
    {
        if( (pChangeSound->Memery_Frequency  > BAND_TOP_CHB) || 
            (pChangeSound->Memery_Frequency  < BAND_BOTTOM_CHB) )
            pChangeSound->Memery_Frequency  = BAND_BOTTOM_CHB;
    }

    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
#ifdef DIVERSITY 
    chipSel++;
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
    chipSel--;
#endif
    displayFreq(); 
}	 */

void KT_MicRX_Next_Fre (void) // 加250KHz
{
    if(currentChannel==channelA)
	{
        Mute_A = 1;
		do
		{
			Delay_ms(100);
			pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  + BAND_STEP;
	        if( (pChangeSound->Memery_Frequency  > BAND_TOP_CHA) || 
	            (pChangeSound->Memery_Frequency  < BAND_BOTTOM_CHA) )
	            pChangeSound->Memery_Frequency  = BAND_BOTTOM_CHA;
			displayChanFreq(channelA);
		}while(Key_UP_A==0);
	}
#ifdef TWOCHANNEL
    else
	{
        Mute_B = 1;
		do
		{
			Delay_ms(100);
			pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  + BAND_STEP;
	        if( (pChangeSound->Memery_Frequency  > BAND_TOP_CHB) || 
	            (pChangeSound->Memery_Frequency  < BAND_BOTTOM_CHB) )
	            pChangeSound->Memery_Frequency  = BAND_BOTTOM_CHB;
			displayChanFreq(channelB);
		}while(Key_UP_B==0);
	}
#endif
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
#ifdef DIVERSITY 
    chipSel++;
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
    chipSel--;
#endif
//    displayFreq(); 
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicRX_Previous_Fre
//功能描述：频率减BAND_STEP，长按减4倍的BAND_STEP，超长按减20倍的BAND_STEP
//函数说明：
//全局变量：Memery_Frequency[i]; Key_UP_flag; Key_DOWN_flag;
//输    入：
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：KANG Hekai                时间：2013-04-24
//版    本：V1.0
//          V1.3    2013-04-24    将A、B两路的频率范围区分开
//-----------------------------------------------------------------------------
/*
void KT_MicRX_Previous_Fre (void) // 减250KHz
{
    Key_UP_flag = 0;
    if(currentChannel==channelA)
        Mute_A = 1;
    else
        Mute_B = 1;
    Delay_ms(50);
    if(Key_DOWN_flag < 20)
    {
        if(Key_DOWN_flag < 10)
            pChangeSound->Memery_Frequency = pChangeSound->Memery_Frequency - BAND_STEP;
        else
            pChangeSound->Memery_Frequency = pChangeSound->Memery_Frequency - BAND_STEP*4;

        Key_DOWN_flag = Key_DOWN_flag + 1;
    }
    else
    {
        pChangeSound->Memery_Frequency = pChangeSound->Memery_Frequency - BAND_STEP*20;
        Key_DOWN_flag = 20;
    }
    if(currentChannel==channelA)
    {
        if(pChangeSound->Memery_Frequency < BAND_BOTTOM_CHA)
            pChangeSound->Memery_Frequency = BAND_TOP_CHA;
    }
    else
    {
        if(pChangeSound->Memery_Frequency < BAND_BOTTOM_CHB)
            pChangeSound->Memery_Frequency = BAND_TOP_CHB;
    }
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
#ifdef DIVERSITY 
    chipSel++;
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
    chipSel--;
#endif
    displayFreq();
}	*/

void KT_MicRX_Previous_Fre (void) // 减250KHz
{
    if(currentChannel==channelA)
	{
        Mute_A = 1;
		do
		{
			Delay_ms(100);
			pChangeSound= &soundA;
			pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  - BAND_STEP;
	        if( (pChangeSound->Memery_Frequency  > BAND_TOP_CHA) || 
	            (pChangeSound->Memery_Frequency  < BAND_BOTTOM_CHA) )
	            pChangeSound->Memery_Frequency  = BAND_TOP_CHA;
			displayChanFreq(channelA);
		}while(Key_DOWN_A==0);
	}
#ifdef TWOCHANNEL
    else
	{
        Mute_B = 1;
		do
		{
			Delay_ms(100);
			pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  - BAND_STEP;
	        if( (pChangeSound->Memery_Frequency  > BAND_TOP_CHB) || 
	            (pChangeSound->Memery_Frequency  < BAND_BOTTOM_CHB) )
	            pChangeSound->Memery_Frequency  = BAND_TOP_CHB;
			displayChanFreq(channelB);
		}while(Key_DOWN_B==0);
	}
#endif
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
#ifdef DIVERSITY 
    chipSel++;
    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
    chipSel--;
#endif
    displayFreq(); 
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicRX_PowerDown_Detecter
//功能描述：
//函数说明：
//全局变量：bChannel; Memery_Frequency[]; Load_Frequency[];
//输    入：
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-20
//修 改 者：                        时间：
//版    本：V1.1
//-----------------------------------------------------------------------------
void KT_MicRX_PowerDown_Detecter (void)
{
    if(VBAT_IN == 0)
    {
        Delay_ms(500);
        if(VBAT_IN == 0) //防止电容放电不干净反复上电
        {
            for(chipSel=0;chipSel<chipNumb;chipSel++)
            {                                                                                                                        
                KT_WirelessMicRx_Volume(0);
            }
            Delay_ms(200);
            Mute_A = 1;
            Mute_B = 1;
            Delay_ms(200);
            Save_Freq();
            Delay_ms(100);
            LDO_CTRL_WRITE0();
        }
    }
//    else
//    {
//        Delay_ms(50);
//        if(VBAT_IN == 1) //防止电容放电不干净反复上电
//            LDO_CTRL_WRITE1();
//    }
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicRX_Batter_Detecter
//功能描述：电池电压监测并显示
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：WSL                     时间：2017-02-17
//版    本：V1.0
//版    本：V1.1  读取电池电量信息的时候与发射（KT0646M）程序位置相对应
//版    本：V1.2  修正了原来要读取0x0249寄存器的时候写成了读0x029的错误
//-----------------------------------------------------------------------------
void KT_MicRX_Batter_Detecter (void)
{
    UINT16 xdata Batter_Voltage;
	UINT8 i;
	if(MorSSelect)
	{
		i=0;
	}
	else
	{
		i=1;
	}
    Batter_Voltage=KT_Bus_Read(0x0248,chipSel+i);
    Batter_Voltage<<=8;
    Batter_Voltage|= KT_Bus_Read(0x0249,chipSel+i);

    if( (Batter_Voltage <= BATTERY_MAX) && (Batter_Voltage > BATTERY_HIGHTH) )
    {
        display_ascii_string(1,2+8*(chipSel>=chipBM),"H");
    }
    else if( (Batter_Voltage <= BATTERY_HIGHTH) && (Batter_Voltage > BATTERY_MIDDLETH) )
    {
        display_ascii_string(1,2+8*(chipSel>=chipBM),"M");
    }
    else if( (Batter_Voltage <= BATTERY_MIDDLETH) && (Batter_Voltage > BATTERY_LOWTH) )
    {
        display_ascii_string(1,2+8*(chipSel>=chipBM),"L");
    }
    else
    { 
        display_ascii_string(1,2+8*(chipSel>=chipBM),"E");
    }
}

//-----------------------------------------------------------------------------
//函 数 名：Vol_Display
//功能描述：电池电压监测并显示
//函数说明：
//全局变量：无
//输    入：
//返    回：正确：1                 错误：0    
//设 计 者：KANG Hekai              时间：2012-11-30
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 Vol_Display(void)
{
    UINT8 cAF;  
    Flag_AUTOMUTE[chipSel]=KT_WirelessMicRx_Automute();
#ifdef DIVERSITY
    chipSel++;
    Flag_AUTOMUTE[chipSel]=KT_WirelessMicRx_Automute();
    chipSel--;
#endif

#ifdef DIVERSITY
    if((Flag_AUTOMUTE[chipSel] == MUTE)&&(Flag_AUTOMUTE[chipSel+1] == MUTE))
#else
    if(Flag_AUTOMUTE[chipSel] == MUTE)
#endif
    {
        display_ascii_string(3,2+8*(chipSel>=chipBM),"mu");
    }
    else
    {
        cAF=KT_WirelessMicRx_GetAF();
        sprintf(strTemp,"%bd ",cAF);
        display_ascii_string(3,2+8*(chipSel>=chipBM),strTemp);
    }
#ifdef DIVERSITY
    if((Flag_AUTOMUTE[0]==MUTE)&&(Flag_AUTOMUTE[1]==MUTE))
    {
        Mute_A = 1;
    }
    else
    {
        Mute_A = 0;
    }
    if((Flag_AUTOMUTE[2]==MUTE)&&(Flag_AUTOMUTE[3]==MUTE))
    {
        Mute_B = 1;
    }
    else
    {
        Mute_B = 0;
    }
#else
    if(Flag_AUTOMUTE[0]==MUTE)
    {
        Mute_A = 1;
    }
    else
    {
        Mute_A = 0;
    }
    if(Flag_AUTOMUTE[1]==MUTE)
    {
        Mute_B = 1;
    }
    else
    {
        Mute_B = 0;
    }
#endif
    return(1);
}

void MSDisplay(void)
{
	if(MorSSelect) 
    {
        display_ascii_string(2,4+8*(chipSel>=chipBM),"M");    
    }
    else
    {
        display_ascii_string(2,4+8*(chipSel>=chipBM),"S");    
    }
}

//-----------------------------------------------------------------------------
//函 数 名：RSSI_Display
//功能描述：信号强度显示
//函数说明：
//全局变量：无
//输    入：
//返    回：正确：1                 错误：0    
//设 计 者：KANG Hekai              时间：2012-11-30
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 RSSI_Display(void)
{
    UINT8 cRssi;
    
    cRssi=KT_WirelessMicRx_GetRSSI();
    sprintf(strTemp,"%02bx",cRssi);
    display_ascii_string(2,2+8*(chipSel>=chipBM),strTemp);

    //rssi特别小的时候，认为没有射频了，肯定也就没有导频了。
    //（为了解决有时手柄关掉的时候没有先检测到pilot先关闭）
    if(KT_WirelessMicRx_CheckPilot()) 
    {
        display_ascii_string(1,3+8*(chipSel>=chipBM),"p");    
    }
    else
    {
        display_ascii_string(1,3+8*(chipSel>=chipBM)," ");    
    }

//    cRssi = KT_Bus_Read(0x0221,0);
//    sprintf(strTemp,"%03bd",cRssi);
//    display_ascii_string(1,4,strTemp);
//
//    cRssi = KT_Bus_Read(0x021c,0);      //0x021c 显示天线分集音频的输出
//    sprintf(strTemp,"%#04bx",cRssi);
//    display_ascii_string(1,12,strTemp);

    return(1);
}

//-----------------------------------------------------------------------------
//函 数 名：Save_Freq
//功能描述：将频率保存在MCU的FLASH里
//函数说明：
//全局变量：Memery_Frequency[i]
//输    入：
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void Save_Freq (void)
{
    UINT8  i;
    UINT8 * pTemp=(UINT8 *)&soundA; //=Memery_Frequency;
    UINT8 xdata * data pwrite; // 程序存储器空间的指针（FLASH），指向待写地址

	EA=0;
//erase
    VDM0CN = 0x80; // enable VDD monitor
    RSTSRC = 0x02; // enable VDD monitor as a reset source
    pwrite = saveAddress;
    FLKEY = 0xA5; // Key Sequence 1
    FLKEY = 0xF1; // Key Sequence 2
    PSCTL |= 0x03; // PSWE = 1; PSEE = 1
    VDM0CN = 0x80; // enable VDD monitor
    RSTSRC = 0x02; // enable VDD monitor as a reset source
    *pwrite = 0; // initiate page erase
    PSCTL &= ~0x03;    // PSWE = 0; PSEE = 0
//erase end

//write
    PSCTL |= 0x01; // PSWE = 1
    VDM0CN = 0x80; // enable VDD monitor
    RSTSRC = 0x02; // enable VDD monitor as a reset source
    pwrite = saveAddress;
    for(i=0;i<sizeof(soundEffect);i++)
    {
        FLKEY = 0xA5; // Key Sequence 1
        FLKEY = 0xF1; // Key Sequence 2
        *pwrite= *pTemp; // write the byte
        pwrite++;
        pTemp++;
    }

#ifdef TWOCHANNEL
    pTemp=(UINT8 *)&soundB;
    for(i=0;i<sizeof(soundEffect);i++)
    {
        FLKEY = 0xA5; // Key Sequence 1
        FLKEY = 0xF1; // Key Sequence 2
        *pwrite= *pTemp; // write the byte
        pwrite++;
        pTemp++;                                    
    }
#endif

    pwrite = saveAddress+0x00A0;
    FLKEY = 0xA5; // Key Sequence 1
    FLKEY = 0xF1; // Key Sequence 2
    *pwrite=0x88;

	if(StateMachineA.MatchSave)
	{
		pwrite = saveAddress+0x00B0;
	    FLKEY = 0xA5; // Key Sequence 1
	    FLKEY = 0xF1; // Key Sequence 2
	    *pwrite=0x01;
		pwrite++;
		FLKEY = 0xA5; // Key Sequence 1
	    FLKEY = 0xF1; // Key Sequence 2
		*pwrite=(StateMachineA.privateCode>>8);
		pwrite++;
		FLKEY = 0xA5; // Key Sequence 1
	    FLKEY = 0xF1; // Key Sequence 2
		*pwrite=(StateMachineA.privateCode&0x00ff);
		pwrite++;
	}
#ifdef TWOCHANNEL
	if(StateMachineB.MatchSave)
	{
		pwrite = saveAddress+0x00C0;
	    FLKEY = 0xA5; // Key Sequence 1
	    FLKEY = 0xF1; // Key Sequence 2
	    *pwrite=0x01;
		pwrite++;
		FLKEY = 0xA5; // Key Sequence 1
	    FLKEY = 0xF1; // Key Sequence 2
		*pwrite=(StateMachineB.privateCode>>8);
		pwrite++;
		FLKEY = 0xA5; // Key Sequence 1
	    FLKEY = 0xF1; // Key Sequence 2
		*pwrite=(StateMachineB.privateCode&0x00ff);
		pwrite++;
	}
#endif
    PSCTL &= ~0x01; // PSWE = 0
//write end
}

//-----------------------------------------------------------------------------
//函 数 名：setSound
//功能描述：频道及音效配置
//函数说明：
//全局变量：
//输    入：UINT8 channel
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void setSound(UINT8 channel)
{
    UINT8 i,j;
    
    pSoundEffect pSoundTemp;
    if(channel==channelA)
    {
        pSoundTemp=&soundA;
        chipSel=chipAM;
    }
#ifdef TWOCHANNEL
    else
    {
        pSoundTemp=&soundB;
        chipSel=chipBM;
    }
#endif
    for(j=0;j<2;j++)
    {
        KT_WirelessMicRx_SW_Echo(pSoundTemp->echoFlag);
        KT_WirelessMicRx_SW_Equalizer(pSoundTemp->equalierFlag);
        KT_WirelessMicRx_SW_Exciter(pSoundTemp->exciterFlag);
        KT_WirelessMicRx_SetEcho(pSoundTemp->echoDepth,pSoundTemp->echoDelay);
        KT_WirelessMicRx_SetExciter(pSoundTemp->exciterOdd,pSoundTemp->exciterEven);
        for(i=0;i<15;i++)
        {
            KT_WirelessMicRx_SetEqualizer(i,(pSoundTemp->equalier16Sel)[i]);
        }
    #ifndef DIVERSITY
        return;
    #endif
        chipSel++;
    }    
}

void loadChannelInfo(void)
{
    UINT8 code *pread= saveAddressSpec; // 程序存储器空间的指针（FLASH），指向待写地址
    UINT8 i,*ptemp=&soundA;
    
    if(*pread==0x88)
    {
        pread= saveAddress; // 程序存储器空间的指针（FLASH），指向待写地址
        for(i=0;i<sizeof(soundEffect);i++)
        {
            *ptemp++ = *pread++;
        }
    #ifdef TWOCHANNEL
        ptemp=(UINT8*)&soundB;
        for(i=0;i<sizeof(soundEffect);i++)
        {
            *ptemp++ = *pread++;
        }
    #endif
		StateMachineA.MatchSave=*((UINT8 code*)(saveAddress+0xB0));
		if(StateMachineA.MatchSave!=1)
			StateMachineA.MatchSave=0;
		if(StateMachineA.MatchSave==0x01)
		{
			StateMachineA.privateCode= *((UINT8 code*)saveAddress+0xB1);
			StateMachineA.privateCode<<=8;
			StateMachineA.privateCode|= *((UINT8 code*)saveAddress+0xB2);
		}
	#ifdef TWOCHANNEL
		StateMachineB.MatchSave=*((UINT8 code*)(saveAddress+0xC0));
		if(StateMachineB.MatchSave!=1)
			StateMachineB.MatchSave=0;
		if(StateMachineB.MatchSave==0x01)
		{
			StateMachineB.privateCode= *((UINT8 code*)saveAddress+0xC1);
			StateMachineB.privateCode<<=8;
			StateMachineB.privateCode|= *((UINT8 code*)saveAddress+0xC2);
		}
	#endif
    }    
    setSound(channelA);
#ifdef TWOCHANNEL
    setSound(channelB);
#endif
}

//-----------------------------------------------------------------------------
//函 数 名：LCD_Refresh
//功能描述：刷新LCD屏幕显示
//函数说明：
//全局变量：CURRENT_VOL; bChannel;
//输    入：
//返    回：无
//设 计 者：zhoudongfeng            时间：2012-12-18
//修 改 者：                        时间：
//版    本：V1.2
//-----------------------------------------------------------------------------
void LCD_Refresh (void)
{
    UINT8 i;
	UINT16 refreshTimeInit;
	#ifdef AUTO_SEARCH_FREQ
	#ifdef TWOCHANNEL
	if(((StateMachineA.seek_mute==0)&&(StateMachineB.seek_mute==0))||(currentStatus!=normal))
	#else
	if(StateMachineA.seek_mute==0)
	#endif
	{
		refreshTimeInit=500;
	}
	else
	{
		refreshTimeInit=5000;
	}
	#else
		refreshTimeInit=500;
	#endif
	   
    if(refreshTimer>=refreshTimeInit)
    {
        refreshTimer=0;
		rfIntCtl();
        if(currentStatus==normal)
        {    
            for(i=0;i<chipNumb;)
            {  
                chipSel = i; 
				KT_WirelessMicRx_SelectMS(); 
				MSDisplay();        
                Vol_Display();        
                voltageDisplay();
                xtalDisp();
                RSSI_Display(); //显示RSSI及pilot标志
                SNR_Display();
//				sprintf(strTemp,"%04x",StateMachineA.privateCode);
//        		display_ascii_string(1,4,strTemp);
//				sprintf(strTemp,"%04x",StateMachineB.privateCode);
//        		display_ascii_string(1,12,strTemp);
				#ifndef debug
//				BATTERY_Display();
				#endif
				#ifdef debug
				DisplayRegx(0x0219,1,4);
				DisplayRegx(0x021a,1,6);
//				DisplayRegx(0x0064,3,6);
				#endif
            #ifndef DIVERSITY
                i++;
            #else
                i+=2;
            #endif
            }
        }
        if(currentStatus==selectSet)
        {
            freshMenu();
        } 
        if(currentStatus==setEcho)
        {
            freshSetEcho();
        }
        if(currentStatus==setEqualizer)
        {
            freshSetEqualizer();
        }
        if(currentStatus==setExciter)
        {
            freshSetExciter();
        }
//        regx=KT_Bus_Read(0x007F,chipSel);//当power_on finish又为1时，说明芯片重启了，需要初始化
//        if(regx&0x01)
//        {
//            DEMO_Init();
//        }
    }
}

void chipPowerOn(void)
{
	CHIP_EN_AM  = 0;
	CHIP_EN_AS  = 0;
	CHIP_EN_BM  = 0;
	CHIP_EN_BS  = 0;
	Delay_ms(10);
	Delay_us(250);
	CHIP_EN_AM  = 1;
	CHIP_EN_AS  = 1;
	CHIP_EN_BM  = 1;
	CHIP_EN_BS  = 1;
	Delay_ms(15);
	for(chipSel=0;chipSel<chipNumb;chipSel++)
    {
        while(!KT_WirelessMicRx_PreInit()); //打开debug选项，使其从外部eeprom load程序
    }
	Delay_ms(200);
	loadChannelInfo();
    for(chipSel=0;chipSel<chipNumb;chipSel++)
    {
        KT_MicRX_Init(); //芯片初始化
		#ifdef TWOCHANNEL
        if(chipSel>=chipBM)	//B 通道时
        {
            KT_WirelessMicRx_SW_Diversity(soundB.diversityFlag);
        }
        else
		#endif
        {
            KT_WirelessMicRx_SW_Diversity(soundA.diversityFlag);
        }
    }
}

//-----------------------------------------------------------------------------
//函 数 名：DEMO_Init
//功能描述：整个DEMO初始化
//函数说明：
//全局变量：bChannel;CURRENT_VOL;
//输    入：
//返    回：无
//设 计 者：zhoudongfeng            时间：2012-12-18
//修 改 者：                        时间：
//版    本：V1.2
//-----------------------------------------------------------------------------
void DEMO_Init (void)
{
    UINT8 i;
    
    Mute_A = 1;
    Mute_B = 1;
    for(i=0;i<chipNumb;i++)
    {
        Flag_AUTOMUTE[i]=1;
    }
    LCD_Init();
	chipPowerOn();	 //控制chip_en上电
}

//-----------------------------------------------------------------------------
//函 数 名：KT_MicRX_Automute_Pilot
//功能描述：导频检测作为Automute的条件的开关程序
//函数说明：
//全局变量：
//输    入：
//返    回：无
//设 计 者：Kang hekai              时间：2013-04-24
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void KT_MicRX_Automute_Pilot_SW(UINT8 pilotSw)
{
    UINT8 regx;
    
    pilotSw &= 0x01;
    regx=KT_Bus_Read(0x0100,chipSel);
    if(pilotSw)
    {
        KT_Bus_Write(0x0100,(regx|0x80),chipSel);
    }
    else
    {
        KT_Bus_Write(0x0100,(regx&0x7f),chipSel);
    }
}

//-----------------------------------------------------------------------------
//函 数 名：displayChanFreq
//功能描述：displayChanFreq
//函数说明：
//全局变量：无
//输    入：UINT8 channel
//返    回：无
//设 计 者：KANG Hekai                时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void displayChanFreq(UINT8 channel)
{
     if(channel==channelA)
    {
        pChangeSound=&soundA;
    }
#ifdef TWOCHANNEL
    else
    {
        pChangeSound=&soundB;
    }
#endif
    sprintf(strTemp,"%ld",pChangeSound->Memery_Frequency);
    display_ascii_string(0,2+8*channel,strTemp);
}

//-----------------------------------------------------------------------------
//函 数 名：displayFreq
//功能描述：displayFreq
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void displayFreq(void)
{
     displayChanFreq(channelA);
#ifdef TWOCHANNEL
    displayChanFreq(channelB);
#endif
}

//-----------------------------------------------------------------------------
//函 数 名：voltageDisplay
//功能描述：voltageDisplay
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void voltageDisplay(void)
{
    UINT8 i;
	KT_WirelessMicRx_CheckAUXCH();
	if(MorSSelect)
	{
		i=0;
	}
	else
	{
		i=1;
	}
    if(Flag_PKGSYNC[chipSel+i] == PKGSYNC) //显示电池电量
    {
        KT_MicRX_Batter_Detecter();
    }
    else
    {
        display_ascii_string(1,2+8*(chipSel>=chipBM),"E");
    }
}

//-----------------------------------------------------------------------------
//函 数 名：xtalDisp
//功能描述：xtalDisp
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void xtalDisp(void)
{
    UINT8 regx;
    
    regx = KT_Bus_Read(0x0012,chipSel);
    if(regx&0x80)
    {
        display_ascii_string(3,7+8*(chipSel>=chipBM),"B");
    }
    else
    {
        display_ascii_string(3,7+8*(chipSel>=chipBM),"A");    
    }
}

//-----------------------------------------------------------------------------
//函 数 名：SNR_Display
//功能描述：SNR_Display
//函数说明：
//全局变量：无
//输    入：无
//返    回：正确：1                 错误：0    
//设 计 者：KANG Hekai              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 SNR_Display(void)
{
    UINT8 snr;
    
    snr=KT_WirelessMicRx_GetSNR();
    sprintf(strTemp,"%02bx",snr);
    display_ascii_string(2,6+8*(chipSel>=chipBM),strTemp);
    return(1);
}

//-----------------------------------------------------------------------------
//函 数 名：BATTERY_DISPLAY
//功能描述：BATTERY_DISPLAY
//函数说明：用来显示接收机GPIO4引脚的电压
//全局变量：无
//输    入：无
//返    回：无   
//设 计 者：wu jinfeng              时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void BATTERY_Display(void)
{
    UINT16 battery;   
    battery=KT_WirelessMicRx_BatteryMeter_Read();
    sprintf(strTemp,"%4d",battery);
    display_ascii_string(1,4+8*(chipSel>=chipBM),strTemp);
}

//-----------------------------------------------------------------------------
//函 数 名：DisplayRegx
//功能描述：DisplayRegx
//函数说明：在屏幕上显示寄存器的值
//全局变量：无
//输    入：UINT16 regx,UINT8 x,UINT8 y
//返    回：无   
//设 计 者：wu jinfeng              时间：2017-03-23
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
#ifdef debug
UINT8 DisplayRegx(UINT16 regx,UINT8 x,UINT8 y)
{
	UINT8 mute;
	mute= KT_Bus_Read(regx,chipSel);
	sprintf(strTemp,"%02bx",mute);
	display_ascii_string(x,y+8*(chipSel>=chipBM),strTemp);
	return(1);
}
#endif

//UINT16 saveBpskA[8];
//UINT16 saveBpskB[8];
UINT32 saveValidFreq[2]={0,0};
void SeekFreqStateMachine(BOOL chan_num)
{
	UINT8 Bpsk_data_AH,Bpsk_data_AL;
	UINT16 Bpsk_data_B;
	static UINT8 vaildNumb[2]={0,0};

	UINT8 chipSelSave=chipSel;
	pSoundEffect pChangeSoundSave=pChangeSound;
	UINT8 currentChannelSave=currentChannel;

	if(chan_num == CH_A)
	{
		pChangeSound= &soundA;
		currentChannel=channelA;
		chipSel = chipAM;
		pState = &StateMachineA;
	}
	#ifdef TWOCHANNEL
	else if(chan_num == CH_B)
	{
		pChangeSound= &soundB;
		currentChannel=channelB;
		chipSel = chipBM;
		pState = &StateMachineB;
	}
	#endif
	if(pState->seek_mute == 1)
		pState->repeat_time = 3;
	else 
		pState->repeat_time = 50;

	switch (pState->seekState)
	{
		case READ_RSSI:		
			if(pState->delay_time > 5)
			{			
				pState->delay_time = 0;
				pState->cRssi = KT_WirelessMicRx_GetRSSI();
				if(pState->cRssi > 0x30)
				{
					rfIntCtlChip(chipSel);
					pState->seekState = READ_SNR;
					pState->confirm_cnt=0;
				}
				else 
				{
					if(++pState->confirm_cnt >= (pState->repeat_time-1))
					{
						pState->seekState = SEARCH_START;
					}
				}
			}
			break;
		case READ_SNR:		
			if(pState->delay_time > 10)
			{			
				pState->delay_time = 0;
				pState->cSnr = KT_WirelessMicRx_GetSNR();
				if(pState->cSnr > 0x58)
				{						
					pState->seekState = READ_PILOT;
					pState->confirm_cnt=0;
				}
				else 
				{
					if(++pState->confirm_cnt >= pState->repeat_time)
					{
						pState->seekState = SEARCH_START;
					}
				}
			}
			break;						
		case READ_PILOT:
			if(pState->delay_time >=40)
			{
				pState->delay_time = 0;
				if((KT_Bus_Read(0x0209,chipSel) & 0x80) == 0x80)
				{
					pState->seekState = READ_BPSK;
					pState->read_bpsk_cnt =0;
					pState->confirm_cnt=0;
					displayChanFreq(currentChannel);
				}									
				else
				{
					if(++pState->confirm_cnt >= pState->repeat_time)
					{					
						pState->seekState = SEARCH_START;
					}
				}						
			}
			break;				
		case READ_BPSK:
			if(pState->delay_time >= 100)
			{				
				pState->delay_time = 0;
				if(KT_Bus_Read(0x0209,chipSel) & 0x40)
				{					
					Bpsk_data_AH = KT_Bus_Read(0x0248,chipSel);
					Bpsk_data_AL = KT_Bus_Read(0x0249,chipSel);
					Bpsk_data_B = (KT_Bus_Read(0x024A,chipSel)<<8) | KT_Bus_Read(0x024B,chipSel);
//					saveBpskA[pState->read_bpsk_cnt]=(Bpsk_data_AH<<8)|Bpsk_data_AL;
//					saveBpskB[pState->read_bpsk_cnt]=(KT_Bus_Read(0x024A,chipSel)<<8) | KT_Bus_Read(0x024B,chipSel);
					pState->CurrentChanNum = CheckChannelNum();
					if((Bpsk_data_AH == VENDOR_ID) && (Bpsk_data_AL == pState->CurrentChanNum)&& ((Bpsk_data_B == pState->privateCode) || (!pState->MatchSave)))
					{
						pState->seek_mute = 0;
						KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
						KT_WirelessMicRx_Volume(PRESET_VOL);
						if(chan_num == CH_A)
						{
							Mute_A=0;
						}
						else
						{
							Mute_B=0;
						}
						#ifdef DIVERSITY 
						    chipSel++;
						    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
							KT_WirelessMicRx_Volume(PRESET_VOL);
						    chipSel--;
						#endif				
						saveValidFreq[chan_num]=pChangeSound->Memery_Frequency;
						pState->read_bpsk_cnt =0;
						pState->seekState = validTuneHold;
					}
					else
					{
						if(++pState->read_bpsk_cnt >= 10)
						{								
							display_ascii_string(0,2+8*currentChannel,"      ");
							pState->seekState = SEARCH_START;
						}
					}							
					break;	
				}
				else
				{
					if(++pState->read_bpsk_cnt >= 10)
					{								
						pState->seekState = SEARCH_START;
						display_ascii_string(0,2+8*currentChannel,"      ");
					}
				}	
			}
			break;					
		case validTuneHold:
			if(pState->delay_time >= 10)
			{
				pState->delay_time = 0;
				vaildNumb[chan_num]++;
				if(vaildNumb[chan_num]>=10)
				{
					vaildNumb[chan_num]=0;
					if(chan_num == CH_A)
					{
						KT_WirelessMicRx_PatchChip(chipAM);
						#ifdef DIVERSITY
						KT_WirelessMicRx_PatchChip(chipAS);
						chipSel = chipAM;
						#endif
					}
					#ifdef TWOCHANNEL 
					else
					{
						KT_WirelessMicRx_PatchChip(chipBM);
						#ifdef DIVERSITY
						KT_WirelessMicRx_PatchChip(chipBS);
						chipSel = chipBM;
						#endif
					}
					#endif
					if((KT_Bus_Read(0x0209,chipSel)&0x40)==0)
					{
						pState->confirm2_cnt++;
					}
					else
					{
						if(pState->CurrentChanNum == KT_Bus_Read(0x0249,chipSel))
						{
							if(pState->MatchSave)
							{
								Bpsk_data_B = (KT_Bus_Read(0x024A,chipSel)<<8) | KT_Bus_Read(0x024B,chipSel);
								if(Bpsk_data_B == pState->privateCode)
								{
									pState->confirm2_cnt=0;
								}
								else
								{
									pState->confirm2_cnt++;
								}
							}
							else
							{
								pState->confirm2_cnt=0;
							}
						}
						else
						{
							pState->confirm2_cnt++;	
						}
					}
				}
				KT_WirelessMicRx_SelectMS();
				pState->cRssi = KT_WirelessMicRx_GetRSSI();
				pState->cSnr = KT_WirelessMicRx_GetSNR();
//				if((pState->cRssi < 0x20)&&(pState->cSnr < 0x30))
//				{
//					pState->seekState = SEARCH_START;
//					break;
//				}
				if((pState->cRssi > 0x22)&&(pState->cSnr > 0x60))
				{					
					pState->confirm_cnt=0;
				}
				else 
				{
					pState->confirm_cnt++;
				}
				if((pState->confirm_cnt >= 40)||(pState->confirm2_cnt>=4))
				{
					pState->confirm_cnt=0;
					pState->confirm2_cnt=0;
					pChangeSound->Memery_Frequency  -= 3*BAND_STEP;
					pState->seekState = SEARCH_START;
				}
//				if(pState->confirm_cnt >= 20)
//				{
//					pState->seekState = SEARCH_START;
//				}
			}
			break;
		case SEARCH_START:
				if(pState->seek_mute==0)
				{
					pState->seek_mute = 1;
					display_ascii_string(0,2+8*currentChannel,"      ");
					pState->match_delay_cnt = 0;				
					KT_WirelessMicRx_Volume(0);
					#ifdef DIVERSITY 
					    chipSel++;
					    KT_WirelessMicRx_Volume(0);
					    chipSel--;
					#endif
					if(chan_num == CH_A)
					{
						Mute_A=1;
					}
					else
					{
						Mute_B=1;
					}
				}
				MorSSelect=1;
				pState->seekState = SEARCH_NEXT_FREQ;
				break;

		case SEARCH_NEXT_FREQ:
			if(++pState->match_delay_cnt>=25)
			{
				pState->match_delay_cnt = 0;   //tune 25个台后看一下700MHz
				
				Seek_Freq_FastTune(737750);	 //737750
				pState->delay_time = 0;
				pState->seekState = SEEK_MATCH_FREQ;
				break;
			}
			if(((pState->match_delay_cnt%5)==0)&&(saveValidFreq[chan_num]!=0))
			{
				Seek_Freq_FastTune(saveValidFreq[chan_num]);
				pState->seekState = WAIT_VALID_PLL_DONE;
				break;
			}				
			Seek_NextFreq_FastTune(0,1);
			pState->delay_time = 0;
			pState->seekState = WAIT_PLL_DONE;
			break;
		case SEEK_MATCH_FREQ:			
			if(pState->delay_time > 20)
			{
				pState->delay_time = 0;
				pState->cRssi = KT_WirelessMicRx_GetFastRSSI();
				if(pState->cRssi > 160)
				{					
//					rfIntCtl();
					rfIntCtlChip(chipSel);
					pState->seekState = SET_MATCH;
				}
				else
				{
					pState->seekState = SEARCH_NEXT_FREQ;
				}
			}
			break;

		case SET_MATCH:				
			if(pState->delay_time <= 2000)
			{
				if(KT_Bus_Read(0x0209,chipSel) & 0x40)
				{
					Bpsk_data_AH = KT_Bus_Read(0x0248,chipSel);
					Bpsk_data_AL = KT_Bus_Read(0x0249,chipSel);
					Bpsk_data_B = (KT_Bus_Read(0x024A,chipSel)<<8) | KT_Bus_Read(0x024B,chipSel);
					if((Bpsk_data_AH == VENDOR_ID) && (Bpsk_data_AL == 100))
					{
						if(Bpsk_data_B!=0)
						{
							pState->privateCode = Bpsk_data_B;
							pState->MatchSave = 1;
							pState->seekState = SEARCH_NEXT_FREQ;
						}
					}								
				}
			}
			else
			{
				pState->seekState = SEARCH_NEXT_FREQ;
			}
			break;
				
		case WAIT_PLL_DONE:
			if(KT_Bus_Read(0x0061,chipSel) & 0x01)
			{
				pState->seekState = READ_RSSI; 
				pState->delay_time = 5;	 //让其直接读RSSI
				pState->confirm_cnt=0;
			}
			break;
		case WAIT_VALID_PLL_DONE:
			if(KT_Bus_Read(0x0061,chipSel) & 0x01)
			{
				KT_WirelessMicRx_SelectMS();
				pState->cRssi = KT_WirelessMicRx_GetRSSI();
				pState->cSnr = KT_WirelessMicRx_GetSNR();
				if((pState->cRssi > 0x22)&&(pState->cSnr > 0x60))
				{
					pState->seek_mute = 0;
					pChangeSound->Memery_Frequency=	saveValidFreq[chan_num];
					KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
					KT_WirelessMicRx_Volume(PRESET_VOL);
					if(chan_num == CH_A)
					{
						Mute_A=0;
					}
					else
					{
						Mute_B=0;
					}
					displayChanFreq(currentChannel);
					#ifdef DIVERSITY 
					    chipSel++;
					    KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);
						KT_WirelessMicRx_Volume(PRESET_VOL);
					    chipSel--;
					#endif				
					pState->read_bpsk_cnt =0;
					pState->confirm2_cnt=0;
					pState->confirm_cnt=0;
					pState->CurrentChanNum = CheckChannelNum();
					pState->seekState = validTuneHold;
				}
				else
				{
					pState->seekState = SEARCH_START;
				}
				pState->delay_time = 0;
				pState->confirm_cnt=0;
			}
			break;	
		default:
			while(1);				
	}
	chipSel=chipSelSave;
	pChangeSound=pChangeSoundSave;
	currentChannel=currentChannelSave;
}

UINT8 CheckChannelNum()
{
	UINT8 chan_num;
    chan_num = (pChangeSound->Memery_Frequency-BAND_BOTTOM_CHA)/BAND_STEP;
	return(chan_num);
}

void Seek_NextFreq_FastTune(BOOL stepNum,BOOL fast_flag)
{
	UINT16 bandStep;
	if(stepNum)
	bandStep = BAND_STEP*2;
	else
	 bandStep = BAND_STEP;
	if(chipSel == chipAM)
	{
		pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  + bandStep;
	    if((pChangeSound->Memery_Frequency>BAND_TOP_CHA)||(pChangeSound->Memery_Frequency<BAND_BOTTOM_CHA))
			pChangeSound->Memery_Frequency  = BAND_BOTTOM_CHA;
	}
    else if(chipSel == chipBM)
	{
		pChangeSound->Memery_Frequency  = pChangeSound->Memery_Frequency  + bandStep;
	    if( (pChangeSound->Memery_Frequency  > BAND_TOP_CHB)||(pChangeSound->Memery_Frequency<BAND_BOTTOM_CHB))
			pChangeSound->Memery_Frequency  = BAND_BOTTOM_CHB;
	}
    if(fast_flag)
    Seek_Freq_FastTune(pChangeSound->Memery_Frequency);
	else
	KT_WirelessMicRx_Tune(pChangeSound->Memery_Frequency);	
}


//-----------------------------------------------------------------------------
// Timer0_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function configures the Timer0 as a 16-bit timer, interrupt enabled.
// Using the internal osc. at 12MHz with a prescaler of 1:8 and reloading the
// T0 registers with TIMER0_RELOAD_HIGH/LOW it will interrupt and then toggle
// the LED.
//
// Note: The Timer0 uses a 1:48 prescaler.  If this setting changes, the
// TIMER_PRESCALER constant must also be changed.
// 现在时钟用的是12M，timer0的时钟是系统时钟的1/48，timer0的时钟为250KHz
// 现在#define TIMER0_RELOAD_HIGH (65536-250)/256  #define	TIMER0_RELOAD_LOW	(65536-250)%256，
// 故现在的timer0的定时为1ms。
//-----------------------------------------------------------------------------
void Timer0_Init(void)
{
	TH0 = TIMER0_RELOAD_HIGH;           // Init Timer0 High register
	TL0 = TIMER0_RELOAD_LOW ;           // Init Timer0 Low register
	TMOD = 0x01;                        // Timer0 in 16-bit mode
	CKCON = 0x02;                       // Timer0 uses a 1:48 prescaler
	//   CKCON = 0x04;                       // Timer0 uses system clock
	ET0 = 1;                            // Timer0 interrupt enabled
	TCON = 0x10;                        // Timer0 ON
	EA = 1;        // Enable global interrupts
}
//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Timer0_ISR
//-----------------------------------------------------------------------------
//
// Here we process the Timer0 interrupt and toggle the LED
//
//-----------------------------------------------------------------------------
void Timer0_ISR (void) interrupt 1
{
	TH0 = TIMER0_RELOAD_HIGH;           // Reinit Timer0 High register
	TL0 = TIMER0_RELOAD_LOW;            // Reinit Timer0 Low register
	StateMachineA.delay_time ++;
	#ifdef TWOCHANNEL
	StateMachineB.delay_time ++;
	#endif
	refreshTimer++;		//用来刷新屏幕显示
	timerCounter++;
}