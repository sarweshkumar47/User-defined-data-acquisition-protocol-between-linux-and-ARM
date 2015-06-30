/****************************************************************************/
/* LCD.c: Functions for 2 line 16 character Text LCD, with 4-bit interface  */
/****************************************************************************/
/* This file is part of the uVision/ARM development tools.                  */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.              */
/* This software may only be used under the terms of a valid, current,      */
/* end user licence from KEIL for a compatible version of KEIL software     */
/* development tools. Nothing else gives you the right to use this software.*/
/****************************************************************************/
/**
* @file lcd.c
*
* Functions for 2 line 16 character Text LCD, with 4-bit interface
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  keil
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include <lpc23xx.h>                     /* LPC23xx definitions             */

/************************** Constant Definitions ****************************/

/*********************** Hardware specific configuration ********************/

/*------------------------- Speed dependant settings -----------------------*/

/* If processor works on high frequency delay has to be increased, it can be
   increased by factor 2^N by this constant                                 */
#define DELAY_2N     0

/*------------------------- Text LCD size definitions ----------------------*/

#define LineLen     16                  /* Width (in characters)            */
#define NumLines     2                  /* Hight (in lines)                 */

/*-------------------- LCD interface hardware definitions ------------------*/

/* PINS:
   - DB4 = P1.24
   - DB5 = P1.25
   - DB6 = P1.26
   - DB7 = P1.27
   - E   = P1.31 (for V1 P1.30)
   - RW  = P1.29
   - RS  = P1.28                                                            */


#define MCB2300_V1                      /* First version of MCB2300         */

#define PIN_E                 0x80000000
#define PIN_RW                0x20000000
#define PIN_RS                0x10000000
#define PINS_CTRL             0xB0000000
#define PINS_DATA             0x0F000000

#ifdef  MCB2300_V1
  #undef  PIN_E
  #define PIN_E               0xC0000000
  #undef  PINS_CTRL
  #define PINS_CTRL           0xF0000000
#endif

/* pin E  setting to 0 or 1                                                 */
#define LCD_E(x)              ((x) ? (IOSET1 = PIN_E)  : (IOCLR1 = PIN_E) );

/* pin RW setting to 0 or 1                                                 */
#define LCD_RW(x)             ((x) ? (IOSET1 = PIN_RW) : (IOCLR1 = PIN_RW));

/* pin RS setting to 0 or 1                                                 */
#define LCD_RS(x)             ((x) ? (IOSET1 = PIN_RS) : (IOCLR1 = PIN_RS));

/* Reading DATA pins                                                        */
#define LCD_DATA_IN           ((IOPIN1 >> 24) & 0xF)

/* Writing value to DATA pins                                               */
#define LCD_DATA_OUT(x)       IOCLR1 = PINS_DATA; IOSET1 = (x & 0xF) << 24;

/* Setting all pins to output mode                                          */
#define LCD_ALL_DIR_OUT       IODIR1  |=  PINS_CTRL | PINS_DATA;

/* Setting DATA pins to input mode                                          */
#define LCD_DATA_DIR_IN       IODIR1 &= ~PINS_DATA;

/* Setting DATA pins to output mode                                         */
#define LCD_DATA_DIR_OUT      IODIR1 |=  PINS_DATA;

/****************************************************************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/* 8 user defined characters to be loaded into CGRAM (used for bargraph)      */
const unsigned char UserFont[8][8] = {
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  { 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10 },
  { 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18 },
  { 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C },
  { 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E },
  { 0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F },
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }
};


/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
* Delay in while loop cycles.
*
* @param	cnt is the number of while cycles to delay.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

static void LcdDelay (int cnt)
{
  cnt <<= DELAY_2N;

  while (cnt--);
}


/****************************************************************************/
/**
* Read status of LCD controller.
*
* @param	None.
*
* @return	Status byte contains busy flag and address pointer.
*
* @note		None.
*
*****************************************************************************/

static unsigned char LcdReadStatus (void)
{
  unsigned char status;

  LCD_DATA_DIR_IN
  LCD_RS(0)
  LCD_RW(1)
  LcdDelay(10);
  LCD_E(1)
  LcdDelay(10);
  status  = LCD_DATA_IN << 4;
  LCD_E(0)
  LcdDelay(10);
  LCD_E(1)
  LcdDelay(10);
  status |= LCD_DATA_IN;
  LCD_E(0)
  LCD_DATA_DIR_OUT
  return (status);
}


/****************************************************************************/
/**
* Wait until LCD controller busy flag is 0
*
* @param	None.
*
* @return	Status byte of LCD controller (busy + address).
*
* @note		None.
*
*****************************************************************************/

static unsigned char LcdWaitWhileBusy (void)
{
  unsigned char status;

  do  {
    status = LcdReadStatus();
  }  while (status & 0x80);           /* Wait for busy flag                 */

  return (status);
}


/****************************************************************************/
/**
* Write 4-bits to LCD controller
*
* @param	c is the command to be written.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdWrite4bit (unsigned char c)
{
  LCD_RW(0)
  LCD_E(1)
  LCD_DATA_OUT(c&0x0F)
  LcdDelay(10);
  LCD_E(0)
  LcdDelay(10);
}


/****************************************************************************/
/**
* Write command to LCD controller.
*
* @param	command to be written.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdWriteCmd (unsigned char c)
{
  LcdWaitWhileBusy();

  LCD_RS(0)
  LcdWrite4bit (c>>4);
  LcdWrite4bit (c);
}


/****************************************************************************/
/**
* Write data to LCD controller.
*
* @param	c is the data to be written.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdWriteData (unsigned char c)
{
  LcdWaitWhileBusy();

  LCD_RS(1)
  LcdWrite4bit (c>>4);
  LcdWrite4bit (c);
}


/****************************************************************************/
/**
* Print Character to current cursor position.
*
* @param	c is the character to be printed.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdPutchar (char c)
{
  LcdWriteData (c);
}


/****************************************************************************/
/**
* Initialize the LCD controller.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdInit (void)
{
  int i;
  unsigned char const *p;

  /* Set all pins for LCD as outputs                                        */
  LCD_ALL_DIR_OUT

  LcdDelay (15000);
  LCD_RS(0)
  LcdWrite4bit (0x3);                 /* Select 4-bit interface             */
  LcdDelay (4100);
  LcdWrite4bit (0x3);
  LcdDelay (100);
  LcdWrite4bit (0x3);
  LcdWrite4bit (0x2);

  LcdWriteCmd (0x28);                 /* 2 lines, 5x8 character matrix      */
  LcdWriteCmd (0x0C);                 /* Display ctrl:Disp=ON,Curs/Blnk=OFF */
  LcdWriteCmd (0x06);                 /* Entry mode: Move right, no shift   */

  /* Load user-specific characters into CGRAM                               */
  LcdWriteCmd(0x40);                  /* Set CGRAM address counter to 0     */
  p = &UserFont[0][0];
  for (i = 0; i < sizeof(UserFont); i++, p++)
    LcdPutchar (*p);

  LcdWriteCmd(0x80);                  /* Set DDRAM address counter to 0     */
}


/****************************************************************************/
/**
* Set cursor position on LCD display.
*
* @param	column is the column position.
*
* @param	line is the line position.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdSetCursor (unsigned char column, unsigned char line)
{
  unsigned char address;

  address = (line * 40) + column;
  address = 0x80 + (address & 0x7F);
  LcdWriteCmd(address);               /* Set DDRAM address counter to 0     */
}


/****************************************************************************/
/**
* Clear the LCD display.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdClear (void)
{
  LcdWriteCmd(0x01);                  /* Display clear                      */
  LcdSetCursor (0, 0);
}


/****************************************************************************/
/**
* Print sting to LCD display.
*
* @param	pointer to output string.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/

void LcdPrint (unsigned char const *string)
{
  while (*string)  {
    LcdPutchar (*string++);
  }
}

/****************************************************************************/
