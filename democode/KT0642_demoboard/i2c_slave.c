#include <interface.h>

#define MyAddress  0xAE

unsigned char xdata buffer_reg[192];

//here for demo.
void LCD_show (void);
bit  dataOk =0;

//-----------------------------------------------------------------------------
// IIC Slave Routine
//-----------------------------------------------------------------------------
bit W_R_Mark;	   //w=0; r=1;
bit last_SDA;	   

unsigned char lastBuffer;
unsigned char StartAddress;
unsigned char index_w = 0;
unsigned char index_r = 0;

char IICState = 0;	 
char MWriteState = 0;
char bit_WState = 0;
char MReadState = 0;
char bit_RState = 0;	

unsigned char bdata IICBuffer;
sbit bt7 = IICBuffer^7;
sbit bt6 = IICBuffer^6;
sbit bt5 = IICBuffer^5;
sbit bt4 = IICBuffer^4;
sbit bt3 = IICBuffer^3;
sbit bt2 = IICBuffer^2;
sbit bt1 = IICBuffer^1;
sbit bt0 = IICBuffer^0;

void IIC_Slave_Task(void)
{


	switch (IICState)                             				
	{
		
	case 0x0:  //IICState = Idle
		{
			bit SD = SDA;
			bit SC = SCL;
			static  char Idle_s = 0;
			if(Idle_s == 0)
			{
				if(SD&SC)
					Idle_s = 1;
			}
			else
			{
				if((SD == 0)&SC)
				{
					Idle_s = 0;
					IICState = 1; //start come.
				}
				else if(SCL == 0)
				{
					Idle_s = 0;
				}
			}
			break;			
		}
		
	case 0x1: //IICState = start & address
		{ 
			static  char start_s = 0;
			static  char bit_s = 7;							
			switch (start_s)                             				
			{
			case 0x0:  
				{
					if(SCL == 0)
						start_s = 1;
					break;
				}
			case 0x1:  
				{
					if(SCL == 1)
					{
						
						start_s = 0;
						
						if(bit_s == 7)
						{
							bt7 = SDA;
							bit_s = 6;
						}
						else if(bit_s == 6)
						{
							bt6 = SDA;
							bit_s = 5;
						}
						else if(bit_s == 5)
						{
							bt5 = SDA;
							bit_s = 4;
						}
						else if(bit_s == 4)
						{
							bt4 = SDA;
							bit_s = 3;
						}
						else if(bit_s == 3)
						{
							bt3 = SDA;
							bit_s = 2;
						}
						else if(bit_s == 2)
						{
							bt2 = SDA;
							bit_s = 1;
						}
						else if(bit_s == 1)
						{
							bt1 = SDA;
							bit_s = 0;
						}
						
						else if(bit_s == 0)
						{
							bt0 = SDA;
							bit_s = 7;
							
							if(MyAddress == (IICBuffer&0xfe))
							{
								start_s = 3;
								W_R_Mark =  bt0;
							}
							else
							{
								IICState = 0;	 //not our address
								start_s = 0;
							}
						}
					}
					break;
				}
			case 0x3: //send ACk 
				{
					if(SCL == 0)
					{
						start_s = 4;
						SDA = 0;
					}
					break;
				}
			case 0x4:  //ACk keep
				{
					if(SCL == 1)
					{
						if(W_R_Mark) //master read
						{
							IICState = 6; //to read	
							index_r = 0;
							start_s = 0;
							bit_RState = 7;
							MReadState = 0;
							IICBuffer = buffer_reg[StartAddress];
						}
						else
							start_s = 5;   //master write
					}
					break;
				}
			case 0x5:  //ACk finish
				{
					if(SCL == 0)
					{
						SDA = 1;
						start_s = 0;							
						{//if(W_R_Mark==0)				
							IICState = 2; //to write task
							index_w = 0;
						}
					}
					break;
				}
			}//End	switch (start_s) 
			break;			
		}

	case 0x2: //IICState = prepare get data, store first bit.   
		{			
			bit SD ;
			bit SC ;			   	
			SC = SCL;
			SD = SDA;
			if(SC == 1)
			{ 
				last_SDA = SD;
				bt7 = SD;
				IICState = 4;
			}
			break;
		}

	case 0x4: //IICState = deduce some kind condition coming. 
		{			
			bit SD ;
			bit SC ;
			
			SD = SDA;							   	
			SC = SCL;
			if(SC == 0)
			{ 
				IICState = 5;  //to get other bit.
				MWriteState = 1;
				bit_WState = 6;
			}
			else 
			{
				if(last_SDA&(SD==0))
					IICState = 1;					  //restart
				else if((last_SDA==0)&SD)
					IICState = 0;					  //  stop
				//else; // do not act , wait.
			}
			
		}break;
		
	case 0x5: //IICState = master write
		{
			bit SD ;
			bit SC ;			   	
			SC = SCL;
			SD = SDA;
			
			switch (MWriteState)                             				
			{
			case 0x0:  
				{
					if(SC == 0)
						MWriteState = 1;
					break;
				}
			case 0x1:  
				{
					if(SC == 1)
					{
						
						MWriteState = 0;

//						if(bit_WState == 7)
//						{
//						  bt7 = SD;
//						  bit_WState = 6;
//						}
//						else
						if(bit_WState == 6)
						{
							bt6 = SD;
							bit_WState = 5;
						}
						else if(bit_WState == 5)
						{
							bt5 = SD;
							bit_WState = 4;
						}
						else if(bit_WState == 4)
						{
							bt4 = SD;
							bit_WState = 3;
						}
						else if(bit_WState == 3)
						{
							bt3 = SD;
							bit_WState = 2;
						}
						else if(bit_WState == 2)
						{
							bt2 = SD;
							bit_WState = 1;
						}
						else if(bit_WState == 1)
						{
							bt1 = SD;
							bit_WState = 0;
						}
						
						else if(bit_WState == 0)
						{
							bt0 = SD;
							MWriteState = 3;
						}
					}
					break;
				}
			case 0x3: //MWriteState =ACk 
				{
					if(SC == 0)
					{
						SDA = 0;
						MWriteState = 4;
					}
					break;
				}
			case 0x4:  ////MWriteState =ACk keep
				{
					if(SC == 1)
					{
						MWriteState = 5;
					}
					break;
				}
			case 0x5:  //MWriteState =ACk finish
				{
					if(SC == 0)	  
					{												
						if(index_w)
							buffer_reg[StartAddress + index_w - 1] = IICBuffer; 
						else
							StartAddress = IICBuffer;
						
						index_w ++;
						if(index_w > 0x80)	//here is for rom and display, not for iic.
//						if(index_w > 0xC0)	//here is for rom and display, not for iic.
						{
							dataOk = 1;	
							LCD_show ();							
						}
						MWriteState = 1;
						IICState = 2;
						SDA = 1;
					}
					break;
				}
			}			
			break;
		}	//IICState = 5
		
	case 0x6: //IICState = master read
		{			
			bit SC ;			   	
			SC = SCL;
			
			switch (MReadState)                             				
			{
			case 0x0:  
				{
					if(SC == 0)
					{						 	
						MReadState = 1;
						
						if(bit_RState == 7)
						{
							SDA = bt7;
							bit_RState = 6;
						}
						else if(bit_RState == 6)
						{
							SDA = bt6;
							bit_RState = 5;
						}
						else if(bit_RState == 5)
						{
							SDA = bt5;
							bit_RState = 4;
						}
						else if(bit_RState == 4)
						{
							SDA = bt4;
							bit_RState = 3;
						}
						else if(bit_RState == 3)
						{
							SDA = bt3;
							bit_RState = 2;
						}
						else if(bit_RState == 2)
						{
							SDA = bt2;
							bit_RState = 1;
						}
						else if(bit_RState == 1)
						{
							SDA = bt1;
							bit_RState = 0;
						}
						else if(bit_RState == 0)
						{
							SDA = bt0;
							bit_RState = 7;
							MReadState = 2;
						}
					}
					break;
				}
			case 0x1:  
				{
					if(SC)
						MReadState = 0;
					break;
				}
			case 0x2:  	 //seek master's ack
				{
					if(SC)
						MReadState = 3;
					break;
				}
			case 0x3:  	 //seek master's ack
				{
					if(SC==0)
					{
						SDA = 1; //SDA must release.
						MReadState = 4;
					}
					break;
				}
			case 0x4:  	 //seek master's ack
				{
					if(SC)//the 9th plus
					{
						if(SDA)	 //no ack
						{
							IICState = 0;
						}
						else //ack come	//repeat read data
						{	 
							index_r++;
							IICBuffer = buffer_reg[(StartAddress + index_r)]; 
						}
						
						MReadState = 0;
					}
					
					break;
				}
				
			}//End 	switch (MReadState)

			break;
		}//End 	IICState =  case =6;
				
	}//End switch (IICState)		
}
