#ifndef __EEPROM_H
#define __EEPROM_H
//#include "delay.h"

void EepromWriteByte( unsigned char Data, unsigned char Address, unsigned char Page)
{
	I2CStart();
	I2CSend(0xA0 | (Page<<1));
	I2CSend(Address);
	I2CSend(Data);
	I2CStop();
	Delay_ms(1);
}

unsigned char EepromReadByte(unsigned char Address, unsigned char Page)
{
  unsigned char Data;

	I2CStart();            
	I2CSend(0xA0 | (Page<<1));
	I2CSend(Address);
	I2CStart();
	I2CSend(0xA1 | (Page<<1));
	Data = I2CRead();
	I2CNak();
	I2CStop();
	Delay_us(10);
	
	return Data;
}


void EepromWriteNBytes(unsigned char Location, unsigned char *Pointer, char N, unsigned char Page)
{
	while(N !=  0)
	{
		EepromWriteByte(*Pointer, Location, Page);
		Location++; Pointer++; N--;		
	}
}

void EepromReadNBytes(unsigned char Location, unsigned char *Pointer, char N, unsigned char Page)
{
	while(N !=  0)
	{
		*Pointer = EepromReadByte(Location, Page);
		Location++; Pointer++; N--;
	}
}

void EepromErasePage(unsigned char Page)
{
	unsigned char i;
	for(i=0; i<255; i++)
		EepromWriteByte(0x00, i, Page);	
}

/*
void EepromEraseAll()
{
	unsigned char i;
	for(i=0; i<255; i++)
	{
		EepromWriteByte(0x00, i, 0);
		EepromWriteByte(0x00, i, 1);
	}	
}
*/
#endif