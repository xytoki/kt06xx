//*****************************************************************************
//  File Name: SH88F4051.h
//  Function:  Header file for SH88F4051 microcontroller
//*****************************************************************************
//        Revision History
//  Version        Date                    Description
//  V1.0        2016-04-26                ≥ı º∞Ê±æ
//*****************************************************************************

#ifndef REGSH88F4051_H
#define REGSH88F4051_H

/* ------------------- BYTE Register-------------------- */
/* CPU */            
sfr ACC = 0xE0;
sfr B = 0xF0;
sfr AUXC = 0xF1;
sfr PSW = 0xD0;
sfr SP = 0x81;
sfr DPL = 0x82;
sfr DPH = 0x83;
sfr DPL1 = 0x84;
sfr DPH1 = 0x85;
sfr INSCON   = 0x86;

/* Power  */
sfr PCON  = 0x87;
sfr SUSLO = 0x8E;

/* Clock */
sfr CLKCON = 0xB2;

/* FLASH */
sfr XPAGE = 0xF7;
sfr IB_OFFSET = 0xFB;
sfr IB_DATA = 0xFC;
sfr IB_CON1 = 0xF2;
sfr IB_CON2 = 0xF3;
sfr IB_CON3 = 0xF4;
sfr IB_CON4 = 0xF5;
sfr IB_CON5 = 0xF6;
sfr FLASHCON = 0xA7;

/* WDT */
sfr RSTSTAT = 0xB1;

/* Interrupt */
sfr IEN0= 0xA8;
sfr IEN1 = 0xA9;
sfr EXF0 = 0xE8;
sfr IPL0 = 0xB8;
sfr IPL1 = 0xB9;
sfr IPH0 = 0xB4;
sfr IPH1 = 0xB5;

/* PORT */
sfr P1 = 0x90;
sfr P3 = 0xB0;
sfr P4 = 0xC0;
sfr P1M0 = 0xEA;
sfr P1M1 = 0xE2;
sfr P3M0 = 0xEC;
sfr P3M1 = 0xE4;
sfr P4M0 = 0xED;
sfr P4M1 = 0xE5;

/* TIMER 0/1 */
sfr TCON = 0x88;
sfr TMOD = 0x89;
sfr TL0  = 0x8A;
sfr TL1  = 0x8B;
sfr TH0  = 0x8C;
sfr TH1  = 0x8D;
sfr T2CON = 0xC8;
sfr T2MOD = 0xC9;
sfr RCAP2L  = 0xCA;
sfr RCAP2H  = 0xCB;
sfr TL2  = 0xCC;
sfr TH2  = 0xCD;
sfr TCON1 = 0xCE;

/* EUART 0 */
sfr SCON = 0x98;
sfr SBUF = 0x99;
sfr SADDR = 0x9A;
sfr SADEN = 0x9B;
        
/* ADC */
sfr ADCON = 0x93;
sfr ADT = 0x94;
sfr ADCH = 0x95;
sfr ADDL = 0x96;
sfr ADDH = 0x97;

/*CMP*/
sfr CMPCON = 0x92;

/*LPD*/
sfr LPDCON = 0xB3;

/* PWM */
sfr PWMCON = 0xD1;
sfr PWMP = 0xD2;
sfr PWMD = 0xD3;

/*--------------------------  BIT Register -------------------- */
/*  PSW   */
sbit CY = 0xD7;
sbit AC = 0xD6;
sbit F0 = 0xD5;
sbit RS1 = 0xD4;
sbit RS0 = 0xD3;
sbit OV = 0xD2;
sbit F1 = 0XD1;
sbit P = 0xD0;

/*  T2CON  */
sbit TF2 = 0xCF;
sbit EXF2 = 0xCE;
sbit RCLK = 0xCD;
sbit TCLK = 0xCC;
sbit EXEN2 = 0xCB;
sbit TR2 = 0xCA;
sbit C_T2 = 0xC9;
sbit CP_RL2 = 0xC8;

/*  EXF0  */
sbit IE2 = 0xE8;
sbit IT20 = 0xEA;
sbit IT21 = 0xEB;

/*  IPL0   */ 
sbit PADCL = 0xBE;
sbit PT2L = 0xBD;
sbit PSL = 0xBC;
sbit PT1L = 0xBB;
sbit PX1L = 0xBA;
sbit PT0L = 0xB9;
sbit PX0L = 0xB8;

/*  IEN0   */
sbit EA = 0xAF;
sbit EADC = 0xAE;
sbit ET2 = 0xAD;
sbit ES0 = 0xAC;
sbit ET1 = 0xAB;
sbit EX1 = 0xAA;
sbit ET0 = 0xA9;
sbit EX0 = 0xA8;

/*  SCON  */
sbit SM0_FE = 0x9F;
sbit SM1_RXOV = 0x9E;
sbit SM2_TXCOL = 0x9D;
sbit REN = 0x9C;
sbit TB8 = 0x9B;
sbit RB8 = 0x9A;
sbit TI = 0x99;
sbit RI = 0x98;

/*  TCON  */
sbit TF1 = 0x8F;
sbit TR1 = 0x8E;
sbit TF0 = 0x8D;
sbit TR0 = 0x8C;
sbit IE1 = 0x8B;
sbit IT1 = 0x8A;
sbit IE0 = 0x89;
sbit IT0 = 0x88;

/* P1 */
sbit P1_0 = P1^0;
sbit P1_1 = P1^1;
sbit P1_2 = P1^2;
sbit P1_3 = P1^3;
sbit P1_4 = P1^4;
sbit P1_5 = P1^5;
sbit P1_6 = P1^6;
sbit P1_7 = P1^7;

/* P3 */
sbit P3_0 = P3^0;
sbit P3_1 = P3^1;
sbit P3_2 = P3^2;
sbit P3_3 = P3^3;
sbit P3_4 = P3^4;
sbit P3_5 = P3^5;
sbit P3_7 = P3^7;

/* P4 */
sbit P4_0 = P4^0;
sbit P4_1 = P4^1;
sbit P4_2 = P4^2;

#endif

