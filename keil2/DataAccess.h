#ifndef __DATAACCESS_H
#define __DATAACCESS_H
#include "eeprom.h"
// 
/* 
 * Example of \data configuration in EEPROM:
 * 0|123|4567890
 * 0|017|050
 * --------------------------------------------------------------
 * Address defination: https://docs.google.com/spreadsheets/d/1maRuC_2jccFvu1Zl6Ll2AjtJf_BLCOZsvrVx8x3rNZg/
 * Address	Length	Description
 * --------------------------------------------------------------
 * 0				1				working mode (value: 1==auto, 0==manual, 2: NaN)
 * 1				2				Celsius: Threshold turn ON the Air Conditional's Compressor
 * 3				2				Celsius: Threshold turn OFF the Air Conditional's Compressor
 * 5				2				Celsius: temperature. i.e.: 05 -> 05
 * 7				2				percent: humidity. i.e.: 16 -> 16
 * 9        2       percent: Threshold turn ON the Dehumidifer fan
 * 11       2       percent: Threshold turn OFF the Dehumidifer fan
 */
 
void DA_SetValue(unsigned char *pointer, int addr)
{
	while(*pointer) 
	{
    EepromWriteByte(*pointer, addr, 0);
    pointer++;
    addr++;
  }
}

void DA_GetHumidity(unsigned char *pointer)
{
  EepromReadNBytes(7, pointer, 2, 0);
}

void DA_SetHumidity(unsigned char *pointer)
{
	DA_SetValue(pointer, 7);
}

void DA_GetTemperature(unsigned char *pointer)
{
  EepromReadNBytes(5, pointer, 2, 0);
}

void DA_SetTemperature(unsigned char *pointer)
{
	DA_SetValue(pointer, 5);
  //EepromWriteNBytes(5, pointer, 2, 0);
}


/*
Start position: 4
Example data: 017 --> 01.7 Celsius (length == 4)
*/
void DA_GetDevice1TurnOffAt(unsigned char *pointer)
{
  EepromReadNBytes(3, pointer, 2, 0);
}

void DA_SetDevice1TurnOffAt(unsigned char *pointer) // set device 1 threshold to turn on
{
	DA_SetValue(pointer, 3);
  //EepromWriteNBytes(3, pointer, 2, 0);
}

/*
Start position: 1
Example data: 017 --> 01.7 Celsius (length == 3)
*/
void DA_GetDevice1TurnOnAt(unsigned char *pointer)
{
  EepromReadNBytes(1, pointer, 2, 0);
}

void DA_SetDevice1TurnOnAt(unsigned char *pointer) // set device 1 threshold to turn on
{
  DA_SetValue(pointer, 1);
  //EepromWriteNBytes(1, pointer, 2, 0);
}


/*
Dehumidifier
*/
/*
void DA_GetDevice2TurnOffAt(unsigned char *pointer)
{
  EepromReadNBytes(11, pointer, 2, 0);
}
void DA_SetDevice2TurnOffAt(unsigned char *pointer) // set device 1 threshold to turn on
{
	DA_SetValue(pointer, 11);
}
void DA_GetDevice2TurnOnAt(unsigned char *pointer)
{
  EepromReadNBytes(9, pointer, 2, 0);
}
void DA_SetDevice2TurnOnAt(unsigned char *pointer) // set device 1 threshold to turn on
{
  DA_SetValue(pointer, 9);
  //EepromWriteNBytes(1, pointer, 2, 0);
}
*/


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