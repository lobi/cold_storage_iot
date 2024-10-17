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
unsigned char
    buf2[] = {0, 0},
    buf16[16],   // buffer size 16, e.g.: for LCD, uart...
    gb_i = 0;    // multi purposes, reset before using
int i = 0, ms0 = 4, ms1 = 100, ms2 = 200;
// include other libraries that need to use above defined variables

#include "dht11.h"


// void set_sample_data()
// {
// 	DA_SetDevice1TurnOnAt("06");
// 	DA_SetDevice1TurnOffAt("01");
//   DA_SetDevice1State("0");

//   DA_SetDevice2TurnOnAt("15");
//   DA_SetDevice2TurnOffAt("50");  
//   DA_SetDevice2State("0");

// 	DA_SetTemperature("05");
// 	DA_SetHumidity("35"); 
// }

void send_metrics(void)
{
  char sh[2] = {'\0', '\0'};
  char st[2] = {'\0', '\0'};

  // send humidity
  Delay_ms(ms0);
  DA_GetHumidity(sh); // retrieve data from eeprom
  Delay_ms(ms0);
  UART_Init();
  UART_TxStr("003:", 4);
  UART_TxString(sh);

  // send temperature
  Delay_ms(ms0);
  DA_GetTemperature(st); // retrieve data from eeprom
  Delay_ms(ms0);
  UART_Init();
  UART_TxStr("004:", 4);
  UART_TxString(st);

  Delay_ms(ms1);
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
  EepromEraseAll();
	DA_Init();
	displayText("ok");
	
	Delay_ms(ms2);
	clearLine(1);
	
	// sample data for testing
	//set_sample_data();
	
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
  volatile char txs[6] = {0, 0, 0, 0, 0, 0};
  unsigned char wm = '0';

  strrst(rxcmd, 4);
  strc(rxcmd, prt, 4);

  clearLine(1);
  displayText(prt);
  if (strcmp(rxcmd, "000") == 0)
  {
    LED2 = 1;
    send_metrics();
  }
  else if (strcmp(rxcmd, "001:") == 0)
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

    // response to uart for confirmation
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
    Delay_ms(ms0);

    // send to uart for confirmation
    UART_Init();
    UART_TxString(txs);
  }
  else if (strcmp(rxcmd, "003:") == 0)
  {
    // 003: telemetry: humidity
    LED2 = 1;
    // generate response content
    strrst(buf2, 2);
    DA_GetHumidity(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("003:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "004:") == 0)
  {
    // 004: telemetry: temperature
    LED2 = 1;
    // generate response content
    strrst(buf2, 2);
    DA_GetTemperature(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("004:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "005:") == 0)
  {
    // 005: setDevice1OnAt
    LED2 = 1;
    strrst(buf2, 2);
    getRxVal(prt, buf2, 2);
    DA_SetDevice1TurnOnAt(buf2);
    Delay_ms(ms0);

    // generate response content
    strrst(buf2, 2);
    DA_GetDevice1TurnOnAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("005:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "006:") == 0)
  {
    // 006: getDevice1OnAt
    LED2 = 1;
    // generate response content
    strrst(buf2, 2);
    DA_GetDevice1TurnOnAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("006:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "007:") == 0)
  {
    // 007: setDevice1OffAt
    LED2 = 1;
    strrst(buf2, 2);
    getRxVal(prt, buf2, 2);
    DA_SetDevice1TurnOffAt(buf2);
    Delay_ms(ms0);

    // generate response content
    strrst(buf2, 2);
    DA_GetDevice1TurnOffAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("007:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "008:") == 0)
  {
    // 008: getDevice1OffAt
    LED2 = 1;
    // generate response content
    strrst(buf2, 2);
    DA_GetDevice1TurnOffAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("008:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "009:") == 0)
  {
    // 009: setDevice2OnAt
    LED2 = 1;
    strrst(buf2, 2);
    getRxVal(prt, buf2, 2);
    DA_SetDevice2TurnOnAt(buf2);
    Delay_ms(ms0);

    // generate response content
    strrst(buf2, 2);
    DA_GetDevice2TurnOnAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("009:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "010:") == 0)
  {
    // 010: getDevice2OnAt
    LED2 = 1;
    // generate response content
    strrst(buf2, 2);
    DA_GetDevice2TurnOnAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("010:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "011:") == 0)
  {
    // 011: setDevice2OffAt
    LED2 = 1;
    strrst(buf2, 2);
    getRxVal(prt, buf2, 2);
    DA_SetDevice2TurnOffAt(buf2);
    Delay_ms(ms0);

    // generate response content
    strrst(buf2, 2);
    DA_GetDevice2TurnOffAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("011:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "012:") == 0)
  {
    // 012: getDevice2OffAt
    LED2 = 1;
    // generate response content
    strrst(buf2, 2);
    DA_GetDevice2TurnOffAt(buf2);
    Delay_ms(ms0);

    // response to uart
    UART_Init();
    UART_TxStr("012:", 4);
    UART_TxString(buf2);
  }
  else if (strcmp(rxcmd, "013:") == 0)
  {
    LED2 = 1;
    // 013: setDevice1State
    
    // update to eeprom
    DA_SetDevice1State(prt[4]);
    Delay_ms(4);

    // generate response content
    stradd(txs, "013:", 0, 4);
    txs[4] = DA_GetDevice1State();
    Delay_ms(4);

    // response to uart for confirmation
    UART_Init();
    UART_TxString(txs);
  }
  else if (strcmp(rxcmd, "014:") == 0)
  {
    LED2 = 1;
    // 014: getDevice1State

    // generate response content
    stradd(txs, "014:", 0, 4);
    txs[4] = DA_GetDevice1State();
    Delay_ms(ms0);

    // send to uart for confirmation
    UART_Init();
    UART_TxString(txs);
  }
  else if (strcmp(rxcmd, "015:") == 0)
  {
    LED2 = 1;
    // 015: setDevice2State
    
    // update to eeprom
    DA_SetDevice2State(prt[4]);
    Delay_ms(4);

    // generate response content
    stradd(txs, "015:", 0, 4);
    txs[4] = DA_GetDevice2State();
    Delay_ms(4);

    // response to uart for confirmation
    UART_Init();
    UART_TxString(txs);
  }
  else if (strcmp(rxcmd, "016:") == 0)
  {
    LED2 = 1;
    // 016: getDevice2State

    // generate response content
    stradd(txs, "016:", 0, 4);
    txs[4] = DA_GetDevice2State();
    Delay_ms(ms0);

    // send to uart for confirmation
    UART_Init();
    UART_TxString(txs);
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
  clearLine(1);
  
  while (icount < 7) // 7: we have 7 requests from thingsboards to get device's states
  {
    Delay_ms(ms1);
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

void loop(void)
{
  LED1 = 0;
  //strrst(buf16, 16);
  //char buf2[2];
  // Refresh DHT11 sensor's data to eeprom
  Dht_Update();
  Delay_ms(3 * ms2);

  // Read temperature/humidity
  send_metrics();
  Delay_ms(ms0);
	
  // UART-RX and proceed command if data is available
  urx();
  
  Delay_ms(ms1);
}



void main(void)
{
	init();
	while(1)
	{
		loop(); 
	}
}