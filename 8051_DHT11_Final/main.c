#include<reg52.h> 
#include "utils.h"
#include "delay.h"
#include "I2C.h"
#include "eeprom.h"
#include "UART.h"

#include "DataAccess.h"
#include "LCD8bit.h"


#include "LCD16x2_DHT11.h"

// Control:
unsigned char    // gb: Global
    gb_wm = 1,   // working mode
   gb_temp[2],  // temperature
    gb_hum[2],   // humidity
  //  gb_d1on[2],  // device 1 turn on at
 //   gb_d1off[2], // device 1 turn off at
  //  db_d2on[2],  // device 2 turn on at
    gb_d2off[2], // device 2 turn off at
    buf2[2],     // buffer size 2
    buf4[4],     // buffer size 2
    buf16[16];   // bugger size 16, e.g.: for LCD, uart...
gb_i = 0;    // multi purposes, reset before using
char *ptr_buf16 = &buf16;
int i = 0, ms0 = 4, ms1 = 100, ms2 = 200;

// include other libraries that need to use above defined variables

void set_sample_data()
{
	// test set temperature
	//clearLine(1);
	//displayText("sample data: ");
	DA_SetDevice1TurnOnAt("30");
	DA_SetDevice1TurnOffAt("25");
	DA_SetTemperature("05");
	DA_SetHumidity("35");
	DA_SetDevice2TurnOnAt("50");
	DA_SetDevice2TurnOffAt("70");
	//displayText("ok");
	//Delay_ms(ms2);
	//clearLine(1);
}

void init(void)
{
	// LCD
	initLCD();
	
	// Welcome
	setCursor(0, 0);
	displayText("Starting...");
	
	// Data Access
	setCursor(0, 1);
	displayText("Data: ");
  EepromEraseAll();
	DA_Init();
	displayText("ok");
	
	Delay_ms(ms2);
	clearLine(1);
	
	// sample data for testing
	set_sample_data();
	
	displayText("Mode: ");
	gb_wm = DA_GetWorkingMode();
	switch(gb_wm)
  {
  case '1':
    displayText("auto");
    break;
  case '0':
    displayText("manual");
    break;
  default:
    displayChar(gb_wm);
    Delay_ms(8);
    displayText("NaN");
    break;
  }

  Delay_ms(ms2);
	clearLine(1);
	
	// UART
	displayText("UART: ");
	UART_Init();
	UART_TxString("hello uart 8051");
	displayText("hello-2!");
	
	Delay_ms(ms2 * 2);
}


void loop(void)
{
  //char buf2[2];
  // 1. Refresh DHT11 sensor's data to eeprom
  Dht_Update();
  Delay_ms(ms2); //test

  // 2.1 Read temperature/humidity
	/*
  DA_GetHumidity(&gb_hum);
  DA_GetTemperature(&gb_temp);

  memset(buf16, 0, 16);
  sprintf(buf16, "Mem-Hum=%s", gb_hum);
  clearLine(0);
  displayText(buf16);

  memset(buf16, 0, 16);
  sprintf(buf16, "Mem-Tem=%s", gb_temp);
  clearLine(1);
  displayText(buf16);
*/
  // 2.2 Send temperature/humidity to 8266(thingsboard) via UART-TX
  // send humidity to uart
  memset(buf16, 0, 16);
  sprintf(buf16, "003:%s/", gb_hum);
  UART_Init();
  UART_TxStr(buf16, 7);
  Delay_ms(ms0);

  // send temerature to uart
  memset(buf16, 0, 16);
  sprintf(buf16, "004:%s/", gb_hum);
  UART_Init();
  UART_TxStr(buf16, 7);
  Delay_ms(ms2);

  // 3.1 Read data configuration to control devices
	// cooling fan: On at
	/*
	DA_GetDevice1TurnOnAt(&buf2);
  memset(buf16, 0, 16);
  sprintf(buf16, "Mem-d1on=%s", buf2);
  clearLine(0);
  displayText(buf16);
  // cooling fan: Off at
  DA_GetDevice1TurnOffAt(&buf2);
  memset(buf16, 0, 16);
  sprintf(buf16, "Mem-d1off=%s", buf2);
  clearLine(1);
  displayText(buf16);
  Delay_ms(1000);
	*/
	
	// Read current temperature & humidity data from eeprom and send it to 8266 via UART-TX
	clearLine(0);
  clearLine(1);
  displayText("+++");
  Delay_ms(ms2);

  // 4.1 UART-RX and proceed command if data is available
    gb_i = 0;
  if (1) // check if buffer is available
  {
    clearLine(0);
    displayText("RI:");
    displayChar(RI);
    
    UART_Init();
    gb_i = UART_RXString(ptr_buf16);
    Delay_ms(850);
  }
	
  clearLine(0);
  clearLine(1);
  displayText("---");
  
  if (gb_i > 0)
	{
		// Identify the command
    memset(buf4, 0, 4);
    for (i = 0; i < 4; i++)
		{
			buf4[i] = buf16[i];
		}
		
		clearLine(0);
		displayText("UART-RX:");
		clearLine(1);
		displayText(&buf16);
		
		if (strcmp(buf4, "001:") == 0)
		{
			// 001: set working mode
			
			// update to eeprom
			gb_wm = buf16[4];
			DA_SetWorkingMode(gb_wm); // save to eeprom
			gb_wm = DA_GetWorkingMode(); // check again to make sure it saved to eeprom
			
			// generate response
			stradd(ptr_buf16, "r01:", 0); // response
			stradd(ptr_buf16, &gb_wm, 4);
			//stradd(ptr_buf16, 'a', 5);
			
			// send reponse to confirm
			UART_Init();
			UART_TxStr(ptr_buf16, 5);
			
			//Delay_ms(ms2);
			//clearLine(1);
			//displayText("sent: 001:");
			//displayText(ptr_buf16);
			//Delay_ms(ms2);
		}
		else if (strcmp(cmd, "002:") == 0)
		{
			// 002: get working mode
			//gb_wm = DA_GetWorkingMode();
			
			// generate response
			stradd(ptr_buf16, "r02:", 0); // response
			stradd(ptr_buf16, &gb_wm, 4);
			//stradd(ptr_buf16, 'a', 5);
			
			// send current working mode to uart
			UART_Init();
			UART_TxString(ptr_buf16);
		}
		
		Delay_ms(ms0);
		//clearLine(1);
	}

  
  Delay_ms(ms2);
}


void main (void)
{
	init();
	while(1)
	{
		loop(); 
	};
}
