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

sbit DHT11=P3^7;		/* Connect DHT11 Sensor Pin to P2.1 Pin */
int I_RH,D_RH,I_Temp,D_Temp,CheckSum; 

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
			displayText("%");
			sprintf(dat,"Tem = %d.%d",I_Temp,D_Temp);   
			setCursor(0,1);
			displayText(dat);
			displayChar(0xDF);
			displayText("C");
		delay(1000);
	}	
}
