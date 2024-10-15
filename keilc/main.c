#include <reg52.h> 
//#include <reg51.h>
//#include <REGX51.H>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "delay.h"
#include "I2C.h"
#include "eeprom.h"
#include "LCD8bit.h"
sbit LED1 = P1 ^ 6;
sbit LED2 = P1 ^ 5;
#include "UART.h"
#include "DataAccess.h"



// Control:
unsigned char    // gb: Global
    //gb_wm = 1,   // working mode
    gb_temp[2],  // temperature
    gb_hum[2],   // humidity
    //gb_d1on[2],  // device 1 turn on at
    //gb_d1off[2], // device 1 turn off at
    //db_d2on[2],  // device 2 turn on at
    //gb_d2off[2], // device 2 turn off at
    buf2[2],     // buffer size 2
    buf4[4],     // buffer size 4
    //buf5[5],     // buffer size 5
    buf16[16],   // buffer size 16, e.g.: for LCD, uart...
    gb_i = 0;    // multi purposes, reset before using
int i = 0, ms0 = 4, ms1 = 100, ms2 = 200;
// include other libraries that need to use above defined variables

#include "dht11.h"


void set_sample_data()
{
	// test set temperature
	//clearLine(1);
	//displayText("sample data: ");
	DA_SetDevice1TurnOnAt("06");
	DA_SetDevice1TurnOffAt("01");
	DA_SetTemperature("05");
	DA_SetHumidity("35");
	//displayText("ok");
	//Delay_ms(ms2);
	//clearLine(1);
}

void send_metrics(void)
{
  // send humidity
  Delay_ms(ms0);
  strrst(buf2, 2);
  DA_GetHumidity(buf2); // retrieve data from eeprom
  Delay_ms(ms1);
  strrst(buf16, 16); // reset
  stradd(buf16, "003:", 0, 4);
  stradd(buf16, buf2, 4, 2);
  UART_Init();
  //UART_TxStr(buf16, 6); // not work on kit
  UART_TxString("003:32\n"); // hard code for testing

  Delay_ms(ms1);

  // send temperature
  Delay_ms(ms2);
  strrst(buf2, 2);
  DA_GetTemperature(buf2); // retrieve data from eeprom
  Delay_ms(ms1);
  strrst(buf16, 16); // reset
  stradd(buf16, "004:", 0, 4);
  stradd(buf16, buf2, 4, 2);
  UART_Init();
  //UART_TxStr(buf16, 6); // not work on kit
  UART_TxString("004:03\n"); // hard code for testing
  Delay_ms(ms0);

  // lcd for testing
  clearLine(0);
  displayText("Mem-Hum:");
  DA_GetHumidity(buf2);
  displayText(buf2);
  clearLine(1);
  displayText("Mem-Tem:");
  DA_GetTemperature(buf2);
  displayText(buf2);
  Delay_ms(ms2);
}

void init(void)
{
  volatile unsigned char wm = '1'; // default: auto
  LED1 = 0;
  LED2 = 0;

  // LCD
	initLCD();
	
	// Welcome
	setCursor(0, 0);
	displayText("Starting...");
	
	// Data Access
	setCursor(0, 1);
	displayText("Data: ");
  //EepromEraseAll();
	DA_Init();
	displayText("ok");
	
	Delay_ms(ms2);
	clearLine(1);
	
	// sample data for testing
	set_sample_data();
	
	displayText("Mode: ");
	wm = DA_GetWorkingMode();
	switch(wm)
  {
  case '1':
    displayText("auto");
    break;
  case '0':
    displayText("manual");
    break;
  default:
    displayChar(wm);
    Delay_ms(8);
    displayText("NaN");
    break;
  }

  Delay_ms(ms2);
	clearLine(1);
	
	// UART
	displayText("UART: ");
  UART_Init();
  //Ext_int_Init(); // enable uart serial interrupt
  //Timer0_Init();  // init timer 0

  //UART_TxString("hello uart 8051");
	displayText("hello-2!");
	
	Delay_ms(ms2 * 2);
}

void on_rx(unsigned char *prt)
{
  unsigned char rxcmd[] = {0, 0, 0, 0};
  unsigned char txs[6] = {0, 0, 0, 0, 0, 0};
  unsigned char wm = '0';

  strrst(rxcmd, 4);
  strc(rxcmd, prt, 4);

  clearLine(1);
  displayText(prt);
  if (strcmp(rxcmd, "001:") == 0)
  {
    LED2 = 1;
    // 001: set working mode
    
    // update to eeprom
    DA_SetWorkingMode(prt[4]);
    Delay_ms(4);

    // generate response content
    stradd(txs, "001:", 0, 4);
    txs[4] = DA_GetWorkingMode();
    Delay_ms(4);

    // send to uart for confirmation
    UART_Init();
    UART_TxString(txs);
  }
  else if (strcmp(rxcmd, "002:") == 0)
  {
    LED2 = 1;
    // 002: get working mode

    // generate response content
    stradd(txs, "002:", 0, 4);
    txs[4] = DA_GetWorkingMode();
    Delay_ms(4);

    // send to uart for confirmation
    UART_Init();
    UART_TxStr(txs, 6);
  }
  
  Delay_ms(ms2); // to have enough time to see the LED turn on
  LED2 = 0;
  LED1 = 0;
  //clearLine(1);
}

void urx()
{
  int icount = 0;
  LED1 = 0;
  LED2 = 0;
  

  clearLine(0);
  displayText("RX...");
  
  while (icount < 3)
  {
    Delay_ms(ms0);
    strrst(buf16, 16);
    gb_i = 0;
    UART_Init();
    gb_i = UART_RXString(buf16);
    if (gb_i > 0)
    {
      LED1 = 1;
      on_rx(buf16);
      icount++;
    }
  }
  LED1 = 0;
}

/*
Flow:
	- 1. Refresh DHT11 sensor's data to eeprom
	- 2. Read current temperature & humidity data from eeprom and send it to 8266 via UART-TX
	- 3. Control devices base on eeprom data
	- 4. UART-RX and proceed command if data is available
*/
void loop(void)
{
  LED1 = 0;
  strrst(buf16, 16);
  //char buf2[2];
  // 1. Refresh DHT11 sensor's data to eeprom
  //Dht_Update();
  //Delay_ms(ms2); //test

  // 2.1 Read temperature/humidity
  send_metrics();

    // 3.1 Read data configuration to control devices
	// cooling fan: On at
	DA_GetDevice1TurnOnAt(&buf2);
  //memset(buf16, 0, 16);
  sprintf(buf16, "Mem-d1on=%s", buf2);
  clearLine(0);
  displayText(buf16);
  // cooling fan: Off at
  DA_GetDevice1TurnOffAt(&buf2);
  //memset(buf16, 0, 16);
  sprintf(buf16, "Mem-d1off=%s", buf2);
  clearLine(1);
  displayText(buf16);
  Delay_ms(ms2);
	
	// Read current temperature & humidity data from eeprom and send it to 8266 via UART-TX
	clearLine(0);
  clearLine(1);
  displayText("+++");
  //Delay_ms(ms2);

  // 4.1 UART-RX and proceed command if data is available
  urx();

  //gb_i = 0;
  // if (0)
  // {
  //   gb_i = UART_RXString(buf16); // blocking forever

  //   clearLine(0);
  //   displayText("RX:");
  //   clearLine(1);
  //   displayText(buf16);
  //   Delay_ms(ms2);

  //   if (gb_i > 0)
  //   {
  //     on_uart(buf16);
  //   }
  // }
	
  clearLine(0);
  clearLine(1);
  displayText("---");
  
  Delay_ms(ms2);
}



void main(void)
{
	init();
	while(1)
	{
		loop(); 
	}
}