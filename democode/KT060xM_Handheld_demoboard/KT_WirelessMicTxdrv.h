//*********************************************************************
//  File Name: KT_WirelessMicTxdrv.h
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
//  V2.4	2012-08-20	The function KT_WirelessMicTx_PowerDownProcedure is added. 

#define uchar unsigned char
#define ulong unsigned long
#define uint unsigned int

//#define KT0605M
#define KT0606M

#define KT_RX					//使用KT芯片的接收机
//#define OTHER_RX				//使用其他方案的接收机

#define KTWirelessMicTxw_address 0x6A
#define KTWirelessMicTxr_address 0x6B


#define XTAL_24M_FREQ 0
#define XTAL_24P576M_FREQ 1
#define XTAL_DUAL
//#define XTAL_24M_ONLY
//#define XTAL_24P576M_ONLY

//#define TUNE_V1

#define INIT_FAIL_TH 3

#ifdef OTHER_RX
	#define PILOT_FREQ		32768
#endif

#define	BAND_TOP		850000
#define	BAND_BOTTOM		800000
#define	BAND_STEP		250

#define PA_SEL 1

#define PA_OFF 0
#define PA_ON  1

#define	AUDIO_UNMUTE	0
#define	AUDIO_MUTE		1

#define	WAKEUP	0
#define	STANDBY	1

#define	PA_GAIN_STEP	1
#define	PA_Neg24dBm		0
#define	PA_Neg21dBm		1
#define	PA_Neg18dBm		2
#define	PA_Neg15dBm		3
#define	PA_Neg12dBm		4
#define	PA_Neg9dBm		5
#define	PA_Neg6dBm		6
#define	PA_Neg3dBm		7
#define	PA_Pos0dBm		8
#define	PA_Pos3dBm		9
#define	PA_Pos6dBm		10
#define	PA_Pos9dBm		11
#define	PA_Pos12dBm		12

#define	FDEV_MONITOR_TC_250ms	0
#define	FDEV_MONITOR_TC_500ms	1
#define	FDEV_MONITOR_TC_1s		2
#define	FDEV_MONITOR_TC_2s		3

#define	PREEMPASIS_ENABLE	0
#define	PREEMPASIS_DISABLE	1

#define	MIC_SENS_GAIN_0		0
#define	MIC_SENS_GAIN_1		1
#define	MIC_SENS_GAIN_2		2
#define	MIC_SENS_GAIN_3		3
#define	MIC_SENS_GAIN_4		4
#define	MIC_SENS_GAIN_5		5
#define	MIC_SENS_GAIN_6		6
#define	MIC_SENS_GAIN_7		7
#define	MIC_SENS_GAIN_8		8
#define	MIC_SENS_GAIN_9		9
#define	MIC_SENS_GAIN_10	10
#define	MIC_SENS_GAIN_11	11
#define	MIC_SENS_GAIN_12	12
#define	MIC_SENS_GAIN_13	13
#define	MIC_SENS_GAIN_14	14
#define	MIC_SENS_GAIN_15	15

#define	COMPANDOR_ENABLE	0
#define	COMPANDOR_DISABLE	1

#define	COMPANDOR_TC_6ms		0
#define	COMPANDOR_TC_12ms		1
#define	COMPANDOR_TC_24ms		2
#define	COMPANDOR_TC_48ms		3
#define	COMPANDOR_TC_93ms		4
#define	COMPANDOR_TC_199ms		5
#define	COMPANDOR_TC_398ms		6
#define	COMPANDOR_TC_796ms		7

#ifdef KT_RX					//使用KT芯片的接收机
	#define	PILOT_ENABLE			1
	#define	PILOT_DISABLE			0
#endif

#ifdef OTHER_RX				//使用其他方案的接收机
	#define	PILOT_ENABLE			0
	#define	PILOT_DISABLE			1
#endif
#define	PILOT_FDEV_2P5K			0
#define	PILOT_FDEV_5K			1
#define	PILOT_FDEV_7P5K			2
#define	PILOT_FDEV_10K			3

#define	BATTERY_METER_DISABLE	0
#define	BATTERY_METER_ENABLE	1

#define	BATTERY_MAX				0x7FF
#define	BATTERY_HIGHTH			0x500
#define	BATTERY_MIDDLETH		0x4C0
#define	BATTERY_LOWTH			0x4A0

#define	LOWVOLTAGE_TH			1400
//#define	LOWVOLTAGE_TH		1800

#define AUXCH_EN				1		//BIT 15
#define AUXDATA_EN				1		//BIT 14
#define AUX_REG_NUM				3		//BIT 13:12
#define AUX_CARRY_NUM			3		//BIT 11:9	00:12bit	01:16bit	10:18bit	11:20bit
#define AUX_FDEV				1		//BIT 8:7
#define REG1F ( (AUXCH_EN << 15) | (AUXDATA_EN << 14) | (AUX_REG_NUM << 12) | (AUX_CARRY_NUM << 9) | (AUX_FDEV << 7) )

#define AUX_ADDRB				0x07	//BIT 15:8
#define AUX_ADDRA				0x10	//BIT 7:0

#define AUX_ADDRD				0x11	//BIT 15:8
#define AUX_ADDRC				0x17	//BIT 7:0

bit KT_WirelessMicTx_PreInit(void);
bit KT_WirelessMicTx_Init(void);

bit KT_WirelessMicTx_Standby(void);
bit KT_WirelessMicTx_WakeUp(void);
void KT_WirelessMicTx_PowerDownProcedure(void);

bit KT_WirelessMicTx_PASW(bit bPA_Switch);
bit KT_WirelessMicTx_PAGain(uchar cPaGain);

uchar KT_WirelessMicTx_Fdev_Monitor(void);
bit KT_WirelessMicTx_Fdev_Monitor_TC(uchar cFdev_Mon_TC);
bit KT_WirelessMicTx_PreEmpasis_SW(uchar cPreEmpasis_Dis);
bit KT_WirelessMicTx_Mic_Sens(uchar cMicSens);
bit KT_WirelessMicTx_Comp_Dis(bit bComp_Dis);
bit KT_WirelessMicTx_Comp_TC(uchar cComp_TC);
bit KT_WirelessMicTx_MuteSel(bit bMute_Sel);

bit KT_WirelessMicTx_Pilot(bit bPilot_Dis);
bit KT_WirelessMicTx_Pilot_Fdev(uchar cPilot_Fdev);

#ifdef OTHER_RX
uchar KT_WirelessMicTx_Set_Pilot_Freq(bit bXtal_Sel);
#endif

bit KT_WirelessMicTx_BatteryMeter_SW(bit bBatteryMeter_En);
uint KT_WirelessMicTx_BatteryMeter_Read(void);
bit KT_WirelessMicTx_LowVoltageIndicator(void);

bit KT_WirelessMicTx_Tune(long Freq); 		//in KHz

uchar KT_WirelessMicTx_Band_Cali_Res(void);
uchar KT_WirelessMicTx_Set_XTAL(bit bXtal_Sel);
uchar KT_WirelessMicTx_SW_XTAL_Freq(bit bXtal_Sel);

#ifdef KT0606M
void KT_WirelessMicTx_AUXCH_SendBurstData(uint iBurstData);
#endif

void KT_Bus_Write(uchar Register_Address, uint Word_Data);
uint KT_Bus_Read(uchar Register_Address);