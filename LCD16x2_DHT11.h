/*
 * DHT11 Interfacing with 8051
 * http://www.electronicwings.com
 */

#ifndef LCD16x2_DHT11
#define LCD16x2_DHT11

#include<reg52.h>
#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include "LCD8bit.h"
#include "eeprom.h"

sbit DHT11=P1^7;		/* Connect DHT11 Sensor Pin to P2.2 Pin */
sbit FanT=P2^3;	
sbit LedH=P2^4;	
sbit TestLed=P2^2;
int I_RH,D_RH,I_Temp,D_Temp,CheckSum;

unsigned char   
	gb_d1on[2],  // device 1 turn on at
  gb_d1off[2], // device 1 turn off at  
    buf2[2],     // buffer size 2
    buf16[16];   // bugger size 16, e.g.: for LCD, uart...

void FanControl()
{	
TestLed = 1;


	

	
  if (DA_GetWorkingMode() == '1') 
  {
  // cooling fan auto on when in AUTO model
	//	DA_GetDevice1TurnOnAt(&gb_d1on);
	//	DA_GetDevice1TurnOffAt(&gb_d1off);

	clearLine(0);
	//DA_GetDevice1TurnOnAt(&gb_d1on);
	 EepromReadNBytes(3, gb_d1on, 2, 0);
  memset(buf16, 0, 16);
  sprintf(buf16, "Fan TemON >= %d", atoi(gb_d1on));
  displayText(buf16);
	clearLine(1);
	DA_GetDevice1TurnOffAt(&gb_d1off);
  memset(buf16, 0, 16);
  sprintf(buf16, "  OFF <= %s", gb_d1off);
	displayText(buf16);
	delay(1000);
 
	if(I_Temp >= atoi(gb_d1on)) {//30 atoi(gb_d1on)
	EepromWriteByte(0, 13, 0);
	FanT=0;
	}else if(I_Temp <= atoi(gb_d1off)){//25 atoi(gb_d1off)
	EepromWriteByte(1, 13, 0);
	FanT=1;
	}
		
  if(I_RH > 25) {//25 atoi(gb_d2off)
	EepromWriteByte(1, 14, 0);
	LedH = 1;
	}else if(I_RH <= 22)	{//21  atoi(gb_d2on)
	EepromWriteByte(0, 14, 0);
	LedH = 0;
		TestLed = 0;
		delay(500);
		TestLed = 1;
	};
 }
	
 else{//Manual mode
	 FanT = EepromReadByte(13, 0) - '0';//Convert char to int
	 LedH = EepromReadByte(14, 0) - '0';
	 

 }
}


void timer_delay20ms()		/* Timer0 delay function */
{
	TMOD = 0x01;
	TH0 = 0xB8;				/* Load higher 8-bit in TH0 */
	TL0 = 0x0C;				/* Load lower 8-bit in TL0 */
	TR0 = 1;					/* Start timer0 */
	while(TF0 == 0);	/* Wait until timer0 flag set */
	TR0 = 0;					/* Stop timer0 */
	TF0 = 0;					/* Clear timer0 flag */
}

void timer_delay30us() /* Timer0 delay function */
{
	TMOD = 0x01;			/* Timer0 mode1 (16-bit timer mode) */
	TH0 = 0xFF;				/* Load higher 8-bit in TH0 */
	TL0 = 0xF1;				/* Load lower 8-bit in TL0 */
	TR0 = 1;					/* Start timer0 */
	while(TF0 == 0);	/* Wait until timer0 flag set */
	TR0 = 0;					/* Stop timer0 */
	TF0 = 0;					/* Clear timer0 flag */
}

void Request()			/* Microcontroller send start pulse or request */
{
	DHT11 = 0;		 		/* set to low pin */
	timer_delay20ms();/* wait for 20ms */
	DHT11 = 1;				/* set to high pin */
}

void Response()			/* receive response from DHT11 */
{
	while(DHT11==1);
	while(DHT11==0);
	while(DHT11==1);
}

int Receive_data()			/* receive data */
{
	int q,c=0;	
	for (q=0; q<8; q++)
	{
		while(DHT11==0);		/* check received bit 0 or 1 */
		timer_delay30us();
		if(DHT11 == 1)			/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);	/* then its logic HIGH */
		
		else								/* otherwise its logic LOW */
		c = (c<<1);
		
		while(DHT11==1);
	}
	return c;
}

void Dht_Update()
{
	unsigned char dat[20];
	unsigned char    

    buf2[2];     // buffer size 2
	initLCD();								/* initialize LCD */
	
//while(1)
	{		
		Request();							/* send start pulse */
		Response();							/* receive response */
		
		I_RH = Receive_data();		/* store first eight bit in I_RH */		
		D_RH = Receive_data();		/* store next eight bit in D_RH */	
		I_Temp = Receive_data();	/* store next eight bit in I_Temp */
		D_Temp = Receive_data();	/* store next eight bit in D_Temp */
		CheckSum = Receive_data(); /* store next eight bit in CheckSum */

		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
  {
    clearLine(0);
    displayText("Sensor error");
    return;
  }
	
			sprintf(buf2, "%d", I_RH);
			DA_SetHumidity(buf2);// Save data to EEPROM
			sprintf(buf2, "%d", I_Temp);
			DA_SetTemperature(buf2);
	
			setCursor(0,0);
			sprintf(dat,"Hum = %d.%d",I_RH,D_RH);
			displayText(dat);
			displayText(" %");
			sprintf(dat,"Tem = %d.%d",I_Temp,D_Temp);   
			setCursor(0,1);
			displayText(dat);
			displayChar(0xDF);
			displayText("C");
		delay(1000);
		FanControl();
	}	
}
