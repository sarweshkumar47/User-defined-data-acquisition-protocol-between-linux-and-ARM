#include <LPC23xx.H>
#include <stdio.h>
#include <stdlib.h>


#define BID 0x00
#define ACK 0x0F
#define NACK 0xF0



void SerialInit(void);
void Init_GPIO(void);
void Init_ADC(void);
void LcdInit (void);
void LcdClear (void);
void LcdWriteData (unsigned char);
void LcdSetCursor (unsigned char column, unsigned char line);
void LCD_display(unsigned char data);
void device0_read(void);
void device0_write(void);
void device1_write(void);
int sendchar (int);
int getkey (void);




void Init_GPIO(void)
{
	PINSEL4 = 0x00000000;
	PINMODE4 = 0x00000000;
	FIO2DIR = 0x000000FF;
	FIO2PIN = 0x00000000;
}

void Init_ADC(void)
{
	PCONP|=(1<<12);
	PINSEL1|=0x00004000;
	AD0CR = 0x00240301;
}



		
	

