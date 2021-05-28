//*****************************************************************************
//  File Name: I2C.c
//  Function:  KT Wireless Mic Transmitter Products Demoboard I2C Function Define
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V1.0    2012-08-01  Initial draft
//  V1.1    2013-11-22  改变变量类型定义方法
//  V1.2    2017-02-08  格式规范化整理
//  V0.3    2017-04-27  加减频率的时候，先把导频关了，tune完台后再恢复原来导频的设置（main.c）
//						调台中一直以相同的步进进行tune台，不会调一段时间后步进变成4倍和10倍（main.c）
//*****************************************************************************

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <interface.h> 
#include <intrins.h>

#ifdef I2C

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
BOOL Ack_Flag=0; // I2C Ack Flag

#ifdef I2C_BYTE_MODE //I2C按Byte操作模

//-----------------------------------------------------------------------------
//函 数 名：I2C_Byte_Write
//功能描述：I2C按Byte写操作
//函数说明：
//全局变量：无
//输    入：uchar device_address,uchar reg_add,uchar writedata
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void I2C_Byte_Write(UINT8 device_address, UINT8 reg_add, UINT8 writedata)
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

//-----------------------------------------------------------------------------
//函 数 名：I2C_Byte_Read
//功能描述：I2C按Byte读操作
//函数说明：
//全局变量：无
//输    入：uchar device_address,uchar reg_add
//返    回：正确：uchar readdata    错误：0x00
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 I2C_Byte_Read(UINT8 device_address, UINT8 reg_add)
{
    UINT8 readdata;
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
                //    SDA pin is high Z
                readdata = I2C_Receivedata();
                I2C_Ack();
                I2C_Stop();    
                return(readdata);
            }
            else
            {
                I2C_Stop();
                return(0x00);
            }
        }
        else
        {
            I2C_Stop();
             return(0x00);
        }            
    }
    else
    {
        I2C_Stop();
        return(0x00);
    }
}
#endif


#ifdef I2C_WORD_MODE //I2C按WORD操作模式

//-----------------------------------------------------------------------------
//函 数 名：I2C_Word_Write
//功能描述：I2C按WORD写操作
//函数说明：
//全局变量：无
//输    入：uchar device_address, uchar reg_add, uint writeword
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void I2C_Word_Write(UINT8 device_address, UINT8 reg_add, UINT16 writeword)
{
    UINT8 writeword_high,writeword_low;

    writeword_low = writeword;
    writeword_high = writeword>>8;

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
            else
                SCL = 0;    
        }
        else
            SCL = 0;            
    }
    else
        SCL = 0;
    I2C_Stop();    
}

//-----------------------------------------------------------------------------
//函 数 名：I2C_Word_Read
//功能描述：I2C按Word读操作
//函数说明：
//全局变量：无
//输    入：uchar device_address,uchar reg_add
//返    回：正确：UINT16 readdata    错误：0x0000
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT16 I2C_Word_Read(UINT8 device_address, UINT8 reg_add)
{
    UINT8 readdata_low;
    UINT16 readdata,readdata_high,temp=0;

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
                // SDA = 1;//SDA 设为输入，读引脚
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
                    I2C_Stop();
        
                    temp = readdata_high << 8;
                    readdata = temp | readdata_low;
                    return(readdata);
                }
                else
                {
                    I2C_Stop();
                    return(0x0000);
                }
            }
            else
            {
                I2C_Stop();
                return(0x0000);
            }
        }
        else
        {
            I2C_Stop();
             return(0x0000);            
        }
    }
    else
    {
        I2C_Stop();
        return(0x0000);
    }
}
#endif

//-----------------------------------------------------------------------------
//函 数 名：I2C_Delay
//功能描述：I2C延时
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void I2C_Delay(void)
{
    UINT8 i;

    for(i=0;i<=40;i++)
    {
        _nop_();
    }
}

//-----------------------------------------------------------------------------
//函 数 名：I2C_Start
//功能描述：I2C数据帧开始
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//函 数 名：I2C_Senddata
//功能描述：I2C发送数据
//函数说明：
//全局变量：无
//输    入：uchar senddata
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void I2C_Senddata(UINT8 senddata)
{
    UINT8 i;

    for (i=0;i<8;i++)
    {    
        I2C_Delay();
        if ((senddata & 0x80) != 0x80)
            SDA = 0;
        else
            SDA = 1;
        senddata = senddata << 1;
        I2C_Delay();
        SCL = 1;
        I2C_Delay();
        SCL = 0;
    }
    I2C_Delay();
}

//-----------------------------------------------------------------------------
//函 数 名：I2C_Receivedata
//功能描述：I2C接收数据
//函数说明：
//全局变量：无
//输    入：无
//返    回：uchar receivedata
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 I2C_Receivedata(void)
{
    UINT8 i,temp,receivedata=0;

    for (i=0;i<8;i++)
    {
        receivedata = receivedata << 1;
        I2C_Delay();
        SCL = 1;
        I2C_Delay();
        temp = SDA;
        SCL = 0;
        receivedata = receivedata | temp;
    }
    I2C_Delay();
    return(receivedata);    
}

//-----------------------------------------------------------------------------
//函 数 名：I2C_Ack
//功能描述：I2C_Ack
//函数说明：
//全局变量：Ack_Flag
//输    入：无
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//函 数 名：I2C_Stop
//功能描述：I2C数据帧结束
//函数说明：
//全局变量：无
//输    入：无
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
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

