//*****************************************************************************
//  File Name: I2C.H
//  Function:  KT Wireless Mic Transmitter Products Demoboard I2C Function Define
//*****************************************************************************
//        Revision History
//  Version Date        Description
//  V1.0    2012-08-01  Initial draft
//  V1.1    2017-02-10  规范化整理
//*****************************************************************************
#ifndef _I2C_H_
#define _I2C_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "interface.h"

//-----------------------------------------------------------------------------
//I2C引脚定义
//-----------------------------------------------------------------------------
sbit SDA = P2^7; // 分配P2.7给SDA
sbit SCL = P3^0; // 分配P3.1给SCL

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
UINT8 I2C_Byte_Read(UINT8 device_address, UINT16 reg_add);
void I2C_Byte_Write(UINT8 device_address, UINT16 reg_add, UINT8 writedata);

UINT8 I2S_Byte_Read(UINT8 device_address, UINT8 reg_add);
void I2S_Byte_Write(UINT8 device_address, UINT8 reg_add, UINT8 writedata);

#endif

