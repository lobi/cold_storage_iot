#ifndef __DATAACCESS_H
#define __DATAACCESS_H
#include "eeprom.h"
// 
/* 
 * Example of \data configuration in EEPROM:
 * 0|123|4567890
 * 0|017|050
 * --------------------------------------------------------------
 * Address	Length	Description
 * --------------------------------------------------------------
 * 0				1				working mode (value: 1==auto, 0==manual, 2: NaN)
 * 1				4				Celsius: Threshold turn on the Air Conditional's Compressor
 * 5				4				Celsius: Threshold turn off the Air Conditional's Compressor
 * 9				4				Celsius: temperature. i.e.: 0550 -> 05.50
 * 13				4				percent: humidity. i.e.: 1650 -> 16.50
 */
 
void DA_SetValue(unsigned char *pointer, int addr)
{
	while(*pointer) 
	{
    EepromWriteByte(*pointer++, addr, 0);
		addr++;
  }
}

//void DA_GetHumidity(unsigned char *pointer)
//{
//	int addr = 13;
//	int i = 0;
//	for (i = 0; i < 4; i++)
//	{
//		*pointer = EepromReadByte(addr, 0);
//		addr++;
//		*pointer++;
//	}
//}
void DA_SetHumidity(unsigned char *pointer)
{
	DA_SetValue(pointer, 13);
}

//void DA_GetTemperature(unsigned char *pointer)
//{
//	int addr = 9;
//	int i = 0;
//	for (i = 0; i < 4; i++)
//	{
//		*pointer = EepromReadByte(addr, 0);
//		addr++;
//		*pointer++;
//	}
//}
void DA_SetTemperature(unsigned char *pointer)
{
	DA_SetValue(pointer, 9);
}


/*
Start position: 4
Example data: 017 --> 01.7 Celsius (length == 4)
*/
void DA_GetDevice1TurnOffAt(unsigned char *pointer)
{
	int addr = 4;
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		*pointer = EepromReadByte(addr, 0);
		addr++;
		*pointer++;
	}
}

void DA_SetDevice1TurnOffAt(unsigned char *pointer) // set device 1 threshold to turn on
{
	DA_SetValue(pointer, 4);
}

/*
Start position: 1
Example data: 017 --> 01.7 Celsius (length == 3)
*/
void DA_GetDevice1TurnOnAt(unsigned char *pointer)
{
	int addr = 1;
	int i = 0;
	for (i = 0; i < 3; i++)
	{
		*pointer = EepromReadByte(addr, 0);
		addr++;
		*pointer++;
	}
}

void DA_SetDevice1TurnOnAt(unsigned char *pointer) // set device 1 threshold to turn on
{
	DA_SetValue(pointer, 1);
}

/*working mode*/
unsigned char DA_GetWorkingMode()
{
	unsigned char mode = EepromReadByte(0, 0);
	if (mode == '0' || mode == '1') // safe way, prevent error exception
	{
		return mode;
	}
	else
	{
		return '2';
	}
}

void DA_SetWorkingMode(unsigned char mode)
{
	EepromWriteByte(mode, 0, 0);
}

void DA_Init()
{
	if (DA_GetWorkingMode() == '2') // not set before
	{
		// set default value: 1 -> auto
		DA_SetWorkingMode('1');
	}
	// else: already set before. no need to set default value
}

#endif