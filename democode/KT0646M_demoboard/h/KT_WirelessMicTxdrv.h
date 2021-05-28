//*****************************************************************************
//  File Name:    KT_WirelessMicTxdrv.h
//  Function:    KT Wireless Mic Transmitter Products Driver For Customer
//                (KT064xM)
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V0.1    2016-04-26  For KT0646M_VX
//  V0.2    2016-06-01  增加VX和VX2的编译选项
//  V0.3    2016-08-18  For KT0646M_VX4
//  V0.3.1  2016-08-24  删除KT_WirelessMicTx_Init程序内一些重复配置
//  V0.4    2016-08-26  修正了XTAL_DUAL问题
//  V0.5    2016-10-11  修改了初始化函数中的一些配置，把0x25,0x26寄存器的配置加到了初始化函数里面，
//                  在初始化函数里面set pilot frequency
//  V0.5.1  2016-10-17  重新修改了一下里面的宏定义，定义了RXISKT0616M就用BPSK的旧模式，然后再根据
//                  是否是双晶振把KT0616M选晶振的程序加进去。
//  V0.5.2  2016-11-15  在tune台前把lo_fine_vref_sel改为3，tune完后改成0.
//  V0.5.3  2016-12-15  在初始化中把lo_fine_vref_sel改为3，不用在tune台的时候再来回修改了.为了温度
//                  变化不锁定的问题，详见被宏DOUBLE_KVCO包围的代码
//  V0.6    2017-02-08  格式规范化整理
//  V1.1    2017-04-27  删除了一些初始化函数里面没用的东西，把有些用小写字母定义的宏定义改成了大写的字母
//  V1.2    2017-05-24  HARD_LIMIT从14改成了15，COMPANDOR_TC改成了3，即48ms。
//  V1.3    2017-09-18	COMPEN_GAIN由3改成1,MIC_SENS_GAIN由9改成5,COMPANDOR_TC_48ms改成COMPANDOR_TC_12ms
//  V1.4    2017-10-10  echo关闭的时候，不真正关echo而是把Echo_Ratio写成0,soft_rst不是在寄存器0x3e的bit15了，而是在0x1e的bit4.
//*****************************************************************************

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "interface.h"

//-----------------------------------------------------------------------------
//功能及参数定义
//-----------------------------------------------------------------------------
#define KT0646M

#define DOUBLE_KVCO
//#define pll_unlock
#define AUX_CH
#define SILENCE_MUTE

#define XTAL_DUAL
//#define XTAL_24M_ONLY
//#define XTAL_24P576M_ONLY

//#define RXISKT0616M

#ifdef     RXISKT0616M
    #define RXISKT0616M_BPSK
    #ifdef XTAL_DUAL
        #define RXISKT0616M_XTAL_DUAL
    #endif
#endif

#define KT_RX //使用KT芯片的接收机
//#define OTHER_RX //使用其他方案的接收机

#define KTWirelessMicTxw_address 0x6A
#define KTWirelessMicTxr_address 0x6B


#define XTAL_24M_FREQ 0
#define XTAL_24P576M_FREQ 1



#define INIT_FAIL_TH 3

#ifdef OTHER_RX
    #define PILOT_FREQ     32768
#endif

#define    BAND_TOP        800000
#define    BAND_BOTTOM     600000
#define    BAND_STEP       250

#define PA_SEL 1

#define PA_OFF 0
#define PA_ON  1

#define    AUDIO_UNMUTE    0
#define    AUDIO_MUTE      1

#define    WAKEUP    0
#define    STANDBY   1

#define    FDEV_MONITOR_TC_250ms    0
#define    FDEV_MONITOR_TC_500ms    1
#define    FDEV_MONITOR_TC_1s       2
#define    FDEV_MONITOR_TC_2s       3

#define    PRE_EMPHASIS_ENABLE      0
#define    PRE_EMPHASIS_DISABLE     1

#define    MIC_SENS_GAIN_0     0
#define    MIC_SENS_GAIN_1     1
#define    MIC_SENS_GAIN_2     2
#define    MIC_SENS_GAIN_3     3
#define    MIC_SENS_GAIN_4     4
#define    MIC_SENS_GAIN_5     5
#define    MIC_SENS_GAIN_6     6
#define    MIC_SENS_GAIN_7     7
#define    MIC_SENS_GAIN_8     8
#define    MIC_SENS_GAIN_9     9
#define    MIC_SENS_GAIN_10    10
#define    MIC_SENS_GAIN_11    11
#define    MIC_SENS_GAIN_12    12
#define    MIC_SENS_GAIN_13    13
#define    MIC_SENS_GAIN_14    14
#define    MIC_SENS_GAIN_15    15

#define    COMPANDOR_ENABLE    0
#define    COMPANDOR_DISABLE   1

#define    COMPANDOR_TC_6ms    0
#define    COMPANDOR_TC_12ms   1
#define    COMPANDOR_TC_24ms   2
#define    COMPANDOR_TC_48ms   3
#define    COMPANDOR_TC_93ms   4
#define    COMPANDOR_TC_199ms  5
#define    COMPANDOR_TC_398ms  6
#define    COMPANDOR_TC_796ms  7

#ifdef SILENCE_MUTE
	#define SLNC_MUTE_DIS          1
	#define SLNC_MUTE_TIME         0x13
	#define SLNC_MUTE_LOW_LEVEL    4
	#define SLNC_MUTE_HIGH_LEVEL   6
	
	#define SILENCE_MUTE_ACT_MCU   0
#endif

#ifdef KT_RX //使用KT芯片的接收机
    #define    PILOT_ENABLE    1
    #define    PILOT_DISABLE   0
#endif

#ifdef OTHER_RX //使用其他方案的接收机
    #define    PILOT_ENABLE    0
    #define    PILOT_DISABLE   1
#endif
#define    PILOT_FDEV_2P5K     0
#define    PILOT_FDEV_5K       1
#define    PILOT_FDEV_7P5K     2
#define    PILOT_FDEV_10K      3

#define    BATTERY_METER_DISABLE    0
#define    BATTERY_METER_ENABLE     1
//ZDF
#define    AGC_VHIGH                3        //AGC上门限：0-15可选
#define    AGC_VLOW                 4        //AGC下门限：0-15可选
#define    AGC_ATTACK               3        //AGC ATTACK时间：0-7可选
#define    AGC_RELEASE              4        //AGC RELEASE时间：0-7可选
#define    AGC_DIS                  1//0        //AGC控制：0-自动控制；1-MCU控制

#define    GAIN_SEL                 1        //PGA增益：0:（-6dB）,1:0dB,2:6dB,3:12dB
#define    COMPEN_GAIN              1//3     //总体增益：0-0dB,1-6dB,2-12dB,3-18dB
#define    BLANK_EN                 1        //BLANK控制：0-DIS；1-EN
#define    BLANK_TIME               3        //BLANK时间：0-7可选

#define    ECHO_MCU                 1        //ECHO控制：0-旋钮控制；1-MCU控制
#define    ECHO_ENABLE              0
#define    ECHO_DISABLE             1
#define    ECHO_STRU                1        //回声大小：0-回声大；1-回声小
#define    ECHO_GAINUP              7        //输出信号进行缩放：7-13.1dB
#define    ECHO_GAINDOWN            0        //输入信号进行缩放：0-（-13dB）

//反馈比例：0-无反馈；25-反馈25/32
#define    ECHO_RATIO_0            0        
#define    ECHO_RATIO_1            1
#define    ECHO_RATIO_2            2
#define    ECHO_RATIO_3            3
#define    ECHO_RATIO_4            4
#define    ECHO_RATIO_5            5
#define    ECHO_RATIO_6            6
#define    ECHO_RATIO_7            7
#define    ECHO_RATIO_8            8
#define    ECHO_RATIO_9            9
#define    ECHO_RATIO_10           10
#define    ECHO_RATIO_11           11
#define    ECHO_RATIO_12           12
#define    ECHO_RATIO_13           13
#define    ECHO_RATIO_14           14
#define    ECHO_RATIO_15           15
#define    ECHO_RATIO_16           16
#define    ECHO_RATIO_17           17
#define    ECHO_RATIO_18           18
#define    ECHO_RATIO_19           19
#define    ECHO_RATIO_20           20
#define    ECHO_RATIO_21           21
#define    ECHO_RATIO_22           22
#define    ECHO_RATIO_23           23
#define    ECHO_RATIO_24           24
#define    ECHO_RATIO_25           25

//信号延时：0-22ms；24-207ms
#define    ECHO_DELAY_22ms         0        
#define    ECHO_DELAY_24ms         1
#define    ECHO_DELAY_27ms         2
#define    ECHO_DELAY_29ms         3
#define    ECHO_DELAY_32ms         4
#define    ECHO_DELAY_35ms         5
#define    ECHO_DELAY_39ms         6
#define    ECHO_DELAY_43ms         7
#define    ECHO_DELAY_47ms         8
#define    ECHO_DELAY_52ms         9
#define    ECHO_DELAY_57ms         10
#define    ECHO_DELAY_63ms         11
#define    ECHO_DELAY_69ms         12
#define    ECHO_DELAY_76ms         13
#define    ECHO_DELAY_84ms         14
#define    ECHO_DELAY_92ms         15
#define    ECHO_DELAY_101ms        16
#define    ECHO_DELAY_111ms        17
#define    ECHO_DELAY_122ms        18
#define    ECHO_DELAY_135ms        19
#define    ECHO_DELAY_148ms        20
#define    ECHO_DELAY_163ms        21
#define    ECHO_DELAY_179ms        22
#define    ECHO_DELAY_197ms        23


#define    EQ_ENABLE               1
#define    EQ_DISABLE              0

//均衡器频率
#define    EQ_25H      0            
#define    EQ_40H      1
#define    EQ_63H      2

#define    EQ_100H     3
#define    EQ_160H     4
#define    EQ_250H     5

#define    EQ_400H     6
#define    EQ_630H     7
#define    EQ_1KH      8

#define    EQ_1K6      9
#define    EQ_2K5      10
#define    EQ_4KH      11

#define    EQ_6K3      12
#define    EQ_10K      13
#define    EQ_16K      14

//均衡器增益
#define    EQ_GAIN_Neg12dB           0            
#define    EQ_GAIN_Neg11dB           1
#define    EQ_GAIN_Neg10dB           2
#define    EQ_GAIN_Neg9dB            3
#define    EQ_GAIN_Neg8dB            4
#define    EQ_GAIN_Neg7dB            5
#define    EQ_GAIN_Neg6dB            6
#define    EQ_GAIN_Neg5dB            7
#define    EQ_GAIN_Neg4dB            8
#define    EQ_GAIN_Neg3dB            9
#define    EQ_GAIN_Neg2dB            10
#define    EQ_GAIN_Neg1dB            11
#define    EQ_GAIN_0dB               12
#define    EQ_GAIN_Pos1dB            13
#define    EQ_GAIN_Pos2dB            14
#define    EQ_GAIN_Pos3dB            15
#define    EQ_GAIN_Pos4dB            16
#define    EQ_GAIN_Pos5dB            17
#define    EQ_GAIN_Pos6dB            18
#define    EQ_GAIN_Pos7dB            19
#define    EQ_GAIN_Pos8dB            20
#define    EQ_GAIN_Pos9dB            21
#define    EQ_GAIN_Pos10dB           22
#define    EQ_GAIN_Pos11dB           23
#define    EQ_GAIN_Pos12dB           24

#define    HARD_LIMIT               15       //最大频偏限制：0-15可选
#define    CPRS_1XLPF_BP            1        //压缩滤波器控制：0-开启；1-关闭
#define    CPRS_KNEE_DIS            0        //噪声控制：0-开启；1-关闭
#define    CPRS_THRSH               8        //噪声门限：0-15可选 18uV-14mV

#define    ALC_DIS                 1//0      //ALC控制：0-开启；1-关闭
#define    ALC_SOFTKNEE            1         //ALC拐点类型：0-硬拐点；1-软拐点
#define    ALC_VMAX                68//4     //ALC门限：0-127可选
#define    ALC_ATTACK              3         //ALC ATTACK时间：0-7可选
#define    ALC_RELEASE             6         //ALC RELEASE时间：0-11可选

//电池电压检测
#define    BATTERY_MAX             0x7FF
#define    BATTERY_HIGHTH          0x500
#define    BATTERY_MIDDLETH        0x4C0
#define    BATTERY_LOWTH           0x4A0

#define    LOWVOLTAGE_TH           1400
//#define    LOWVOLTAGE_TH        1800

#define BPSK_NEW_MODE            1        //1:new mode  0:old mode
#define AUXDATA_EN               1        //BIT 14
#define AUX_REG_NUM              3        //BIT 13:12
#define AUX_CARRY_NUM            3        //BIT 11:9    00:12bit    01:16bit    10:18bit    11:20bit

#define AUX_ADDRB                0x17    //BIT 15:8
#define AUX_ADDRA                0x07    //BIT 7:0

#define AUX_ADDRD                0x01    //BIT 15:8
#define AUX_ADDRC                0x00    //BIT 7:0


//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
BOOL KT_WirelessMicTx_PreInit(void);
BOOL KT_WirelessMicTx_Init(void);

BOOL KT_WirelessMicTx_Standby(void);
BOOL KT_WirelessMicTx_WakeUp(void);
void KT_WirelessMicTx_PowerDownProcedure(void);

BOOL KT_WirelessMicTx_PASW(BOOL bPA_Switch);
BOOL KT_WirelessMicTx_PAGain(UINT8 cPaGain);

UINT8 KT_WirelessMicTx_Fdev_Monitor(void);
BOOL KT_WirelessMicTx_Mic_Sens(UINT8 cMicSens);
BOOL KT_WirelessMicTx_Comp_Dis(BOOL bComp_Dis);
BOOL KT_WirelessMicTx_Comp_TC(UINT8 cComp_TC);
BOOL KT_WirelessMicTx_MuteSel(BOOL bMute_Sel);

BOOL KT_WirelessMicTx_Pilot(BOOL bPilot_Dis);
BOOL KT_WirelessMicTx_Pilot_Fdev(UINT8 cPilot_Fdev);

#ifdef OTHER_RX
UINT8 KT_WirelessMicTx_Set_Pilot_Freq(BOOL bXtal_Sel);
#endif

BOOL KT_WirelessMicTx_Tune(INT32 Freq); //in KHz

UINT8 KT_WirelessMicTx_Band_Cali_Res(void);
UINT8 KT_WirelessMicTx_Set_XTAL(BOOL bXtal_Sel);
UINT8 KT_WirelessMicTx_SW_XTAL_Freq(BOOL bXtal_Sel);

void KT_Bus_Write(UINT8 Register_Address, UINT16 Word_Data);
UINT16 KT_Bus_Read(UINT8 Register_Address);

void KT_WirelessMicTx_ECHO(BOOL bEcho_Dis,UINT8 Echo_Ratio,UINT8 Echo_Delay);
void KT_WirelessMicTx_EQSW(BOOL bEq_Dis);
void KT_WirelessMicTx_EQGAIN(UINT8 Eq_Freq,UINT8 Eq_Gain);
UINT16 KT_WirelessMicTx_BatteryMeter_Read(void);
BOOL KT_WirelessMicTx_BatteryMeter_SW(BOOL bBatteryMeter_En);
BOOL KT_WirelessMicTx_Mic_Sens(UINT8 cMicSens);

