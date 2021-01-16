///**********************************************************************
//  File Name: KT_WirelessMicRxdrv.h
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
//	V1.8	2013-04-24	Added BAND_TOP_CHA, BAND_BOTTOM_CHA, BAND_TOP_CHB and BAND_BOTTOM_CHB.
#define uchar unsigned char
#define ulong unsigned long
#define uint unsigned int

#define KTWirelessMicRxw_address 0x6A
#define KTWirelessMicRxr_address 0x6B

#define CHANNEL_NUMBER	2

//#define KT0615M
#define KT0616M

#define AUTOMUTE_EN			//undefine to disable automute
//#define INTERRUPT_EN		//undefine to disable interrupt output

#ifdef KT0616M
//	#define SQUEAL_EN
#endif

#ifdef SQUEAL_EN
//	#define SQUEAL_RST		//Reset SQUEAL FSM
#endif

#define XTAL_DUAL
//#define XTAL_24M_ONLY
//#define XTAL_24P576M_ONLY


#define XTAL_24M_FREQ 0
#define XTAL_24P576M_FREQ 1

#define INIT_FAIL_TH		3

#define PILOT_FREQ			32768

#define	BAND_TOP_CHA			700000
#define	BAND_BOTTOM_CHA			650000
#define	BAND_TOP_CHB			850000
#define	BAND_BOTTOM_CHB			800000
#define	BAND_STEP			250

#define	BATTERY_MAX				0x7FF
#define	BATTERY_HIGHTH			0x500
#define	BATTERY_MIDDLETH		0x4C0
#define	BATTERY_LOWTH			0x4A0

//REG18
#define	INT_EN				1
#define	INT_LVL				0
#define	SQUEAL_INT_EN		1
#define	AUTOMUTE_INT_EN		1
#define	PKGSYNC_INT_EN		1
#define	PILOT_INT_EN		1
#define BURSTDATA_INT_EN	1

#define	SQUEAL				1
#define	NON_SQUEAL			0
#define	AUTOMUTE			1
#define	NON_AUTOMUTE		0
#define	PKGSYNC				1
#define	NON_PKGSYNC			0
#define	PILOT				1
#define	NON_PILOT			0

//REG19
#define	AUTOMUTE_STATUS		1
#define	PKGSYNC_STATUS		1
#define	PILOT_STATUS		1

#define	SQUEAL_INT			1
#define	AUTOMUTE_INT		1
#define	PKGSYNC_INT			1
#define	PILOT_INT			1
#define BURSTDATA_INT		1

//REG1D
#define DEEMPHASIS_DIS		0	//0: ensable; 1: disable
#define	AU_GAIN				1	//1: 50K
#define AFC_EN 				1	//0: disable; 1: enable
#define AFC_RNG 			3	//0: +-20KHz; 1: +-40KHz; 2: +-60Khz; 3: +-90KHz;

//REG1F
#define EXP_DIS				0	//0: ensable; 1: disable
#define EXP_TC 				1	//0: fastest; 7: slowest
#define THD_POWER			0
#define	TREBLE_BOOST		0 
#define	BASS_BOOST			0 


//------------------------------------------------------------------------------------
//REG20
#define AUXDATA_EN			1	//1: ensable; 0: disable
#define AUX_FEDV			1	//1: 5KHz

//REG21
#define AUX_CARRY_NUM		3	//3: 20
#define AUX_CARRY_NUM_TH	4	//3: 16

//REG22
#define PKG_SYNC_NUM		2	//
#define BYTE_LOSTSYNC_NUM	2	//
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
//REG24
#define SQUEAL_DIS			0	// 0: ensable;	1: disable
#define FNOTCH_NOTRESP_TH	1//0	// 0: 1/4;		1: 1/2
#define N_OCTAVE			0//2//3	// 0: 7.21;		1: 14.42	2: 28.85	3:115.42
#define FFT_R_TH			8	// 0: 0;		15: 30
#define FRAME_NUM_TH		0//4	//
#define NOTCH_CTRL_BY_EXT	0	// 0:AUTOMODE	1:MANUAL

//REG2F
#define	PMAX_HITH			8
#define	PMAX_LOWTH			6
#define	FDIFF_HITH			15
#define	FDIFF_LOWTH			7
//------------------------------------------------------------------------------------

#define PRESET_VOL		30	//0: mute; 31: maximum volume


bit KT_WirelessMicRx_PreInit(void);
bit KT_WirelessMicRx_Init(void);

bit KT_WirelessMicRx_Standby(void);
bit KT_WirelessMicRx_WakeUp(void);


bit KT_WirelessMicRx_Tune(long Freq); 		//in KHz

uchar KT_WirelessMicRx_Band_Cali_Res(void);

uchar KT_WirelessMicRx_Set_XTAL(bit bXtal_Sel);
uchar KT_WirelessMicRx_SW_XTAL_Freq(bit bXtal_Sel);

bit KT_WirelessMicRx_Volume(uchar cVolume);

void  KT_WirelessMicRx_CheckPilot(void);
void  KT_WirelessMicRx_CheckAutomute(void);
#ifdef KT0616M
void  KT_WirelessMicRx_CheckAUXCH(void);
	#ifdef SQUEAL_EN
		void  KT_WirelessMicRx_CheckSqueal(void);
	#endif
#endif

#ifdef INTERRUPT_EN
void KT_WirelessMicRx_Interrupt(void);
#endif
uint  KT_WirelessMicRx_GetBurstData(void);

void KT_WirelessMicRx_Automute(void);

void KT_WirelessMicRx_SetBass(uchar cBass);
void KT_WirelessMicRx_SetTreble(uchar cTreble);
void KT_WirelessMicRx_SetTHD(uchar cTHD);

uchar KT_WirelessMicRx_GetAF(void);
uchar KT_WirelessMicRx_GetRSSI(void);
char KT_WirelessMicRx_GetAFC(void);


void KT_Bus_Write(uchar Register_Address, uint Word_Data);
uint KT_Bus_Read(uchar Register_Address);
void KT_WirelessMicRx_SQUEAL_Inhibition (void);