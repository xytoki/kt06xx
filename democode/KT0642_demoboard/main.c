//*********************************************************
//	File Name: Main.c
//	Function:  KT06xx EEPROM Programmer
//	Rev:       1.2
//	Date:      2014-06-13
//*********************************************************
//		Revision History
//  Version	Date		Description
//	V1.0	2012-02-01	Initial version
//	V1.1	2013-04-08	Modified the function of "Write_EEPROM".
//	V1.2	2014-06-13	Supported KT062x.
//	V1.3	2016-09-26	Supported KT064x.
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <intrins.h>
#include <stdio.h>
#include <interface.h>
#include <eeprom.h>

#define EEPROM_Address 0xA0
//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
void IIC_Slave_Task(void);
extern unsigned char xdata buffer_reg[128];
//extern bit dataOk;

void KT06xx_Next_Page (void);								
void KT06xx_Previous_Page (void);						
void KT06xx_show2 (void);
void SYSCLK_Init (void);
void PORT_Init (void);

uchar EEPROM_Index=0;
void KT_Bus_Write(uchar Register_Address, uint iRegData);
uint KT_Bus_Read(uchar Register_Address);
void Write_EEPROM(void);
void Verify_EEPROM(uint EEPROM_Data);

char Rstate;
int index = 0;
int regg = 0;

//------------------------------------------------------------------------------------
//函 数 名：main
//功能描述：主程序
//函数说明：
//调用函数：SYSCLK_Init(); PORT_Init(); LCD_Init(); IIC_Slave_Task();
//			KEY_Scan(); KT06xx_Previous_Page(); KT06xx_Next_Page();
//			KT06xx_show2(); Write_EEPROM();
//全局变量：无
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-02-01
//修 改 者：						时间：
//版    本：V1.0	2012-02-01	Initial version
//------------------------------------------------------------------------------------
void main (void)
{ 	
	unsigned char key = 0;
	unsigned int Regx;
	Rstate = 1;

	REG0CN	|= 0x80;												// Disable REG0 of C8051F340
	PCA0MD &= ~0x40;                       							// WDTE = 0 (clear watchdog timer enable)
	SYSCLK_Init ();                        							// Initialize system clock to 24.5MHz
	PORT_Init ();                          							// Initialize crossbar and GPIO
	LCD_Init();

	index = 16;

	KeyInterface = 0xf0;
/*	
	Regx = KT_Bus_Read(0x00);
	Regx = KT_Bus_Read(0x01);
	Regx = KT_Bus_Read(0x40);
	Regx = KT_Bus_Read(0x41);
*/
	while (1)
	{
		if(Rstate)
			IIC_Slave_Task();


//		 if(dataOk)
		 {		
			 if((KeyInterface&0xF0)!=0xF0)
			 {
			 	 key=KEY_Scan();                             					// 取键值
			   	 if (key!=0)             
			     {
					switch (key)                             					// 判断键
					{
//Pre_Page	: 0x48,01001000		Next_Page	: 0x28,00101000		Clear		: 0x18,00011000
//Mode_SW	: 0x44,01000100		N.A.		: 0x24,00100100		Write_Start	: 0x14,00010100
//N.A.		: 0x42,01000010		N.A.		: 0x22,00100010		N.A.		: 0x12,00010010
//N.A.		: 0x41,01000001		N.A.		: 0x21,00100001		N.A.		: 0x11,00010001
						case 0x28:{	KT06xx_Previous_Page();	}break;
						case 0x48:{	KT06xx_Next_Page();		}break;
						case 0x18:{	KT06xx_show2();			}break;
						case 0x14:{	Write_EEPROM();			}break;
						
					}
				 }

				 KeyInterface = 0xf0;
			 }
		 }
	}
}




//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// This routine initializes the system clock to use the internal 24.5MHz / 8 
// oscillator as its clock source.  Also enables missing clock detector reset.
//
void SYSCLK_Init (void)
{
 	OSCICN |= 0x03;   //0xc3								// configure internal oscillator for 12MHz
}

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Configure the Crossbar and GPIO ports.
// P3.3 - LED (push-pull)
//
void PORT_Init (void)
{

	XBR0    = 0x00;											// Disable all
	XBR1    = 0x40;											// Enable crossbar and Disable weak pull-ups
	P0MDOUT	= 0x00;
	P1MDOUT	= 0x00;
	P2MDOUT	= 0x7F;											// set P2^0-P2^6 push-pull
	P3MDOUT	= 0x02;											// set P3.1 push-pull
//	P0 = 0xff;
//	P1 = 0xff;
}

//-----------------------------------------------------------------------------
// KT06xx_Previous_Page
//-----------------------------------------------------------------------------
void KT06xx_Previous_Page (void)								
{

  while(1)
   {
   		unsigned char key;
		key=KEY_Scan();                             					// 取键值

   	 	if (key==0)
	 		break; 
	} 

    CODE_Send(0x01);

	regg = (index)/2;

  	CODE_Send(0x80);

	DATA_Send(HexToASCII(regg/16));
	DATA_Send(HexToASCII(regg%16));
 	DATA_Send(' ');
	DATA_Send(':');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));

	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;

	DATA_Send(' ');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));

//----------------------------------------------------------
	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;

	regg = (index)/2;

  	CODE_Send(0x90);

	DATA_Send(HexToASCII(regg/16));
	DATA_Send(HexToASCII(regg%16));
 	DATA_Send(' ');
	DATA_Send(':');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));

	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;

	DATA_Send(' ');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));
//----------------------------------------------------------
	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;

	regg = (index)/2;

  	CODE_Send(0x88);

	DATA_Send(HexToASCII(regg/16));
	DATA_Send(HexToASCII(regg%16));
 	DATA_Send(' ');
	DATA_Send(':');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));

	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;

	DATA_Send(' ');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));
	
	//----------------------------------------------------------
	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;

	regg = (index)/2;

  	CODE_Send(0x98);

	DATA_Send(HexToASCII(regg/16));
	DATA_Send(HexToASCII(regg%16));
 	DATA_Send(' ');
	DATA_Send(':');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));

	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;

	DATA_Send(' ');
	DATA_Send(' ');
	DATA_Send(HexToASCII(buffer_reg[index]/16));
	DATA_Send(HexToASCII(buffer_reg[index]%16));
	DATA_Send(HexToASCII(buffer_reg[index+1]/16));
	DATA_Send(HexToASCII(buffer_reg[index+1]%16));

	index+=2;
	if(index >= 0x80)
//	if(index >= 0xC0)
		index = 0;
}

void KT06xx_Next_Page (void)									
{
  while(1)
   {
   		unsigned char key;
		key=KEY_Scan();                             			// 取键值

   	 	if (key==0)
	 		break; 
	} 

	if(index < 32)
		index = index + 0x80 - 32;
//		index = index + 0xC0 - 32;
	else
		index = index - 32; 

	KT06xx_Previous_Page();
}

void LCD_show (void)								
{

    CODE_Send(0x01);
	regg = (index)/2;
  	CODE_Send(0x80);

 	CODE_Send(0x90);
	DATA_Send('W');
	DATA_Send('a');
 	DATA_Send('i');
 	DATA_Send('t');

	Delay_ms(100);


	if(index < 16)
		index = index + 0x80 - 16;
//		index = index + 0xC0 - 16;
	else
		index = index - 16; 

	KT06xx_Previous_Page();


}

void KT06xx_show2 (void)								
{
  while(1)
   {
   		unsigned char key;
		key=KEY_Scan();                             					// 取键值

   	 	if (key==0)
	 		break; 
	} 
	index = 0;
    CODE_Send(0x01);

 	CODE_Send(0x90);
	DATA_Send('c');
	DATA_Send('l');
 	DATA_Send('e');
	DATA_Send('a');
	DATA_Send('r');
  
  {
  		unsigned char u = 0;
		for(u = 0; u< 128; u++)
		{
			buffer_reg[u] = 0;
		}
  }

  //dataOk = 0;
}



//------------------------------------------------------------------------------------
//函 数 名：Write_EEPROM
//功能描述：EEPROM烧写程序
//函数说明：
//调用函数：KT_Bus_Write(); Delay_ms(); Verify_EEPROM(); 
//			CODE_Send(); DATA_Send(); KT06xx_Next_Page();
//全局变量：EEPROM_error
//输    入：
//返    回：无
//设 计 者：KANG Hekai				时间：2012-02-01
//修 改 者：KANG Hekai				时间：2016-09-26
//版    本：V1.0	2012-02-01	Initial version
//			V1.1	2013-04-08	Modified the function of "Write_EEPROM".
//			V1.2	2014-06-13	Supported KT062x.
//			V1.3	2016-09-26	Supported KT064x.
//------------------------------------------------------------------------------------
char EEPROM_error;

void Write_EEPROM(void)
{	
//	uint Regx;
//	if(Rstate)
//		return;
	EEPROM_error = 0;
	for(EEPROM_Index=0;EEPROM_Index<EEPROM_SIZE;EEPROM_Index++)
	{
#ifdef KT0602
		KT_Bus_Write(EEPROM_Index,KT0602_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0602_REG[EEPROM_Index]);
#endif
#ifdef KT0603
		KT_Bus_Write(EEPROM_Index,KT0603_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0603_REG[EEPROM_Index]);
#endif
#ifdef KT0612
		KT_Bus_Write(EEPROM_Index,KT0612_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0612_REG[EEPROM_Index]);
#endif
#ifdef KT0613
		KT_Bus_Write(EEPROM_Index,KT0613_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0613_REG[EEPROM_Index]);
#endif
#ifdef KT0622
		KT_Bus_Write(EEPROM_Index,KT0622_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0622_REG[EEPROM_Index]);
#endif
#ifdef KT0623
		KT_Bus_Write(EEPROM_Index,KT0623_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0623_REG[EEPROM_Index]);
#endif
#ifdef KT0622D
		KT_Bus_Write(EEPROM_Index,KT0622D_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0622D_REG[EEPROM_Index]);
#endif
#ifdef KT0623D
		KT_Bus_Write(EEPROM_Index,KT0623D_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0623D_REG[EEPROM_Index]);
#endif
#ifdef KT0642
		KT_Bus_Write(EEPROM_Index,KT0642_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0642_REG[EEPROM_Index]);
#endif
#ifdef KT0643D
		KT_Bus_Write(EEPROM_Index,KT0643D_REG[EEPROM_Index]);
		Delay_ms(10);
		Verify_EEPROM(KT0643D_REG[EEPROM_Index]);
#endif
		if(EEPROM_error)
			break;
	}

	CODE_Send(0x01);										// 清除显示

	CODE_Send(0x98);	
	DATA_Send( ' ' );
	DATA_Send( ' ' );
	DATA_Send( 'W' );
	DATA_Send( 'r' );
	DATA_Send( 'i' );
	DATA_Send( 't' );
	DATA_Send( 'e' );
	DATA_Send( ' ' );
	if(EEPROM_error)
	{
		DATA_Send( 'e' );
		DATA_Send( 'r' );
		DATA_Send( 'r' );
		DATA_Send( 'o' );
		DATA_Send( 'r' );
	}else{
		DATA_Send( ' ' );
		DATA_Send( 'o' );
		DATA_Send( 'k' );
	}

}

void KT_Bus_Write(uchar Register_Address, uint iRegData)
{
	I2C_Word_Write(EEPROM_Address, Register_Address * 2, iRegData);
}

uint KT_Bus_Read(uchar Register_Address)
{
	return( I2C_Word_Read(EEPROM_Address, Register_Address * 2) );
}

void Verify_EEPROM(uint EEPROM_Data)
{
	uint EEPROM_Reg;
	EEPROM_Reg = KT_Bus_Read(EEPROM_Index);
	while ( EEPROM_Reg != EEPROM_Data)
	{
		EEPROM_Reg = KT_Bus_Read(EEPROM_Index);
		KT_Bus_Write(EEPROM_Index,EEPROM_Data);

		CODE_Send(0x80);	
		DATA_Send( 'R' );
		DATA_Send( 'e' );
		DATA_Send( 'g' );
		DATA_Send( HexToASCII(EEPROM_Index>>4) );
		DATA_Send( HexToASCII(EEPROM_Index) );
		DATA_Send( ' ' );
		DATA_Send( 'W' );
		DATA_Send( 'r' );
		DATA_Send( 'i' );
		DATA_Send( 't' );
		DATA_Send( 'e' );
		DATA_Send( 'E' );
		DATA_Send( 'r' );
		DATA_Send( 'r' );
		DATA_Send( 'o' );
		DATA_Send( 'r' );

		CODE_Send(0x90);
		DATA_Send( ' ' );	
		DATA_Send( 'T' );
		DATA_Send( 'r' );
		DATA_Send( 'y' );
		DATA_Send( ' ' );
		DATA_Send( 'A' );
		DATA_Send( 'n' );
		DATA_Send( 'o' );
		DATA_Send( 't' );
		DATA_Send( 'h' );
		DATA_Send( 'e' );
		DATA_Send( 'r' );
		DATA_Send( '!' );

		EEPROM_error = 1;
		return;
	}
	CODE_Send(0x01);										// 清除显示
	CODE_Send(0x80);	
	DATA_Send( 'R' );
	DATA_Send( 'e' );
	DATA_Send( 'g' );
	DATA_Send( HexToASCII(EEPROM_Index>>4) );
	DATA_Send( HexToASCII(EEPROM_Index) );
	DATA_Send( '=' );
	DATA_Send( '0' );
	DATA_Send( 'x' );
	DATA_Send( HexToASCII(EEPROM_Data>>12) );
	DATA_Send( HexToASCII(EEPROM_Data>>8) );
	DATA_Send( HexToASCII(EEPROM_Data>>4) );
	DATA_Send( HexToASCII(EEPROM_Data) );
	DATA_Send( ' ' );
	DATA_Send( ' ' );
	DATA_Send( 'o' );
	DATA_Send( 'k' );
	Delay_ms(10);
}