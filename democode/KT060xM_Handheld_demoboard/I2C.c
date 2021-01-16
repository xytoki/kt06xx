//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <interface.h>										// SFR declarations
#include <intrins.h>
#include <stdio.h>

#ifdef I2C

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
bit Ack_Flag=0;													// I2C Ack Flag
//------------------------------------------------------------------------------------
// Function PROTOTYPES
//------------------------------------------------------------------------------------
void I2C_Delay(void);											// 扫键，LCD，I2C延时
void I2C_Start(void);
void I2C_Senddata(uchar senddata);
void I2C_Ack(void);
void I2C_Stop(void);
uchar I2C_Receivedata(void);

//------------------------------------------------------------------------------------
//I2C Write Byte
//------------------------------------------------------------------------------------
void I2C_Byte_Write(uchar device_address, uchar reg_add, uchar writedata)
{
	I2C_Start();
	I2C_Senddata(device_address & 0xFE);
	I2C_Ack();
	if (Ack_Flag == 0)
	{
		I2C_Senddata(reg_add);
		I2C_Ack();
		if (Ack_Flag == 0)
		{
			I2C_Senddata(writedata);
			I2C_Ack();
		}
		else
			SCL = 0;			
	}
	else
		SCL = 0;
	I2C_Stop();	
}

//------------------------------------------------------------------------------------
//I2C Read Byte
//------------------------------------------------------------------------------------
uchar I2C_Byte_Read(uchar device_address, uchar reg_add)
{
	uchar readdata;
	I2C_Start();
	I2C_Senddata(device_address & 0xFE);
	I2C_Ack();
	if (Ack_Flag == 0)	//7.12 edit
	{
		I2C_Senddata(reg_add);
		I2C_Ack();
		if (Ack_Flag == 0)	//7.12 edit
		{
			I2C_Start();
			I2C_Senddata(device_address | 0x01);
			I2C_Ack();
			if (Ack_Flag == 0)
			{
//	SDA pin is high Z
				readdata = I2C_Receivedata();
				I2C_Ack();
			}
			else
			{
				SCL=0;
				return(0x00);
			}
		}
		else
		{
			SCL = 0;
		 	return(0x00);
		}			
	}
	else
	{
		SCL = 0;
		return(0x00);
	}

	I2C_Stop();	
	return(readdata);
}

//------------------------------------------------------------------------------------
//I2C Write Word
//------------------------------------------------------------------------------------
void I2C_Word_Write(uchar device_address, uchar reg_add, uint writeword)
{
	uchar writeword_high,writeword_low;

	writeword_low 	=	writeword;
	writeword_high	=	writeword>>8;

	I2C_Start();
	I2C_Senddata(device_address & 0xFE);
	I2C_Ack();
	if (Ack_Flag == 0)
		{
		 I2C_Senddata(reg_add);
		 I2C_Ack();
		 if (Ack_Flag == 0)
			{
			 I2C_Senddata(writeword_high);
			 I2C_Ack();
			 if (Ack_Flag == 0)
				{
				 I2C_Senddata(writeword_low);
				 I2C_Ack();
				}
			 else SCL = 0;	
			}
		 else SCL = 0;			
		}
	else SCL = 0;
	I2C_Stop();	
}

//------------------------------------------------------------------------------------
//I2C Read Word
//------------------------------------------------------------------------------------
uint I2C_Word_Read(uchar device_address, uchar reg_add)
{
	unsigned char readdata_low;
	unsigned int readdata,readdata_high,temp=0;
	I2C_Start();
	I2C_Senddata(device_address & 0xFE);
	I2C_Ack();
	if (Ack_Flag == 0)
		{
		 I2C_Senddata(reg_add);
		 I2C_Ack();
		 if (Ack_Flag == 0)
			{
			 I2C_Start();
			 I2C_Senddata(device_address | 0x01);
			 I2C_Ack();
			 if (Ack_Flag == 0)
				{
//				 SDA = 1;//SDA 设为输入，读引脚
				 readdata_high = I2C_Receivedata();
				 SDA = 0;
				 I2C_Delay();I2C_Delay();
				 SCL = 1;
				 I2C_Delay();I2C_Delay();
				 SCL = 0;
				 I2C_Delay();I2C_Delay();
				 SDA = 1;

			 	 if (Ack_Flag == 0)
					{
					 readdata_low = I2C_Receivedata();
					 I2C_Ack();
					}
				 else
					{
					 SCL=0;
					 return(0x0000);
					}
				}
			 else
				{
				 SCL=0;
				 return(0x0000);
				}
			}
		 else
		 	{
			 SCL = 0;
		 	 return(0x0000);			
			}
		}
	else
		{
		 SCL = 0;
		 return(0x0000);
		}

	I2C_Stop();
		
	temp=readdata_high<<8;
	readdata=temp | readdata_low;
	return(readdata);
}

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
//I2C Start Condition
//------------------------------------------------------------------------------------
void I2C_Start(void)
{
	I2C_Delay();I2C_Delay();
	SDA = 1;
	I2C_Delay();I2C_Delay();
	SCL = 1;
	I2C_Delay();I2C_Delay();
	SDA = 0;
	I2C_Delay();I2C_Delay();
	SCL = 0;
	I2C_Delay();I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Send Data
//------------------------------------------------------------------------------------
void I2C_Senddata(uchar senddata)
{
	uchar i;
	for (i=0;i<8;i++)
	{	
	 I2C_Delay();
	 if ((senddata & 0x80) != 0x80)
		SDA = 0;
	 else SDA = 1;
		senddata = senddata << 1;
		I2C_Delay();
		SCL = 1;
		I2C_Delay();
		SCL = 0;
	}
	I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Receive Data
//------------------------------------------------------------------------------------
uchar I2C_Receivedata(void)
{
	uchar i,temp,receivedata=0;
	for (i=0;i<8;i++)
		{
		 I2C_Delay();
		 SCL = 1;
		 I2C_Delay();
		 temp = SDA;
		 SCL = 0;
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
void I2C_Ack(void)
{
	SDA = 1;
	I2C_Delay();I2C_Delay();
	SCL = 1;
	I2C_Delay();
	Ack_Flag = SDA;
	SCL = 0;
	I2C_Delay();I2C_Delay();
}
//------------------------------------------------------------------------------------
//I2C Stop Condition
//------------------------------------------------------------------------------------
void I2C_Stop(void)
{
	SCL = 0;
	I2C_Delay();I2C_Delay();
	SDA = 0;
	I2C_Delay();I2C_Delay();
	SCL = 1;
	I2C_Delay();I2C_Delay();
	SDA = 1;
	I2C_Delay();I2C_Delay();
}

#endif