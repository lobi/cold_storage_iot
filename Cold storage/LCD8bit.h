#ifndef __LCD8bit_H
#define __LCD8bit_H

//#include "delay.h"

#define PORT P0
/* on real 8051 kit */
sbit RS = P2^6;
sbit RW = P2^5;
sbit EN = P2^7;

/* on proteus */
// sbit RS = P2^7;
// sbit RW = P2^6;
// sbit EN = P2^5;


void cmd(char a)
{
	P0 = a;
  RS=0;RW=0;EN=1;
	Delay_ms(8);
	EN=0;
}

void displayChar(char b)
{
	P0 = b;
	RS=1; RW=0; EN=1;
	Delay_ms(8);
	EN=0;
}

void displayText(const char *str)
{
	int i;
	for (i = 0; str[i] != '\0' && i < 16; i++) /* Send each char of string till the NULL */
	{
		displayChar(str[i]); /* Call LCD data write */
	}
}

void setCursor(int a, int b)
{
	int i=0;
	
  switch(b)
	{
		case 0:cmd(0x80);break; // force cursor to beginning of the first line
		case 1:cmd(0xC0);break; // force cursor to beginning of second line
		case 2:cmd(0x94);break; // on 3rd line is 0x94  --> write text "Hello3 " on the 3rd line ??
		case 3:cmd(0xd4);break; // Shift the cursor position to the right
	}
	
	for(i = 0; i < a; i++)
	{
		cmd(0x14); // Shift the cursor position to the right
	}
}

void clearLine(unsigned int line)
{
	int i = 0;
	setCursor(0, line); // set to the first position
	for (i = 0; i < 16; i++)
	{
		P0 = ' ';
		RS=1; RW=0; EN=1;
		Delay_ms(2);
		EN=0;
	}
	setCursor(0, line); // set to the first position
	//cmd(0x80);// force cursor to beginning of the first line
}

void initLCD()
{
	cmd(0x0F); // Display on, cursor blinking
	cmd(0x38); // 2 lines and 5x7 matrix
	cmd(0x01); // clear display
	cmd(0x80); // force cursor to the beginning (1st line)
	cmd(0x06); // Increment cursor (shift cursor to right)
	cmd(0x0C); // force cursor to the beginning (2nd line), cursor off
}

void dispIntAsStr(unsigned int Integer)
{
	if((Integer/100+48)!='0')
		displayChar(Integer/100+48);
	
	displayChar((Integer%100)/10+48);
	displayChar(Integer%10+48);
}	

#endif
