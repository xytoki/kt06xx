//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <interface.h>										// SFR declarations
#include <intrins.h>
#include <stdio.h>

extern uchar IAL;
extern uchar TRL;
//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------
uchar code LCD_Welcome1[8]={0x57,0x65,0x6c,0x63,0x6f,0x6d,0x65,0x20};												// Welcome
#ifdef DEMO_RX
uchar code LCD_Welcome2[14]={0x46,0x4D,0x2D,0x52,0x65,0x63,0x65,0x69,0x76,0x65,0x72,0x20,0x20,0x20};				// FM-Receiver
#endif 
#ifdef DEMO_TX
uchar code LCD_Welcome2[14]={0x46,0x4D,0x2D,0x54,0x72,0x61,0x6e,0x73,0x6d,0x69,0x74,0x74,0x65,0x72};				// FM-Transmitter
#endif
uchar code LCD_Welcome3[16]={0x20,0x20,0x20,0x20,0x4B,0x54,0x20,0x4D,0x69,0x63,0x72,0x6F,0x20,0x20,0x20,0x20};		// KT Micro
uchar code LCD_Welcome4[10]={0x32,0x30,0x30,0x38,0x2E,0x20,0x37,0x2E,0x20,0x20};									// 2008年7月
uchar code LCD_Frequency_Title[6]={0x46,0x72,0x65,0x2E,0xA1,0xC3};						// Frequency：
uchar code LCD_Number[10]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};			// 0,1,2,3,4,5,6,7,8,9
uchar code LCD_RSSI_Title[6]={0x52,0x53,0x53,0x49,0xA1,0xC3};						// RSSI
uchar code LCD_Rfgain_Title[8]={0x50,0x41,0x20,0x47,0x61,0x69,0x6e,0x3a};				// PA Gain：
uchar code LCD_PGA_Title[8]={0x41,0x75,0x20,0x47,0x61,0x69,0x6e,0x3a};		// AudioGain:
uchar code LCD_PGA[22]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x31,0x30,0x31,0x31,0x31,0x32,0x31,0x33,0x31,0x34,0x31,0x35};	//0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
uchar code LCD_PETCS_Title[8]={0x50,0x72,0x65,0x2d,0x65,0x6d,0x70,0x3a};				// Pre-emp:
uchar code LCD_PETCS[4]={0x37,0x35,0x35,0x30};											// 75,50


uchar HexToASCII(uint num)
{
		 switch (num&0x0f){
		 	case 0x00: {return(0x30);}break;
		 	case 0x01: {return(0x31);}break;
		 	case 0x02: {return(0x32);}break;
		 	case 0x03: {return(0x33);}break;
		 	case 0x04: {return(0x34);}break;
		 	case 0x05: {return(0x35);}break;
		 	case 0x06: {return(0x36);}break;
		 	case 0x07: {return(0x37);}break;
		 	case 0x08: {return(0x38);}break;
		 	case 0x09: {return(0x39);}break;
		 	case 0x0a: {return(0x41);}break;
		 	case 0x0b: {return(0x42);}break;
		 	case 0x0c: {return(0x43);}break;
		 	case 0x0d: {return(0x44);}break;
		 	case 0x0e: {return(0x45);}break;
		 	case 0x0f: {return(0x46);}break;
		 }
}
//------------------------------------------------------------------------------------
// KEY_Scan
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
//键值表
//Next	: 0x48,01001000		Previous: 0x28,00101000		Autoscan: 0x18,00011000
//Up	: 0x44,01000100		Down	: 0x24,00100100		RDS		: 0x14,00010100
//3D	: 0x42,01000010		Bass	: 0x22,00100010		S/M		: 0x12,00010010
//EQU	: 0x41,01000001		Save	: 0x21,00100001		Mute	: 0x11,00010001
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// 键盘扫描去抖动延时函数
//------------------------------------------------------------------------------------
#ifdef C8051F340
void dlms(void)												// 键盘扫描去抖动延时函数
{
	unsigned int i,j;
	for (i=0x1000;i>0;i--)
		{
		 for (j=7;j>0;j--);
		}
}
#endif


//------------------------------------------------------------------------------------
// 键盘扫描程序
//------------------------------------------------------------------------------------
uchar KEY_Scan (void)
{
	uchar sccode,recode;
	KeyInterface=0xF0;
	delay();												// 行扫描信号全发0
	if((KeyInterface&0xF0)!=0xF0)										// 判断是否有键按下
		{
		 delay();
		 dlms();											// 延时去抖动
		 delay();
		 if ((KeyInterface&0xF0)!=0xF0)								// 还有键按下
			{
			 delay();
			 sccode=0xFE;									// 逐行扫描初值
			 delay();
			 while((sccode&0x10)!=0)
				{delay();
				 KeyInterface=sccode;									// 输出行扫描码
				 delay();
				 if((KeyInterface & 0xF0) != 0xF0)					// 如果本行有键按下
					{
					 delay();
					 recode=(KeyInterface & 0xF0) | 0x0F;				// 输出列扫描码
					 delay();
					 return ((~sccode)+(~recode));			// 求出键值，并返回键值
					 delay();
					}
				 else sccode=(sccode<<1)|0x01;				// 如果本行无键按下，则扫下一行
				}
			}
		}
	return(0);												// 无键按下，返回键值为0
}

#ifdef C8051F314

void Delay_ms(uint time_number)
{
	uint i;
	uint j;

	for (j=0;j<time_number;j++)
	{
			for (i=0;i<0x900;i++)
		{
		}
	}
}

#endif

#ifdef C8051F340

void Delay_ms(uint time_number)
{
	uint i;
	uint j;

	for (j=time_number;j>0;j--)
	{
			for (i=0x1118;i>0;i--)
		{
		}
	}
}
/**/
#endif
//------------------------------------------------------------------------------------
// delay
//------------------------------------------------------------------------------------

void delay(void	)		//<<<修改11
{
	uchar i;	//<<<修改14
	for(i=0;i<=1;i++)
		{
		 _nop_();
		}
}

//------------------------------------------------------------------------------------
// LCD显示函数
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// LCD显示初始化子程序
//------------------------------------------------------------------------------------
void LCD_Init(void)
{
	LCD_Reset=0;
	delay();												// LCD初始化延时
	LCD_Reset=1;											// 禁止复位
	LCD_RW=0;												// 写入模式
	LCD_Enalbe=0;
	CODE_Send(0x30);										// 扩充功能设定：RE=0基本指令集动作、G=0绘图显示关
	CODE_Send(0x01);										// 清除显示
	CODE_Send(0x06);										// 点设定：I/D=1光标右移、S=0 DDRAM为读状态，整体显示不移动
	CODE_Send(0x0c);										// 显示状态D=整体显示开、C=游标关、B=游标位置关
	CODE_Send(0x01);										// 清除显示
	DISPLAY_Start();										// 显示欢迎画面
	DELAY_Display();										// 显示欢迎画面延时
	CODE_Send(0x01);										// 清除显示
//	Display1();												// 第一行显示子程序
//	Display2();												// 第二行显示子程序
//	Display3();												// 第三行显示子程序
//	Display4();												// 第四行显示子程序
}

void Display_Start(void)
{
	uchar wide,offset;										// 定义字符/汉字显示长度，定义显示偏移量
	CODE_Send(0x82);
	for(wide=0;wide<4;wide++)								// 欢迎画面第一行，显示：欢迎使用
		{
		 offset=wide<<1;									// LCD显示数组每2位1组
		 DATA_Send(LCD_Welcome1[offset]);
		 DATA_Send(LCD_Welcome1[offset+1]);
		}

	CODE_Send(0x90);
	for(wide=0;wide<7;wide++)								// 欢迎画面第二行，显示：调频发射机系统  
		{
		 offset=wide<<1;									// LCD显示数组每2位1组
		 DATA_Send(LCD_Welcome2[offset]);
		 DATA_Send(LCD_Welcome2[offset+1]);
		}

	CODE_Send(0x88);
	for(wide=0;wide<8;wide++)								// 欢迎画面第三行，显示：KT Micro亢鹤凯
		{
		 offset=wide<<1;									// LCD显示数组每2位1组
		 DATA_Send(LCD_Welcome3[offset]);
		 DATA_Send(LCD_Welcome3[offset+1]);
		}

	CODE_Send(0x9a);
	for(wide=0;wide<5;wide++)								// 欢迎画面第四行，显示：2007年11月
		{	
		 offset=wide<<1;									// LCD显示数组每2位1组
		 DATA_Send(LCD_Welcome4[offset]);
		 DATA_Send(LCD_Welcome4[offset+1]);
		}
}

/*
//------------------------------------------------------------------------------------
//第一行显示子程序
//------------------------------------------------------------------------------------

void Display1(void)
{
	uchar wide,offset;										// 定义字符/汉字显示长度，定义显示偏移量
	CODE_Send(0x80);										// 频率：
	for(wide=0;wide<3;wide++)
		{
		 offset=wide<<1;									// LCD显示数组每2位1组
		 DATA_Send(LCD_Frequency_Title[offset]);
		 DATA_Send(LCD_Frequency_Title[offset+1]);
		}
   DATA_Send(0x2E);										// .
   DATA_Send(0x2E);										// .
   DATA_Send(0x2E);										// .
   DATA_Send(0x2E);										// .
   DATA_Send(0x2E);										// .
	DATA_Send(0x4D);										// M
	DATA_Send(0x48);										// H
	DATA_Send(0x7A);										// z
}

//------------------------------------------------------------------------------------
//第二行显示子程序
//------------------------------------------------------------------------------------
#ifdef DEMO_RX
void Display2(void)   
{
	uchar wide,offset;										// 定义字符/汉字显示长度，定义显示偏移量
	CODE_Send(0x90);										// 音频增益：
	for(wide=0;wide<5;wide++)
		{
		 offset=wide<<1;									// LCD显示数组每2位1组
		 DATA_Send(LCD_PGA_Title[offset]);
		 DATA_Send(LCD_PGA_Title[offset+1]);
		}
//	offset=IAL*3;											// LCD显示数组每3位1组
	CODE_Send(0x94);										// 显示：xxx（-12-12）dB
	if(IAL<10)
		{
		DATA_Send(0x20);
		DATA_Send(LCD_PGA[IAL]);
		}
	else
		{
		DATA_Send(LCD_PGA[IAL/10]);
		DATA_Send(LCD_PGA[IAL%10]);
		}
//		DATA_Send(LCD_PGA[offset+1]);
//		DATA_Send(LCD_PGA[offset+2]);
//		DATA_Send(0x64);
//		DATA_Send(0x42);
}
#endif
#ifdef DEMO_TX
void Display2(void)
{
	uchar offset;											// 定义字符/汉字显示长度，定义显示偏移量
	CODE_Send(0x90);										// 发射功率：
	for(offset=0;offset<8;offset++)
	{
		DATA_Send(LCD_Rfgain_Title[offset]);
	}

	CODE_Send(0x94);										// 显示：x（1-15）level
	DATA_Send( (TRL/10)+ 0x30);
	DATA_Send( (TRL%10)+ 0x30);
//	DATA_Send(0x4C);
//	DATA_Send(0x65);
//	DATA_Send(0x76);
//	DATA_Send(0x65);
//	DATA_Send(0x6C);
}
#endif
//------------------------------------------------------------------------------------
//第三行显示子程序	RSSI
//------------------------------------------------------------------------------------
#ifdef DEMO_RX
void Display3(void)
{
	uchar wide,offset;										// 定义字符/汉字显示长度，定义显示偏移量
	CODE_Send(0x88);										// 发射功率：
	for(wide=0;wide<3;wide++)
		{
		 offset=wide<<1;									// LCD显示数组每2位1组
		 DATA_Send(LCD_RSSI_Title[offset]);
		 DATA_Send(LCD_RSSI_Title[offset+1]);
		}
}
#endif
#ifdef DEMO_TX
void Display3(void)   
{
	uchar offset;											// 定义字符/汉字显示长度，定义显示偏移量
	CODE_Send(0x88);										// 音频增益：
	for(offset=0;offset<8;offset++)
	{
		DATA_Send(LCD_PGA_Title[offset]);
	}

	CODE_Send(0x8C);										// 显示-/+xx
	if(IAL<15)
	{
		DATA_Send('-');
		DATA_Send( (15-IAL)/10 + 0x30 );
		DATA_Send( (15-IAL)%10 + 0x30 );
	}
	else if(IAL==15)
	{
		DATA_Send(' ');
		DATA_Send(' ');
		DATA_Send('0');
	}
	else if(IAL>15)
	{
		DATA_Send('+');
		DATA_Send( (IAL-15)/10 + 0x30  );
		DATA_Send( (IAL-15)%10 + 0x30  );
	}

	DATA_Send(0x64);
	DATA_Send(0x42);
}
#endif
//------------------------------------------------------------------------------------
//第四行显示子程序
//------------------------------------------------------------------------------------

#ifdef DEMO_RX
void Display4(void)											// 显示：预加重和静音
{
#ifdef TST
	uint reg12;
	reg12 = KT_Bus_Read(0x12);
	reg12 = reg12 &	0x0200;
	if (reg12 == 0x0200)
		{
		 CODE_Send(0x98);										// LCD显示"S"符号
		 DATA_Send(0x53);
		 DATA_Send(0x74);
		 DATA_Send(0x65);
		 DATA_Send(0x72);
		 DATA_Send(0x65);
		 DATA_Send(0x6f);
		}
	else
		{
		 CODE_Send(0x98);										// LCD显示"M"符号
		 DATA_Send(0x4D);
		 DATA_Send(0x6f);
		 DATA_Send(0x6e);
		 DATA_Send(0x6f);
		 DATA_Send(0x20);
		 DATA_Send(0x20);
		}

#else
	CODE_Send(0x98);										// LCD显示"S"符号
	DATA_Send(0x53);
	DATA_Send(0x74);
	DATA_Send(0x65);
	DATA_Send(0x72);
	DATA_Send(0x65);
	DATA_Send(0x6f);

	CODE_Send(0x9C);										// 显示B0-B3
	DATA_Send(0x42);
	DATA_Send(0x41);
	DATA_Send(0x53);
	DATA_Send(0x53);
	DATA_Send(BASS+0x30);

#endif
	CODE_Send(0x9F);
	DATA_Send(0x0E);										//显示音乐符号
}
#endif
#ifdef DEMO_TX
void Display4(void)											// 显示：预加重和静音
{
//	uchar offset;											// 定义字符/汉字显示长度，定义显示偏移量
	CODE_Send(0x98);										// 预加重：
//	for(offset=0;offset<8;offset++)
// 	{
//		DATA_Send(LCD_PETCS_Title[offset]);
//	}

	if( (I2C_Byte_Read(KT0806r_address, 0x02) & 0x01) == 0)	// 检查预加重标志位，如果标志位为低电平说明预加重为75us
	{
		DATA_Send('7');
		DATA_Send('5');
	}
	else													// 检查预加重标志位，如果标志位为高电平说明预加重为50us
	{
		DATA_Send('5');
		DATA_Send('0');
	}
	DATA_Send('u');
	DATA_Send('s');

	CODE_Send(0x9B);										// LCD显示“BASS”
	DATA_Send('B');
	DATA_Send('A');
	DATA_Send('S');
	DATA_Send('S');
	DATA_Send(BASS_LEVEL+ 0x30);

	CODE_Send(0x9F);
	if( (I2C_Byte_Read(KT0806r_address, 0x04) & 0x40) == 0)	// 检查Mono标志位，如果是当前是立体声则显示“ST”
	{
		DATA_Send('S');
		DATA_Send('T');

	}
	else													// 检查Mono标志位，如果是当前是单声道则显示“ M”
	{
		DATA_Send(' ');
		DATA_Send('M');
	}
}
#endif
*/
//------------------------------------------------------------------------------------
// 显示延时
//------------------------------------------------------------------------------------
void DELAY_Display(void)									// 显示欢迎画面延时
{
	unsigned int i,j;
	for (i=0x2000;i>0;i--)
		{
		 for (j=50;j>0;j--);
		}
}

//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void DELAY_Write(void)										// 显示欢迎画面延时
{
	uchar i;
	for(i=0;i<100;i++)
    {}
}

//------------------------------------------------------------------------------------
// 数据发送子程序
//------------------------------------------------------------------------------------
void DATA_Send(uchar data mydata)
{
	BUSY_Check();											// 检查是否处于忙状态
	DELAY_Write();
	LCDInterface=BYTE_Reversal(mydata);								// 写入mydata代表的数据
	DELAY_Write();
	LCD_RS=1;												// 数据模式
	DELAY_Write();
	LCD_RW=0;												// 写入模式
	DELAY_Write();
	LCD_Enalbe=1;											// 允许写入
	DELAY_Write();
	LCD_Enalbe=0;											// 禁止写入
	DELAY_Write();
	LCDInterface=0xff;												// P2口置全1
}
//------------------------------------------------------------------------------------
// 命令发送子程序
//------------------------------------------------------------------------------------
void CODE_Send(uchar data command)
{
	BUSY_Check();											// 检查是否处于忙状态
	DELAY_Write();
	LCDInterface=BYTE_Reversal(command);								// 写入command代表的指令码
	DELAY_Write();
	LCD_RS=0;												// 指令模式
	DELAY_Write();
	LCD_RW=0;												// 写入模式
	DELAY_Write();
	LCD_Enalbe=1;											// 允许写入
	DELAY_Write();
	LCD_Enalbe=0;											// 禁止写入
	DELAY_Write();
	LCDInterface=0xff;												// P2口置全1

}
//------------------------------------------------------------------------------------
// 检测忙信号子程序
//------------------------------------------------------------------------------------
void BUSY_Check(void)										// 忙信号检查子程序
{
	DELAY_Write();
	LCD_RS=0;												// 指令模式
	DELAY_Write();
	LCD_RW=1;												// 读出模式
	DELAY_Write();
	LCD_Enalbe=1;											// 允许读出
	DELAY_Write();
	while(LCD_BF){;}										// 等待，直到不忙为止
	DELAY_Write();
	LCD_Enalbe=0;											// 禁止输入
	DELAY_Write();
}
//------------------------------------------------------------------------------------
// 1Byte 反转程序
//------------------------------------------------------------------------------------
uchar BYTE_Reversal(uchar Reversal_data)
{
	uchar temp_data=0;
	bit Reversal_data_0=0;
	bit Reversal_data_1=0;
	bit Reversal_data_2=0;
	bit Reversal_data_3=0;
	bit Reversal_data_4=0;
	bit Reversal_data_5=0;
	bit Reversal_data_6=0;
	bit Reversal_data_7=0;

	Reversal_data_0 = Reversal_data & 0x01;
	Reversal_data=Reversal_data>>1;
	Reversal_data_1 = (Reversal_data)&0x01;
	Reversal_data=Reversal_data>>1;
	Reversal_data_2 = (Reversal_data)&0x01;
	Reversal_data=Reversal_data>>1;
	Reversal_data_3 = (Reversal_data)&0x01;
	Reversal_data=Reversal_data>>1;
	Reversal_data_4 = (Reversal_data)&0x01;
	Reversal_data=Reversal_data>>1;
	Reversal_data_5 = (Reversal_data)&0x01;
	Reversal_data=Reversal_data>>1;
	Reversal_data_6 = (Reversal_data)&0x01;
	Reversal_data=Reversal_data>>1;
	Reversal_data_7 = (Reversal_data)&0x01;

	temp_data=temp_data | Reversal_data_0;

	temp_data=temp_data<<1;
	temp_data=(temp_data) | Reversal_data_1;

	temp_data=temp_data<<1;
	temp_data=(temp_data) | Reversal_data_2;

	temp_data=temp_data<<1;
	temp_data=(temp_data) | Reversal_data_3;

	temp_data=temp_data<<1;
	temp_data=(temp_data) | Reversal_data_4;

	temp_data=temp_data<<1;
	temp_data=(temp_data) | Reversal_data_5;

	temp_data=temp_data<<1;
	temp_data=(temp_data) | Reversal_data_6;

	temp_data=temp_data<<1;
	temp_data=(temp_data) | Reversal_data_7;

	return (temp_data);
}
//-----------------------------------------------------------------------------
// LCD_RSSI_Number_Calculator
//-----------------------------------------------------------------------------
//
//
// 
//
/*
void LCD_RSSI_Display (uchar RSSI_Number)
{
	uchar RSSI_Hundred_Remainder,RSSI_Hundred,RSSI_Ten_Remainder,RSSI_Ten;
	uchar data RSSI_high,RSSI_mid,RSSI_low;


	RSSI_Hundred_Remainder = RSSI_Number % 100;
	RSSI_Hundred = RSSI_Number / 100;
	RSSI_high = 0x30 + RSSI_Hundred;

	RSSI_Ten_Remainder = RSSI_Hundred_Remainder % 10;
	RSSI_Ten = RSSI_Hundred_Remainder / 10;
	RSSI_mid = 0x30 + RSSI_Ten;
	RSSI_low = 0x30 + RSSI_Ten_Remainder;
	 CODE_Send(0x8B);										// 显示：-XXX
	 DATA_Send(0x2D);//"-"
	 DATA_Send(RSSI_high);//
	 DATA_Send(RSSI_mid);
	 DATA_Send(RSSI_low);

}


void Display_Channel (uint LCD_Number)
{
	uint HundredMHz_Remainder,TenMHz_Remainder,MHz_Remainder,HundredKHz_Remainder;
	uchar HundredMHz,TenMHz,MHz,HundredKHz;

	HundredMHz_Remainder = LCD_Number % 10000;
	HundredMHz = LCD_Number / 10000;

	TenMHz_Remainder = HundredMHz_Remainder % 1000;
	TenMHz = HundredMHz_Remainder / 1000;

	MHz_Remainder = TenMHz_Remainder % 100;
	MHz = TenMHz_Remainder / 100;

	HundredKHz_Remainder = MHz_Remainder % 10;
	HundredKHz = MHz_Remainder / 10;

	CODE_Send(0x83);										// 显示xxx.x
	if(HundredMHz == 0)
		DATA_Send(0x20);
	else
		DATA_Send(0x30 + HundredMHz);
	DATA_Send(0x30 + TenMHz);
	DATA_Send(0x30 + MHz);
	DATA_Send(0x2E);
	DATA_Send(0x30 + HundredKHz);
//	DATA_Send(0x30 + HundredKHz_Remainder);
}
 */