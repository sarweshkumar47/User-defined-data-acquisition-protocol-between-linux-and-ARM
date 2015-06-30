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

* Crerate a hex data file according to the above described prrotocol
* frame is the filename
--------------------------------------------------------------------------
$ echo fe010801000000000102030405060708ff | xxd -r -p > frame
$ hexdump -C frame
--------------------------------------------------------------------------
* Linux machine can read / write the data from / to MCB2300 ARM Microcontroller (LPC2377/78) 
* on standard UART by sending these control bytes to ARM Microcontroller

* Write mode -- writes the data to output devices on ARM Microcontroller
LED-----0
LCD-----1

* Read mode -- reads the data from ARM Microcontroller
ADC-----0 (default)  (Sensor / pot is attached)

*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>


#define BUFSIZE 100
#define FLAG O_RDWR
#define F_FLAG O_TRUNC | O_RDWR

struct data
{	
	char enter;
	unsigned char array[100];
	unsigned char ack[1];
	unsigned char stop[1];	
}dt;

int main(int argc,char *argv[])
{
	int fdwr1,fdwr2;					//fdwr1 & fdwr2 are file descriptors;
	int fread;
	char r,i;
	unsigned char fdata[BUFSIZE];				//fdata is buffer;
	      
	dt.stop[0] = 0x01;

	if(argc < 3)
	{
		printf("ERROR Usage: %s <wrFile>\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	
	if((fdwr1 = open(argv[1],F_FLAG))== -1)			//Open ttyS0 file;
	{							//fdwr1-- file descriptor for ttyS0 file;
		perror("ERROR open()");
		exit(EXIT_FAILURE);
	}


	if((fdwr2 = open(argv[2],FLAG))== -1)			//Open header file (contains hex binary values);
	{							//fdwr2-- file descriptor for header file;
		perror("ERROR open()");
		exit(EXIT_FAILURE);
	}
	
	while(1)
	{
		do
		{	
			printf("\nPress Enter\n");			//Program is waiting for user input;
			dt.enter = getchar();
			lseek(fdwr2,0,SEEK_SET);			//Setting cursor position to 0(at the start of header;
			if((r=read(fdwr2,fdata,8))== -1)		//Read first 8 bytes(header);
			{
				perror("ERROR read()");
				exit(EXIT_FAILURE);	
			} 	
			printf("\nr=%d\n",r);				//Prints no.of bytes read;

			for(i=0;i<8;i++)				//Storing the read contents in array;
			{		
				*(dt.array+i) = *(fdata+i);
				printf(" %x\t", *(dt.array+i));
			}		
	
			if((write(fdwr1,fdata,8)) == -1)		//Writting header bytes to ttyS0 file;
			{
				perror("ERROR write");
				exit(EXIT_FAILURE);
			}
		
			while((read(fdwr1,dt.ack,1)) == 0);		//Waiting for ACK;		
			printf("\ncontent of ack:%x\n",dt.ack[0]);
			if(dt.ack[0] == 0x0F)				//If ACK != 0x0f
				printf("\nACK received\n");		//Prints error in communication;
			else 
				printf("\nError in communication\n");	
		} while(dt.ack[0] != 0x0F);				//If ACK != 0x0f go back to the start; 

		printf("\nEntering the switch\n");			//If ACK arrives correctly, control enters into switch statement;	
		
		switch(dt.array[3])					// Read or Write operations based on mode bits
		{
			default :					//default--Write mode;
			{
				lseek(fdwr2,8,SEEK_SET);		//Setting the file descriptor to after header bytes(after 8 bytes);

				if((r=read(fdwr2,fdata,(dt.array[2]+1)))== -1)	//Read the data bytes a/c to length of Payload field+ stop bits;
				{
					perror("ERROR read()");
					exit(EXIT_FAILURE);	
				} 
				
				printf("\nWrite data:\n");
				
				for(i=0;i<(dt.array[2]+1);i++)
				{		
					printf(" %x\t", *(fdata+i));
				}

				if((write(fdwr1,fdata,(dt.array[2]+1))) == -1)	//Write the data bytes a/c to length of Payload field+ stop bits;
				{
					perror("ERROR write");
					exit(EXIT_FAILURE);
				}
				
				printf("\nwrite is over\n");			//Prints write is over;

				while((read(fdwr1,dt.ack,1)) == 0);		//Waiting for ACK;

				printf("\ncontent of ack:%x\n",dt.ack[0]);	//Prints contents of ACK received;
				
				if(dt.ack[0] == 0xf0)				//If ACK == 0xf0, Stop bits error and prints error in stop bits;
				{
					printf("\nError in Stop bits\n");
					break;
				}
				printf("\nsuccess\n");				//prints success if everything is perfect in write operation;				
				break;		
			}							//default case ends;

			case 1:							//case 1-- Read mode;
			{				
				while((r=read(fdwr1,fdata,(dt.array[2])))== 0);	// waiting for Data bytes;
				
				if(r == -1)
				{
					perror("ERROR read()");
					exit(EXIT_FAILURE);
				} 
				
				printf("\nread data:\n");			// Prints read contents(ADC values);

				for(i=0;i<(dt.array[2]);i++)
				{
					printf(" %x\t", *(fdata+i));
				}
								
				if((write(fdwr2,fdata,(dt.array[2]))) == -1)	//Write the read contents into header file;
				{
					perror("ERROR write");
					exit(EXIT_FAILURE);
				}
				printf("\ndata read success\n");		//Prints read success;
						
				if((r=read(fdwr2,fdata,1)) == -1)		//Read the stop bits;
				{
					perror("ERROR read()");
					exit(EXIT_FAILURE);
				} 

				printf("\nStop bits: %x\n",*fdata);

				if((write(fdwr1,fdata,1)) == -1)		//Sending stop bits;
				{
					perror("ERROR write");
					exit(EXIT_FAILURE);
				}
			
				while((read(fdwr1,dt.ack,1)) == 0);		//Waiting for ACK;

				printf("\ncontent of ack:%x\n",dt.ack[0]);	//Prints contents of ACK received;
				
				if(dt.ack[0] == 0xf0)				//If ACK == 0xf0, Stop bits error and prints error in stop bits;
				{
					printf("\nError in Stop bits\n");					
					break;
				}
				
				printf("\nsuccess\n");				//In read operation, of everything is success, prints success;
				break;			
			}							//case 1 -- ends
		}								//Switch case ends
	}					
	exit(EXIT_SUCCESS);
	//	while ends
}


