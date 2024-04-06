/******************************************************************************
 * File Name   :  OLED_SSD1306.c
 * Author      :  43oh - MSP430 News Projects and Forums.
               :  (http://www.43oh.com)
 * Description :  Lowlevel driver for the OLED SSD1306 
 * Date        :  October 21, 2011.
 *****************************************************************************/
//#include <msp430.h>
#include <msp430f5529.h>
#include<stdio.h>
#include<math.h>

#include "OLED_SSD1306.h"

#include "FontPack.h"

char SSD1306_init[] = {
		SSD1306_DISPLAYOFF,
		SSD1306_SETLOWCOLUMN,
		SSD1306_SETHIGHCOLUMN,
		SSD1306_SETSTARTLINE,
		SSD1306_SETCONTRAST,
		0xcf,
		SSD1306_SEGREMAP,
		SSD1306_NORMALDISPLAY,
		SSD1306_SETMULTIPLEX,
		0x3f,
		SSD1306_SETDISPLAYOFFSET,
		0x00,
		SSD1306_SETDISPLAYCLOCKDIV,
		0x80,
		SSD1306_SETPRECHARGE,
		0xf1,
		SSD1306_SETCOMPINS,
		0x12,
		SSD1306_SETVCOMDETECT,
		0x40,
		SSD1306_CHARGEPUMP,
		0x14,
		SSD1306_DISPLAYON
};

/* Function    : SSD1306PinSetup()
 * Description : Sets up the pins to the OLED display.
 * Input       : None
 * Output      : None
 * 
 */
void SSD1306PinSetup( void )
{
	// G2553 UCB0
//	OLED_SPI_SEL |= OLED_SIMO + OLED_SCLK;
//	OLED_SPI_SEL2 |= OLED_SIMO + OLED_SCLK;

	// Corrected for MSP430F5529
	OLED_SPI_SEL |= OLED_SIMO + OLED_SCLK;
	// The line using OLED_SPI_SEL2 is removed since P1SEL2 doesn't exist and is likely not needed


	OLED_SPI_DIR |= OLED_SIMO + OLED_SCLK;

	OLED_IO_DIR |= OLED_RES + OLED_DC;
	OLED_CS_DIR |= OLED_CS;

   // 100ms delay at 16Mhz clock
   __delay_cycles(1600000);
}

/* Function    : SSD1306Init()
 * Description : Initializes the OLED display
 * Input       : None
 * Output      : None
 */
void SSD1306Init( void )
{
	OLED_DESELECT;

	OLED_COMMAND;

	OLED_RES_LOW;

   // Delay for 200ms at 16Mhz
   __delay_cycles(3200000);

   OLED_RES_HIGH;

   __delay_cycles(1600000);

   //TRIGGER;

   SSD1306SendCommand(SSD1306_init, sizeof SSD1306_init);

}

/* Function    : SSD1306SendCommand( char *data, int i )
 * Description : Sends a command to the OLED display
 * Input       : Command pointer and number of bytes to transfer
 * Output      : None
 * gwd
 */
void SSD1306SendCommand( char *data, int i )
{ 
	__disable_interrupt();

	OLED_DESELECT;	// cs high
	OLED_COMMAND;	// D/C low
	OLED_SELECT;	// cs low

	while(i)
	{
//		while(!(IFG2 & UCB0TXIFG));	// wait for buffer
//		while(!(UCB0IFG & UCB0TXIFG)); // wait for buffer
		while (!(UCB0IFG & UCTXIFG)); // USCI_A0 TX buffer ready?



		UCB0TXBUF = *data;	// load data

		data++;				// increment pointer

		i--;				// decrement byte counter
	}

	while(UCB0STAT & UCBUSY); 			// wait for all to finish
	UCB0RXBUF;							// clear IFG and overruns

	OLED_DESELECT;	// cs high

	__enable_interrupt();
}

/* Function    : SSD1306SendData( char c )
 * Description : Sends data to the OLED display
 * Input       : Data
 * Output      : None
 */
void SSD1306SendData( char *data, int i )
{
	__disable_interrupt();

	OLED_DESELECT;	// cs high
	OLED_DATA;		// D/C high
	OLED_SELECT;	// cs low

	while(i)
	{

		// insert boundary check here

//		while(!(IFG2 & UCB0TXIFG));	// wait for buffer
//		while(!(UCB0IFG & UCB0TXIFG)); // wait for buffer
	    while (!(UCB0IFG & UCTXIFG)); // USCI_A0 TX buffer ready?

		UCB0TXBUF = *data;	// load data

		data++;				// increment pointer

		i--;				// decrement byte counter
	}

	while(UCB0STAT & UCBUSY); 			// wait for all to finish
	UCB0RXBUF;							// clear IFG and overruns

	OLED_DESELECT;	// cs high

	__enable_interrupt();
}

/* Function		: setAddress(char page, char column)
 * Description	: sets position of LCD RAM
 * Input		: page (0-7), column (0-127)
 * Output		: none
 */
void setAddress( char page, char column )
{

	char pageAddress[] = {SSD1306_PAGE_START_ADDRESS};
	char columnAddress[] = { SSD1306_COLUMNADDRESS, SSD1306_COLUMNADDRESS_MSB, SSD1306_COLUMNADDRESS_LSB };

	if (page > SSD1306_MAXROWS)
	{
		page = SSD1306_MAXROWS;
	}

	if (column > SSD1306_LCDWIDTH)
	{
		column = SSD1306_LCDWIDTH;
	}

	pageAddress[0] = SSD1306_PAGE_START_ADDRESS | (SSD1306_MAXROWS - page);

	columnAddress[0] = SSD1306_COLUMNADDRESS;
	columnAddress[1] = SSD1306_COLUMNADDRESS_MSB | column;
	columnAddress[2] = SSD1306_COLUMNADDRESS_LSB;

	//__no_operation();

	SSD1306SendCommand(pageAddress, 1);
	SSD1306SendCommand(columnAddress, 3);

}

/* Function		: clearScreen(void)
 * Description	: wipes data in memory to 0x00
 * Input		: none
 * Output		: blank screen (0x00)
 */
void clearScreen(void)
{
	char ramData[] = {0x00};

	char i,j;

	for(i=0; i < 8 ;i++)
	{
		setAddress(i,0);

	    for(j=0; j < SSD1306_LCDWIDTH; j++)
	    {
	    	SSD1306SendData(ramData, 1);
	    }
	}
}

/* Function		: charDraw(char row, char column, int data)
 * Description  : write char to address
 * Input		: (row, column), data
 * Output		: char at address
 */
void charDraw(char row, char column, int data)
{
	int h;

	if (row > SSD1306_MAXROWS)
	{
		row = SSD1306_MAXROWS;
	}

	if (column > SSD1306_LCDWIDTH)
	{
		column = SSD1306_LCDWIDTH;
	}

	if ( data < 32 || data > 129 )
	{
		data = '.';
	}

	h = (data - 32) * 6;

	setAddress(row, column);
	SSD1306SendData( (char *)FONT6x8 + h, 6 );

}

/* Function		: stringDraw(char row, char column, char *word)
 * Description  : write string to address
 * Input		: (row, column), data
 * Output		: char at address
 */
void stringDraw( char row, char column, char *word)
{
	char a = 0;

	if (row > SSD1306_MAXROWS)
	{
		row = SSD1306_MAXROWS;
	}

	if (column > SSD1306_LCDWIDTH)
	{
		column = SSD1306_LCDWIDTH;
	}

	while (word[a] != 0)
	{
		if (word[a] != 0x0A)
		{
			if (word[a] != 0x0D)
			{
				charDraw(row, column, word[a]);

				__no_operation();

				column += 6;

				if (column >= (SSD1306_LCDWIDTH - 6 ))
				{
					column = 0;
					if ( row < SSD1306_MAXROWS )
						row++;
					else
						row = 0;
				}

			}
			else
			{
				if ( row < SSD1306_MAXROWS )
					row++;
				else
					row = 0;
				column = 0;
			}
			a++;
		}
	}
}

/* Function		: pixelDraw(char x, char y)
 * Description  : draw individual pixel at coordinate
 * Input		: X,Y coordinates
 * Output		: single pixel
 */
void pixelDraw(char x, char y)
{
	char page, temp;
	char coordinate[] = {0x00};

	if (x > SSD1306_LCDWIDTH - 1)
	{
		x = SSD1306_LCDWIDTH - 1;
	}

	if (y > SSD1306_LCDHEIGHT - 1)
	{
		y = SSD1306_LCDHEIGHT - 1;
	}

	page = y / 8;
	temp = 0x80 >> (y % 8);
	coordinate[0] = temp;

	setAddress(page,x);
	SSD1306SendData(coordinate, 1);

}

/* Function		: horizontalLine(char xStart, char xStop, char y)
 * Description  : draw horizontal line from xStart to xStop
 * Input		: X Start & Stop coordinate, Y
 * Output		: display horizontal line
 */
void horizontalLine(char xStart, char xStop, char y)
{
	char temp = 0;
	char page, a;
	char ramData[] = { 0x00 };

	if (xStart > SSD1306_LCDWIDTH - 1)
	{
		xStart = SSD1306_LCDWIDTH - 1;
	}

	if (xStop > SSD1306_LCDWIDTH - 1)
	{
		xStop = SSD1306_LCDWIDTH - 1;
	}

	if (y > SSD1306_LCDHEIGHT - 1)
	{
		y = SSD1306_LCDHEIGHT - 1;
	}

	if (xStart > xStop)
	{
		temp = xStart;
		xStart = xStop;
		xStop = temp;
	}

	page = y / 8;
	temp = 0x80 >> (y %8);

	a = xStart;
	ramData[0] = temp;

	setAddress(page, xStart);

	while (a <= xStop)
	{
		SSD1306SendData(ramData, 1);
		a++;
	}
}

/* Function		: verticalLine(char x, char yStart, char yStop)
 * Description  : draw vertical line from yStart to yStop
 * Input		: Y Start & Stop coordinate, X
 * Output		: display vertical line
 */
void verticalLine(char x, char yStart, char yStop)
{
	char temp1 = 0, temp2 = 0;
	char page1, page2, pageStart;
	char a;
	char ramData1[] = {0x00};
	char ramData2[] = {0x00};

	if (x > SSD1306_LCDWIDTH - 1)
	{
		x = SSD1306_LCDWIDTH - 1;
	}

	if (yStart > SSD1306_LCDHEIGHT - 1)
	{
		yStart = SSD1306_LCDHEIGHT - 1;
	}

	if (yStop > SSD1306_LCDHEIGHT - 1)
	{
		yStop = SSD1306_LCDHEIGHT - 1;
	}

		if (yStart > yStop)
		{
			temp1 = yStart;
			yStart = yStop;
			yStop = temp1;
		}

	page1 = yStart / 8;
	page2 = yStop / 8;
	pageStart = yStart % 8;

	if (page1 != page2)
	{
		if ( pageStart > 0)
		{
			temp1 = 0xFF00 >> pageStart;
			temp1 = temp1 ^ 0xFF;
		}
		else
		{
			temp1 = 0xFF;
		}
	}
	else
	{
		temp1 = 0;

		a = yStart - (page1 * 8);
		a = 0xFF00 >> a;
		temp2 = temp2 ^ a;
	}

	ramData1[0] = temp1;

	setAddress(page1,x);
	SSD1306SendData(ramData1, 1);

	a = page1 + 1;
	ramData1[0] = 0xFF;

	while (a < page2)
	{
		setAddress(a, x);
		SSD1306SendData(ramData1, 1);
		a++;
	}

	temp2 = 8 - (yStop % 8);
	temp2--;
	temp2 = 0xFF << temp2;

	ramData2[0] = temp2;

	setAddress(page2, x);
	SSD1306SendData(ramData2, 1);
}

/* Function		: imageDraw(const char IMAGE[], char row, char column)
 * Description  : draw image at row, column
 * Input		: image, row, column
 * Output		: display image
 */
// from TI_DogsLCD_HAL
void imageDraw(const char IMAGE[], char row, char column)
{
	char a, height, width;

	width = IMAGE[0];
	height = IMAGE[1];

	for ( a = 0; a < height; a++)
	{
		setAddress(row + a, column);
		SSD1306SendData((char *)IMAGE + 2 + a * width, width);
	}
}


// circleDraw function is not complete. needs checks for page surround x-axis for proper column fill

/* Function		: circleDraw(register int x, register int y, int r)
 * Description  : draw circle at x,y of radius r
 * Input		: x,y, radius
 * Output		: display circle
 */
void circleDraw(register int x, register int y, int r)
{
    register int xx = -r;
    register int yy = 0;
    register int e = 2 - (2 * r);
    do {
        pixelDraw(x - xx, y + yy);
        pixelDraw(x - yy, y - xx);
        pixelDraw(x + xx, y - yy);
        pixelDraw(x + yy, y + xx);
        if(e >  xx) e += ((++xx << 1) + 1);
        if(e <= yy) e += ((++yy << 1) + 1);
    } while (xx < 0);
}


/* Function    : Fill_RAM( unsigned char c )
 * Description : Fills the OLED with one particular data
 * Input       : Data
 * Output      : None
 */
void Fill_RAM( char data )
{
   unsigned char i,j;

   char ramData[] = { 0x00};

   ramData[0] = data;

   for(i=0; i < 8 ;i++)
   {
	  setAddress(i,0);

      for(j=0; j < SSD1306_LCDWIDTH; j++)
      {
         SSD1306SendData(ramData, 1);
      }
   }
}

/* Function    : Fill_RAM_PAGE
 * Description : Fills an OLED page with a particular data
 * Input       : page no.[0-7], Data
 * Output      : None
 */
void Fill_RAM_PAGE(unsigned char page, char data)
{
   unsigned char j;

   char ramData[] = { 0x00 };

   ramData[0] = data;

   setAddress(page,0);

   for(j=0; j < SSD1306_LCDWIDTH ;j++)
   {
      SSD1306SendData(ramData, 1);
   }
}

void Set_Contrast_Control(unsigned char d)
{
	char ramData[] = {SSD1306_SETCONTRAST, 0x00};
	ramData[1] = d;

	SSD1306SendCommand(ramData, 2);
}

void Set_Inverse_Display(unsigned char d)
{
	// 0 = normal, 1 = inverted

	char ramData[] = {SSD1306_NORMALDISPLAY};
	ramData[0] = ramData[0] + d;

	SSD1306SendCommand(ramData, 1);
}

void Set_Display_On_Off(unsigned char d)	
{
	// 0 = off, 1 = on

	char ramData[] = {SSD1306_DISPLAYOFF};
	ramData[0] = ramData[0] + d;

	SSD1306SendCommand(ramData, 1);
}
