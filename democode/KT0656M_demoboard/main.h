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
#ifndef _MAIN_H_
#define    _MAIN_H_

//-----------------------------------------------------------------------------
// 功能及参数定义
//-----------------------------------------------------------------------------
#ifdef     TWOCHANNEL
    #define channelNum 2
#else 
    #define channelNum 1
#endif

#define channelA 0
#define channelB 1

#ifdef DIVERSITY
    #ifdef TWOCHANNEL
           #define chipNumb 4
        #define chipAM 0
        #define chipAS 1
        #define chipBM 2
        #define chipBS 3
       #else
         #define chipNumb 2
        #define chipAM 0
        #define chipAS 1

        //显示的时候根据display_ascii_string(1,2+8*(chipSel>=chipBM),"H");来决
        //定显示的位置，如果是一路的话，让chipBM为一个比较大的数，使得一直显示在左边
        #define chipBM 8
       #endif
#else
    #ifdef TWOCHANNEL
           #define chipNumb 2
        #define chipAM 0
        #define chipBM 1
       #else
         #define chipNumb 1
        #define chipAM 0

        //显示的时候根据display_ascii_string(1,2+8*(chipSel>=chipBM),"H");来决
        //定显示的位置，如果是一路的话，让chipBM为一个比较大的数，使得一直显示在左边
        #define chipBM 8 
       #endif
#endif

//设备地址定义
#define CS4398AddrA 0x98
#define    CS4398AddrB    0x9A

#define saveAddress   0x6A00 //把频点及音效等保存在flash里面的地址

//在此保存了数据0x88,每次上电的时候先读取此数据，看是否是0x88,如果是，再load 0x6A00处
//的数据，否则就不再load
#define saveAddressSpec   0x6A00+0x00A0        

//菜单状态定义
enum statusName{normal,selectSet,setEcho,setEqualizer,setExciter};

typedef struct
{
	UINT8 seekState;
	UINT16 delay_time;
	UINT8 repeat_time;
	UINT8 confirm_cnt;
	UINT8 confirm2_cnt;
	UINT16 privateCode;	 //私人码
	UINT8 MatchSave;	 //私人码保存标志位
	UINT8 seek_mute;	 //搜台时置1，搜完后置0
	INT8 CurrentChanNum;
	UINT8 read_bpsk_cnt;
	UINT8 cSnr;
	UINT8 cRssi;
	UINT8 match_delay_cnt;
}StateMachine,*pStateMachine;

#define IS_VALID_TUNE	0x01
#define READ_SNR	0x02
#define SEARCH_NEXT_FREQ	0x03
#define READ_RSSI	0x04
#define READ_PILOT	0x05
#define READ_BPSK	0x06
#define WAIT_PLL_DONE	0x07
#define GET_RSSI_500K	0x08
#define GET_RSSI_250K_L	0x09
#define GET_RSSI_250K_R	0x0A
#define SCAN_OVER  0x0B
#define SEARCH_PREVIOUS_FREQ 0x0C
#define SEARCH_START 0x0D
#define SEEK_MATCH_FREQ	0x0E
#define SET_MATCH	0x0F
#define validTuneHold 0x10
#define WAIT_VALID_PLL_DONE 0x11

#define VENDOR_ID	0xA5

#define TIMER0_RELOAD_HIGH (65536-250)/256
#define	TIMER0_RELOAD_LOW	(65536-250)%256

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
void SYSCLK_Init (void);
void PORT_Init (void);

void KT_MicRX_Init (void); // KT_MicRX初始化
void KT_MicRX_Next_Fre (void); // KT_MicRX加100KHz
void KT_MicRX_Previous_Fre (void); // KT_MicRX减100KHz

void KT_MicRX_PowerDown_Detecter (void);
void KT_MicRX_Batter_Detecter (void);
void LCD_Refresh (void);
void DEMO_Init (void);
void Save_Freq (void);
void Load_Freq (void);
void KT_FreqSearch(void);
void KT_FreqSearchB(void);
UINT8 isValidTune(void);
void saveFreqInfo(void);
void displayFreq(void);
void voltageDisplay(void);
void mutePointChange(void);
void antChange(void);
UINT8 SNR_Display(void);
void BATTERY_Display(void);
void setMenu(void);
void keyWork(UINT8 keyName);
void freshMenu(void);
void MenuAdd(void);
void MenuDec(void);
void MenuSelect(void);

void freshSetEqualizer(void);
void freshSetExciter(void);
void loadChannelInfo(void);
void toNormal(void);

void KT_MicRX_Automute_Pilot(void);
void KT_MicRX_Previous_searchFre (BOOL stepNum);
void KT_MicRX_Next_searchFre (BOOL stepNum);
void CS4398Init(UINT8 address);
void dacInit(void);
void KT_MicRX_Automute_Pilot_SW(UINT8 pilotSw);
void xtalDisp(void);
void batCodeDisp(void);
void displayMenu(void);
void MSDisplay(void);

UINT8 CheckChannelNum();
void Seek_Freq_FastTune(long Freq);
void Seek_NextFreq_FastTune(BOOL stepNum,BOOL fast_flag);
void Timer0_Init(void);
UINT8 MuteSW(void);
void StateMachineRefresh(BOOL chan_num);
void SeekFreqStateMachine(BOOL chan_num);

#endif

