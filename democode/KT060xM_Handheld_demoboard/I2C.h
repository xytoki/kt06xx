#define uchar unsigned char
#define ulong unsigned long
#define uint unsigned int

#define kt0810w_address 0x6E								// kt0810写地址
#define kt0810r_address 0x6F								// kt0810读地址

void I2C_Word_Write(uchar device_address, uchar reg_add, uint writeword);
uint I2C_Word_Read(uchar device_address, uchar reg_add);

void KT_Bus_Write(uchar Register_Address, uint Word_Data);
uint KT_Bus_Read(uchar Register_Address);


sbit SDA = P0^4;											// 分配P1.0给SDA
sbit SCL = P0^5;											// 分配P1.0给SCL

sbit NSS = P0^3;
sbit GPIO3 = P0^6;
sbit GPIO2 = P0^7;
sbit GPIO1 = P0^1;											// Slave output, master input

sbit SW_FM = P3^1;											// SWITCH_1='1' means switch pressed