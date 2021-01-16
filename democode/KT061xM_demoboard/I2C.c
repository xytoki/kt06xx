//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <interface.h>										// SFR declarations
#include <intrins.h>
#include <stdio.h>


//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
extern uchar bChannel;

bit Ack_Flag_A=0;													// I2C Ack Flag
bit Ack_Flag_B=0;													// I2C Ack Flag
//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
void I2C_Delay(void);											// I2C延时

void I2C_Start_A(void);
void I2C_Senddata_A(uchar senddata);
void I2C_Ack_A(void);
void I2C_Stop_A(void);
uchar I2C_Receivedata_A(void);

void I2C_Start_B(void);
void I2C_Senddata_B(uchar senddata);
void I2C_Ack_B(void);
void I2C_Stop_B(void);
uchar I2C_Receivedata_B(void);

//------------------------------------------------------------------------------------
// I2C_Delay
//------------------------------------------------------------------------------------

void I2C_Delay(void	)
{
	uchar i;
	for(i=0;i<=40;i++)
		{
		 _nop_();
		}
}

//------------------------------------------------------------------------------------
//I2C Write Word
//------------------------------------------------------------------------------------
void I2C_Word_Write(uchar device_address, uchar reg_add, uint writeword)
{
	uchar writeword_high,writeword_low;

	writeword_low 	=	writeword;
	writeword_high	=	writeword>>8;

	if(bChannel == CH_A)
	{
		
		I2C_Start_A();
		I2C_Senddata_A(device_address & 0xFE);
		I2C_Ack_A();
		if (Ack_Flag_A == 0)
		{
			I2C_Senddata_A(reg_add);
			I2C_Ack_A();
			if (Ack_Flag_A == 0)
			{
				I2C_Senddata_A(writeword_high);
				I2C_Ack_A();
				if (Ack_Flag_A == 0)
				{
					I2C_Senddata_A(writeword_low);
					I2C_Ack_A();
				}
				else SCL_A = 0;	
			}
		 	else SCL_A = 0;			
		}
		else SCL_A = 0;
		I2C_Stop_A();
	}
	else
	{
		I2C_Start_B();
		I2C_Senddata_B(device_address & 0xFE);
		I2C_Ack_B();
		if (Ack_Flag_B == 0)
		{
			I2C_Senddata_B(reg_add);
			I2C_Ack_B();
			if (Ack_Flag_B == 0)
			{
				I2C_Senddata_B(writeword_high);
				I2C_Ack_B();
				if (Ack_Flag_B == 0)
				{
					I2C_Senddata_B(writeword_low);
					I2C_Ack_B();
				}
				else SCL_B = 0;	
			}
		 	else SCL_B = 0;			
		}
		else SCL_B = 0;
		I2C_Stop_B();
	}	
}

//------------------------------------------------------------------------------------
//I2C Read Word
//------------------------------------------------------------------------------------
uint I2C_Word_Read(uchar device_address, uchar reg_add)
{
	unsigned char readdata_low;
	unsigned int readdata,readdata_high,temp=0;

	if(bChannel== CH_A)
	{
		I2C_Start_A();
		I2C_Senddata_A(device_address & 0xFE);
		I2C_Ack_A();
		if (Ack_Flag_A == 0)
		{
			I2C_Senddata_A(reg_add);
			I2C_Ack_A();
			if (Ack_Flag_A == 0)
			{
				I2C_Start_A();
				I2C_Senddata_A(device_address | 0x01);
				I2C_Ack_A();
				if (Ack_Flag_A == 0)
				{
//					SDA_A = 1;//SDA_A 设为输入，读引脚
					readdata_high = I2C_Receivedata_A();
					SDA_A = 0;
					I2C_Delay();I2C_Delay();
					SCL_A = 1;
					I2C_Delay();I2C_Delay();
					SCL_A = 0;
					I2C_Delay();I2C_Delay();
					SDA_A = 1;

				 	if (Ack_Flag_A == 0)
					{
						readdata_low = I2C_Receivedata_A();
						I2C_Ack_A();
					}
					else
					{
						SCL_A=0;
						return(0x0000);
					}
				}
				else
				{
					SCL_A=0;
					return(0x0000);
				}
			}
			 else
			{
				SCL_A = 0;
			 	return(0x0000);			
			}
		}
		else
		{
			SCL_A = 0;
			return(0x0000);
		}

		I2C_Stop_A();
	}
	else
	{
		I2C_Start_B();
		I2C_Senddata_B(device_address & 0xFE);
		I2C_Ack_B();
		if (Ack_Flag_B == 0)
		{
			I2C_Senddata_B(reg_add);
			I2C_Ack_B();
			if (Ack_Flag_B == 0)
			{
				I2C_Start_B();
				I2C_Senddata_B(device_address | 0x01);
				I2C_Ack_B();
				if (Ack_Flag_B == 0)
				{
//					SDA_B = 1;//SDA_B 设为输入，读引脚
					readdata_high = I2C_Receivedata_B();
					SDA_B = 0;
					I2C_Delay();I2C_Delay();
					SCL_B = 1;
					I2C_Delay();I2C_Delay();
					SCL_B = 0;
					I2C_Delay();I2C_Delay();
					SDA_B = 1;

				 	if (Ack_Flag_B == 0)
					{
						readdata_low = I2C_Receivedata_B();
						I2C_Ack_B();
					}
					else
					{
						SCL_B=0;
						return(0x0000);
					}
				}
				else
				{
					SCL_B=0;
					return(0x0000);
				}
			}
			 else
			{
				SCL_B = 0;
			 	return(0x0000);			
			}
		}
		else
		{
			SCL_B = 0;
			return(0x0000);
		}

		I2C_Stop_B();
	}		
	temp=readdata_high<<8;
	readdata=temp | readdata_low;
	return(readdata);
}

//------------------------------------------------------------------------------------
//I2C Start Condition
//------------------------------------------------------------------------------------
void I2C_Start_A(void)
{
	I2C_Delay();I2C_Delay();
	SDA_A = 1;
	I2C_Delay();I2C_Delay();
	SCL_A = 1;
	I2C_Delay();I2C_Delay();
	SDA_A = 0;
	I2C_Delay();I2C_Delay();
	SCL_A = 0;
	I2C_Delay();I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Send Data
//------------------------------------------------------------------------------------
void I2C_Senddata_A(uchar senddata)
{
	uchar i;
	for (i=0;i<8;i++)
	{	
	 I2C_Delay();
	 if ((senddata & 0x80) != 0x80)
		SDA_A = 0;
	 else SDA_A = 1;
		senddata = senddata << 1;
		I2C_Delay();
		SCL_A = 1;
		I2C_Delay();
		SCL_A = 0;
	}
	I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Receive Data
//------------------------------------------------------------------------------------
uchar I2C_Receivedata_A(void)
{
	uchar i,temp,receivedata=0;
	for (i=0;i<8;i++)
		{
		 I2C_Delay();
		 SCL_A = 1;
		 I2C_Delay();
		 temp = SDA_A;
		 SCL_A = 0;
		 receivedata = receivedata | temp;
		 if (i<7)
			{
			 receivedata = receivedata << 1;
			}
		}
	I2C_Delay();
	return(receivedata);	
}
//------------------------------------------------------------------------------------
//I2C Ack
//------------------------------------------------------------------------------------
void I2C_Ack_A(void)
{
	SDA_A = 1;
	I2C_Delay();I2C_Delay();
	SCL_A = 1;
	I2C_Delay();
	Ack_Flag_A = SDA_A;
	SCL_A = 0;
	I2C_Delay();I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Stop Condition
//------------------------------------------------------------------------------------
void I2C_Stop_A(void)
{
	SCL_A = 0;
	I2C_Delay();I2C_Delay();
	SDA_A = 0;
	I2C_Delay();I2C_Delay();
	SCL_A = 1;
	I2C_Delay();I2C_Delay();
	SDA_A = 1;
	I2C_Delay();I2C_Delay();
}


//------------------------------------------------------------------------------------
//I2C Start Condition
//------------------------------------------------------------------------------------
void I2C_Start_B(void)
{
	I2C_Delay();I2C_Delay();
	SDA_B = 1;
	I2C_Delay();I2C_Delay();
	SCL_B = 1;
	I2C_Delay();I2C_Delay();
	SDA_B = 0;
	I2C_Delay();I2C_Delay();
	SCL_B = 0;
	I2C_Delay();I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Send Data
//------------------------------------------------------------------------------------
void I2C_Senddata_B(uchar senddata)
{
	uchar i;
	for (i=0;i<8;i++)
	{	
	 I2C_Delay();
	 if ((senddata & 0x80) != 0x80)
		SDA_B = 0;
	 else SDA_B = 1;
		senddata = senddata << 1;
		I2C_Delay();
		SCL_B = 1;
		I2C_Delay();
		SCL_B = 0;
	}
	I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Receive Data
//------------------------------------------------------------------------------------
uchar I2C_Receivedata_B(void)
{
	uchar i,temp,receivedata=0;
	for (i=0;i<8;i++)
		{
		 I2C_Delay();
		 SCL_B = 1;
		 I2C_Delay();
		 temp = SDA_B;
		 SCL_B = 0;
		 receivedata = receivedata | temp;
		 if (i<7)
			{
			 receivedata = receivedata << 1;
			}
		}
	I2C_Delay();
	return(receivedata);	
}
//------------------------------------------------------------------------------------
//I2C Ack
//------------------------------------------------------------------------------------
void I2C_Ack_B(void)
{
	SDA_B = 1;
	I2C_Delay();I2C_Delay();
	SCL_B = 1;
	I2C_Delay();
	Ack_Flag_B = SDA_B;
	SCL_B = 0;
	I2C_Delay();I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Stop Condition
//------------------------------------------------------------------------------------
void I2C_Stop_B(void)
{
	SCL_B = 0;
	I2C_Delay();I2C_Delay();
	SDA_B = 0;
	I2C_Delay();I2C_Delay();
	SCL_B = 1;
	I2C_Delay();I2C_Delay();
	SDA_B = 1;
	I2C_Delay();I2C_Delay();
}