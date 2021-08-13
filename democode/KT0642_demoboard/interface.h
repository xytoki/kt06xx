//#define C8051F314
#define C8051F340

#ifdef C8051F314

	#include <c8051f310.h>										// SFR declarations

#endif

#ifdef C8051F340

	#include <c8051f340.h>										// SFR declarations

#endif

#define uchar unsigned char
#define ulong unsigned long
#define uint unsigned int

//#define SPI3
#define I2C

#define DEMO_RX
//#define DEMO_TX

#ifdef C8051F314
sbit SDA = P0^4;											// 分配P1.0给SDA
sbit SCL = P0^5;											// 分配P1.0给SCL
sbit GPIO3 = P0^6;
sbit GPIO2 = P0^7;
sbit GPIO1 = P0^1;											// Slave output, master input
sbit SW_FM = P3^1;											// SWITCH_1='1' means switch pressed
sbit SCK = P0^0;											// SPI clock
sbit MOSI = P0^2;											// Master output, slave input
sbit NSS = P0^3;											// SPI select
#endif

#ifdef C8051F340
sbit SDA = P3^0;											// 分配P3.0给SDA
sbit SCL = P2^7;											// 分配P2.7给SCL

sbit GPIO3 = P2^5;
sbit GPIO2 = P2^4;
sbit GPIO1 = P2^3;											// Slave output, master input
sbit SW_FM = P3^1;											// SWITCH_1='1' means switch pressed
sbit SCK = P3^2;											// SPI clock
sbit MOSI = P3^3;											// Master output, slave input
sbit NSS = P2^6;											// SPI select
#endif


#ifdef SPI3	
void SPI_Write (uchar Reg_addr, uint spi_data);
uint SPI_Read_3wire (uchar Reg_addr);
#endif

#ifdef I2C
void I2C_Word_Write(uchar device_address, uchar reg_add, uint writeword);
uint I2C_Word_Read(uchar device_address, uchar reg_add);
void I2C_Byte_Write(uchar device_address, uchar reg_add, uchar writedata);
uchar I2C_Byte_Read(uchar device_address, uchar reg_add);
#endif

//------------------------------------------------------------------------------------
//LCD控制引脚定义
//------------------------------------------------------------------------------------
#ifdef C8051F314
sbit LCD_RS = P3^2;											// RS=1：数据； RS=0：指令
sbit LCD_RW = P3^3;											// RW=1：读； RW=0：写
sbit LCD_Enalbe = P3^4;										// L：允许；H：禁止
sbit LCD_Reset = P1^7;										// LCD_Reset ='1' means LCD reset
sbit LCD_BF = P2^0;											// 忙信号检查
sfr LCDInterface = 0xA0;									// P2
#endif

#ifdef C8051F340
sbit LCD_RS = P2^2;											// RS=1：数据； RS=0：指令
sbit LCD_RW = P2^1;											// RW=1：读； RW=0：写
sbit LCD_Enalbe = P2^0;										// L：允许；H：禁止
sbit LCD_Reset = P0^7;										// LCD_Reset ='1' means LCD reset
sbit LCD_BF = P1^0;											// 忙信号检查
sfr LCDInterface = 0x90;									// P1
#endif
//------------------------------------------------------------------------------------
//KEY引脚定义
//------------------------------------------------------------------------------------
#ifdef C8051F314
sfr KeyInterface = 0x90;									// P1
#endif

#ifdef C8051F340
sfr KeyInterface = 0x80;									// P0
#endif

//------------------------------------------------------------------------------------
//LCD显示部分函数定义
//------------------------------------------------------------------------------------
void LCD_Init(void);										// LCD显示初始化子程序//
void BUSY_Check(void);										// 检测忙信号子程序
void DISPLAY_Start(void);									// 显示欢迎画面
void Display1(void);										// 第一行显示子程序
void Display2(void);										// 第一行显示子程序
void Display3(void);										// 第三行显示子程序
void Display4(void);										// 第四行显示子程序
void CODE_Send(uchar data command);							// 命令发送子程序
void DATA_Send(uchar data mydata);							// 数据发送子程序
void DELAY_Write(void);										// 写入延时
void DELAY_Display(void);									// 显示欢迎画面延时
uchar BYTE_Reversal(uchar Reversal_data);					// 1字节的8为反转程序
void LCD_Number_Cal (uint LCD_Number);
void LCD_RSSI_Display (uchar RSSI_Number);
uchar HexToASCII(uint num);
uchar KEY_Scan (void);
void dlms(void);											// 键盘扫描去抖动延时程序
void delay(void);											// 扫键，LCD延时
void Display_Channel (uint LCD_Number);
void Delay_ms(uint time_number);

