# User-defined-data-acquisition-protocol-between-linux-and-ARM

   The process of data transmission and reception between ARM Microcontroller and Linux host machine is 
   implemented over standard UART. A user defined protocol is implemented over standard UART protocol for data
   acquisition.
    
    
                    LED
                     |                      UART                        (Controller)
                    ARM     <---------------------------------->     Linux host machine
                /        \         User defined protocol       (Read/write the data from/to ARM)
            LCD      Analog sensor
      
      
    Software Tools:
    	Keil V4
    	
    Hardware:
    	LPC2378/88 (MCB2300)
    
    Communication between a linux host machine and ARM LPC2377/78 Microncontroller based MCB2300 
    
    User defined protocol on a standard UART/RS-232 protocol:
    *-------------------------------------------------------------------------
	Start bits--1B
	Identifier--1B
	Payload length--1B
	Mode (Read/Write)--1B
	Future use--4B
	User payload (depends on mode bits)
	Stop bits--1B
    *-------------------------------------------------------------------------
    Example: fe|01|08|01|00000000|0102030405060708|ff
*   Create an hex data file according to the above described protocol

           $ echo fe010801000000000102030405060708ff | xxd -r -p > frame
           $ hexdump -C frame
           
*   frame is the hex file


    Linux host machine can read/write the data from/to ARM Microcontroller(LPC2377/78) on standard UART 
    by sending these control bytes to ARM Microcontroller
    
*    Write mode -- writes the data to the output devices on MCB2300

        LED-----0     -- set these values in identifier byte
        LCD-----1
    
*   Read mode -- reads the data from MCB2300

        ADC-----0 (default)  (Analog sensor / pot is attached) -- set this value in identifier byte
        
  
  --> Execution on ARM:
  
  1)  Create a project on Keil tool and add all the files from ARM_LPC2377_78_MCB2300 directory
  
  *   Note: lcd.c, serial.c and retarget.c are defined in "LPC23xx.H" header file (so not necessary to 
  define again in "library.h" file)
  
  2) Compile the program and upload it to MCB2300
    
  
  --> Execution on Linux machine:

  1) Compile linux_arm_customprotocol_uart.c file in Linux_Host_Machine folder using gcc
  
            $ gcc linux_arm_customprotocol_uart.c -o test
            
  2) Create an hex file using the above described commands and execute the compiled binary file using
  the below command
    
            $ ./test /dev/ttyS0 frame
            
  
  
