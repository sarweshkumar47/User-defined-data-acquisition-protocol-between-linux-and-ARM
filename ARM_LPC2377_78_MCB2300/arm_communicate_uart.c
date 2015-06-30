/*Communication between a linux host machine and ARM LPC2377/78 Microncontroller based MCB2300 board 
* Implementation of custom protocol on a standard UART/RS-232 protocol
*
*-------------------------------------------------------------------------
*  Custom Protocol:
	Start bits--1B
	Identifier--1B
	Payload length--1B
	Mode (Read/Write)--1B
	Future use--4B
	User payload
	Stop bits--1B

* Example: fe|01|08|01|00000000|0102030405060708|ff

--------------------------------------------------------------------------
* Linux machine can read / write the data from / to MCB2300 ARM Microcontroller (LPC2377/78) 
* on standard UART by sending these control bytes to ARM Microcontroller
* ARM Microcontroller receives these control bytes and sends / receives the data according to the mode bits

* Write mode -- writes the data to output devices based on identifier
LED-----0
LCD-----1

* Read mode -- reads the data from ADC (Sensor / pot is attached)
ADC-----0 (default)

*/

#include "library.h"

unsigned char *pdata;

struct Header
{
	/*
	Start bits--1B
	Identifier--1B
	Payload length--1B
	Mode (Read/Write)--1B
	Future use--4B
	User payload (Depends on mode)
	Stop bits--1B
	*/
	unsigned char start_bits, identifier, length_payload, mode;	//For Header fields;
	unsigned char r1,r2,r3,r4;
	unsigned char stop_bits;

}head;

void delay(void)	//Delay for displaying the LEDs.
{
	unsigned char i;
	int j;
	for(i=0;i<100;i++)
		for(j=0; j<50000; j++);
}

int main()
{
	char i;
	int read;
	int ind1=0,ind2=0;
	   
	SerialInit();
	Init_GPIO();
	Init_ADC();
	LcdInit();
	LcdClear();

	//	FIO2PIN = 0x01;		Just to debug the code
	  
	while(1)
	{
		do
		{
	   		do
		  	{
				//	Storing header;
				do
				{
					//	printf("Entering do while\n");					
					//	FIO2PIN = 0x11;     Just to debug the code
					while((head.start_bits = getkey()) != 0xFE);	// Waiting for Start bits(0xFE) once again;	
					head.identifier = getkey();			// Storing header bytes;			
					head.length_payload = getkey();		
					head.mode = getkey();		
					head.r1 = getkey();
					head.r2 = getkey();
					head.r3 = getkey();
					head.r4 = getkey();
						
					if((head.identifier & 0xFC) != BID)		// Checking for a Board ID. If BID is error, send a NACK;
						sendchar(NACK);
							
	 			}while((head.identifier & 0xFC) != BID); 		// Control goes back to start, if BID error occurs;
			
				//	FIO2PIN = 0x22;       Just to debug the code
			        //	printf("checking mode error\n");			
				//	check for mode error;

				if(!((head.mode == 0x01) || (head.mode == 0x02)))	// Checking for mode error, if mode error occurs send NACK;
					sendchar(NACK);

		   	}while(!((head.mode == 0x01) || (head.mode == 0x02)));		// Control goes back to start if Mode error occurs;
			 
		   	//	FIO2PIN = 0x33;      Just to debug the code		  
			//	Dynamic memory allocation;
			pdata = malloc(head.length_payload * sizeof(char));

			if (pdata == NULL)
			{
			//	If any error in memory allocation, send NACK;
				free(pdata);
				sendchar(NACK);
		  	}
		}while(pdata == NULL);					//Control goes back to start if any error in memory allocation;

		//	printf("memory allocated\n");
		//	FIO2PIN = 0x44;     Just to debug the code

		switch(head.mode)					// Entering Switch case;
		{
			default:		// Default--- Write mode;
			{
				sendchar(0x0f);	// Send ACK after receiving the header correctly and  memory allocation is done;
				//	FIO2PIN = 0x55;		Just to debug the code
				for(i=0;i<head.length_payload;i++)	// Storing the data bytes in aloocated memory;
				{
					*(pdata + i) = (char) getkey();
				}
				//	FIO2PIN = 0x66;         Just to debug the code
				head.stop_bits = getkey();
				if(head.stop_bits != 0x01)		// Checking Stop Bits error;
				{
					sendchar(NACK);			// If any error in Stop bits, send NACK;
					//	printf("Stop bits error\t");
						free(pdata);		//Free the allocated memory if Stop Bits error occurs;
						ind1 = 1;
						break;	
				}
				//	printf("entering the switch\t");  
				if(ind1 != 1)
				{
					sendchar(ACK);			// Send ACK if everything is Perfect(including Stop bits);
					ind1 = 0;
					switch(head.identifier & 0x03)	// Checking Peripheral ID
					{						
						case 0:
							//	FIO2PIN = 0xee;			Just to debug the code
							device0_write();			// Write the data to the LED;
							free(pdata);				// free the memory after writting the data
							break;

						default:
							//	FIO2PIN = 0xbb;			Just to debug the code
							device1_write();			// Write the data to the LCD;
							free(pdata);				// free the memory after writting the data
							break;
					}
				}
				break;
					   
			}
					
			case 1:			// Case 1--- Read mode;		
			{
				sendchar(ACK);			// Send ACK after receiving the header correctly;
				//	printf("Read mode\t");
				//	FIO2PIN = 0x77;
				switch(head.identifier & 0x03)	//  Checking peripheral ID;
				{
					default:
						device0_read();							
				}
					
				for(i=0; i<head.length_payload; i++)	//Write the read data to UART from allocated memory;
				{
					read =  *(pdata + i);
					sendchar(read);
				}
				free(pdata);				// Free the data after sending the payload;
				//	FIO2PIN = 0x88;	
				head.stop_bits = getkey();		// Waiting for Stop bits from other end;
				if(head.stop_bits != 0x01)
				{
					sendchar(NACK);
					ind2 = 1;			// If any error in Stop bits send NACK;
					break;							
				}
				if(ind2 != 1)
				{
					ind2 = 0;
					sendchar(ACK);			// If everything is correct send ACK(including Stop bits);
				}
				break;
			 }				
		 }
		//	 printf("Exiting from switch\n");
	  }
  }

 
void device0_write(void)			// Writting the data to LED;
{
	char j;	
	for(j=0; j<head.length_payload; j++)
	{
		FIO2PIN =  *(pdata + j);
		delay();	
	}
 }

void device1_write(void)			// Writting the data to LCD(displays first 16 characters);
{
	unsigned char i=0,j=0,ch;
	LcdClear();
	for(j=0;( (j<8) && (i<head.length_payload));i++, j++)
	{		
		ch = *(pdata + j);
		//	LcdWriteData(ch);
		LCD_display(ch);
	}
		
	if(head.length_payload > 8)
	{
		LcdSetCursor (0,1);			
		for(j=8,i=8; ((i<16) && (j<head.length_payload)); i++,j++)
		{
			ch = *(pdata + i);
			//	LcdWriteData(ch);
			LCD_display(ch);
		}
	}	 
}

void LCD_display(unsigned char data)		//Converting Digital Data into ASCII 
{
	unsigned char res;
	unsigned char q,rem;
	if((data > 0x0F) && (data < 0x100))
	{
        	q = data/0x10;
        	rem = data % 0x10;
        	res = (char) q;
        	data = rem;

		if(res < 0x0a)
			res += 0x30;
		else
			res += 0x37;
	 LcdWriteData(res);
	}
	if(data < 0x10) 
	{
        	res = (char) data;		 
		if(res < 0x0a)
			res += 0x30;
		else
			res += 0x37;
	 	LcdWriteData(res);
	}       
}

void device0_read(void)				// Read the data from ADC in Read mode;
{
	char j;
	for(j=0;j<head.length_payload;j++)
	{
		AD0CR |= 0x01000000;
		while((AD0DR0 & (1 << 31))==0);
		*(pdata + j) = ((AD0DR0 >> 6) & 0xFF);
		//	printf("adc value1: %x ",*(pdata+j));	
	}
}

