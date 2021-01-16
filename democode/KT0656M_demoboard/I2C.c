//*****************************************************************************
//  File Name: I2C.c
//  Function:  KT Wireless Mic Transmitter Products Demoboard I2C Function Define
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V1.0    2012-08-01  Initial draft
//  V1.1    2017-02-10  规范化整理
//  V1.4N   2017-04-01  根据当前是主路或者从路显示相对应的RSSI及SNR,pilot及BPSK值(Main.c)
//						把I2C的速度改快了(I2C.c)
//						根据最新的命名规则进行了版本号修改
//  V1.5     2017-04-21 在主循环里面一直读取导频和SNR的值，并根据最新的值决定是否mute(Main.c)
//  V1.6     2017-06-05 原来读写电池电压的寄存器为0x029,应该为0x0249
//						把rfIntCtl();pilotMuteRefresh();snrMuteRefresh();移到了驱动文件(Main.c)
//  V1.7     2017-06-28 增加了BATTERY_Display函数，用来显示接收机的电池电压(Main.c)
//*****************************************************************************

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <interface.h>    
#include <intrins.h>
#include <stdio.h>
#include <I2C.h>

//-----------------------------------------------------------------------------
// Global VARIABLES
//-----------------------------------------------------------------------------
bit Ack_Flag=0; // I2C Ack Flag

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

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
//    UINT8 i;
    
//    for(i=0;i<=1;i++)
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

//-----------------------------------------------------------------------------
//函 数 名：I2C_Byte_Write
//功能描述：I2C按Byte写操作
//函数说明：寄存器地址范围为16位
//全局变量：无
//输    入：uchar device_address,UINT16 reg_add,uchar writedata
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void I2C_Byte_Write(UINT8 device_address, UINT16 reg_add, UINT8 writedata)
{
    UINT8 add_H,add_L;
    
    add_H = reg_add>>8;
    add_L = reg_add & 0x00FF;

    I2C_Start();
    I2C_Senddata(device_address & 0xFE);
    I2C_Ack();
    if (Ack_Flag == 0)
    {
        I2C_Senddata(add_H);
        I2C_Ack();
        if (Ack_Flag == 0)
        {
            I2C_Senddata(add_L);
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
    }
    else
        SCL = 0;
    I2C_Stop();    
}

//-----------------------------------------------------------------------------
//函 数 名：I2C_Byte_Read
//功能描述：I2C按Byte读操作
//函数说明：寄存器地址范围为16位
//全局变量：无
//输    入：uchar device_address,UINT16 reg_add
//返    回：正确：uchar readdata    错误：0x00
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 I2C_Byte_Read(UINT8 device_address, UINT16 reg_add)
{
    UINT8 readdata;
    UINT8 add_H,add_L;
    
    add_H = reg_add>>8;
    add_L = reg_add & 0x00FF;

    I2C_Start();
    I2C_Senddata(device_address & 0xFE);
    I2C_Ack();
    if (Ack_Flag == 0)
    {
        I2C_Senddata(add_H);
        I2C_Ack();
        if (Ack_Flag == 0)
        {
            I2C_Senddata(add_L);
            I2C_Ack();
            if (Ack_Flag == 0)
            {
                I2C_Start();
                I2C_Senddata(device_address | 0x01);
                I2C_Ack();
                if (Ack_Flag == 0)
                {
                    // SDA pin is high Z
                    readdata = I2C_Receivedata();
                    I2C_Ack();
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

//-----------------------------------------------------------------------------
//函 数 名：I2C_Byte_Write
//功能描述：I2C按Byte写操作
//函数说明：
//全局变量：无
//输    入：uchar device_address,UINT8 reg_add,uchar writedata
//返    回：无
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
void I2S_Byte_Write(UINT8 device_address, UINT8 reg_add, UINT8 writedata)
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
//输    入：uchar device_address,UINT8 reg_add
//返    回：正确：uchar readdata    错误：0x00
//设 计 者：PAE                     时间：2012-08-01
//修 改 者：                        时间：
//版    本：V1.0
//-----------------------------------------------------------------------------
UINT8 I2S_Byte_Read(UINT8 device_address, UINT8 reg_add)
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
                // SDA pin is high Z
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

