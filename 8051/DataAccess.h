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
 * 0				1				bit: working mode (value: 1==auto, 0==manual, 2: NaN)
 * 1				2				Celsius: Threshold turn ON the Air Conditional's Compressor
 * 3				2				Celsius: Threshold turn OFF the Air Conditional's Compressor
 * 5				2				Celsius: temperature. i.e.: 05 -> 05
 * 7				2				percent: humidity. i.e.: 16 -> 16
 * 9        2       percent: Threshold turn ON the Dehumidifer fan
 * 11       2       percent: Threshold turn OFF the Dehumidifer fan
 * 13       1       bit: Cooling Fan Sate
 * 14       1       bit: Humidifier Sate
 */
 
void DA_SetValue(unsigned char *pointer, int addr, int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    EepromWriteByte(pointer[i], addr, 0);
    Delay_ms(8);
    addr++;
  }
}

void DA_GetValue(unsigned char *ptr, int addr, int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    ptr[i] = EepromReadByte(addr, 0);
    //Delay_ms(4);
    addr++;
  }
}

void DA_GetHumidity(unsigned char *ptr)
{
  DA_GetValue(ptr, 7, 2);
}

void DA_SetHumidity(unsigned char *ptr)
{
	DA_SetValue(ptr, 7, 2);
}

void DA_GetTemperature(unsigned char *ptr)
{
  DA_GetValue(ptr, 5, 2);
}

void DA_SetTemperature(unsigned char *ptr)
{
	DA_SetValue(ptr, 5, 2);
}

void DA_GetDevice1TurnOffAt(unsigned char *ptr)
{
  DA_GetValue(ptr, 3, 2);
}

void DA_SetDevice1TurnOffAt(unsigned char *ptr)
{
	DA_SetValue(ptr, 3, 2);
}

void DA_GetDevice1TurnOnAt(unsigned char *ptr)
{
  DA_GetValue(ptr, 1, 2);
}

void DA_SetDevice1TurnOnAt(unsigned char *ptr)
{
  DA_SetValue(ptr, 1, 2);
}

void DA_GetDevice2TurnOffAt(unsigned char *ptr)
{
  DA_GetValue(ptr, 11, 2);
}

void DA_SetDevice2TurnOffAt(unsigned char *ptr)
{
	DA_SetValue(ptr, 11, 2);
}

void DA_GetDevice2TurnOnAt(unsigned char *ptr)
{
  DA_GetValue(ptr, 9, 2);
}

void DA_SetDevice2TurnOnAt(unsigned char *ptr)
{
  DA_SetValue(ptr, 9, 2);
}

char DA_GetDevice1State()
{
  unsigned char state = EepromReadByte(13, 0);
	return (state == '0' || state == '1' ? state : '2');
}

void DA_SetDevice1State(unsigned char state) // set device 1 threshold to turn on
{
  EepromWriteByte(state, 13, 0);
}
//
char DA_GetDevice2State()
{
  unsigned char state = EepromReadByte(14, 0);
	return (state == '0' || state == '1' ? state : '2');
}

void DA_SetDevice2State(unsigned char state) // set device 1 threshold to turn on
{
  EepromWriteByte(state, 14, 0);
}

/*working mode*/
unsigned char DA_GetWorkingMode()
{
	unsigned char mode = EepromReadByte(0, 0);
	return (mode == '0' || mode == '1' ? mode : '2');
}

void DA_SetWorkingMode(unsigned char mode)
{
	EepromWriteByte(mode, 0, 0);
}

void DA_Init()
{
  // check if your eeprom is not set, then set default value
	if (DA_GetWorkingMode() == '2')
	{
		// set default value
		DA_SetWorkingMode('1');
    Delay_ms(4);
    DA_SetDevice1TurnOnAt("06");
    DA_SetDevice1TurnOffAt("01");
    DA_SetDevice1State('0');
    Delay_ms(4);
    DA_SetDevice2TurnOnAt("15");
    DA_SetDevice2TurnOffAt("50");  
    DA_SetDevice2State('0');
    Delay_ms(4);
    DA_SetTemperature("05");
    DA_SetHumidity("35"); 
	}
}

#endif