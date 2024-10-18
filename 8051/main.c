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
sbit CTRL_T = P1 ^ 6;  // Cooling Fan
sbit CTRL_H = P1 ^ 5;  // Humidifier Machine
sbit LED1 = P1 ^ 4;   // UART RX indicator
sbit LED2 = P1 ^ 3;   // UART TX indicator
sbit LED_WM = P1 ^ 2; // Working Mode (auto/manual) indicator
#include "UART.h"
#include "DataAccess.h"

// Global Variables
unsigned char
    buf2[] = {0, 0}, // buffer size 2, e.g.: for display decimal number...
    buf16[16],       // buffer size 16, e.g.: for LCD, uart...
    gb_i = 0;        // multi purposes, reset before using
int i = 0, ms0 = 4, ms1 = 100, ms2 = 200;

// include other libraries that need to use above defined variables
#include "Controller.h"
#include "dht11.h"


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

/* refresh working mode base on eeprom data */
void refresh_wm(void)
{
  volatile unsigned char wm = '1'; // default: auto

  clearLine(1);
  clearLine(0);
  displayText("Mode: ");
  wm = DA_GetWorkingMode();
	switch(wm)
  {
  case '1':
    LED_WM = 1;
    displayText("Auto");
    break;
  case '0':
    LED_WM = 0;
    displayText("Manual");
    break;
  default:
    LED_WM = 0;
    displayChar(wm);
    Delay_ms(8);
    displayText("NaN");
    break;
  }
}

void init(void)
{
  CTRL_T = 0;
  CTRL_H = 0;
  LED1 = 0;
  LED2 = 0;
  LED_WM = 0;

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

  // set_sample_data(); // sample data for testing

  refresh_wm(); // refresh working mode

  Delay_ms(ms2);
  clearLine(1);

  // UART
  displayText("UART: ");
  UART_Init();
  // Ext_int_Init(); // enable uart serial interrupt
  // Timer0_Init();  // init timer 0

  // UART_TxString("hello uart 8051");
  displayText("Ready!");
  Delay_ms(ms2 * 2);
}

void on_rx(unsigned char *prt)
{
  unsigned char rxcmd[] = {0, 0, 0, 0};
  volatile char txs[6] = {0, 0, 0, 0, 0, 0};
  unsigned char wm = '0';

  strrst(rxcmd, 4);
  strc(rxcmd, prt, 4);
  
  // clearLine(1);
  // displayText(prt);

  // required to have a litle bit delay before continue:
  Delay_ms(ms0);

  if (strcmp(rxcmd, "000:") == 0)
  {
    LED2 = 1;

    // First, refresh sensor data & controller
    Dht_Update();

    // Then, send it
    send_metrics();
  }
  else if (strcmp(rxcmd, "001:") == 0)
  {
    LED2 = 1;
    // 001: set working mode
    
    // First, update to eeprom
    DA_SetWorkingMode(prt[4]);
    Delay_ms(4);

    // Next, generate response content
    stradd(txs, "001:", 0, 4);
    txs[4] = DA_GetWorkingMode();
    Delay_ms(4);

    /*
    Then, send the response
    Response to uart for confirmation, no need for now 
    but it could be useful in the future
    */
    UART_Init();
    UART_TxString(txs);

    // Finally, refresh the working mode
    refresh_wm();
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

    // Finally, refresh sensor data & controller:
    Dht_Update();
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

    // Finally, refresh sensor data & controller:
    Dht_Update();
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

    // Finally, refresh sensor data & controller:
    Dht_Update();
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

    // Finally, refresh sensor data & controller:
    Dht_Update();
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
    
    // only update to controller if mode == manual
    if (DA_GetWorkingMode() == '0')
    {
    
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

      // update to controller:
      Delay_ms(ms0);
      ctrl_t(DA_GetDevice1State());
    }
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

    // only update to controller if mode == manual
    if (DA_GetWorkingMode() == '0')
    {
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

      // update to controller:
      Delay_ms(ms0);
      ctrl_h(DA_GetDevice2State());
    }
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

  Delay_ms(ms1); // to have enough time to see the LED turn on
  LED2 = 0;
  LED1 = 0;
  //clearLine(1);
}

void urx()
{
  int icount = 0;
  LED1 = 0;
  //LED2 = 0;

  // clearLine(0);
  // displayText("RX...");
  // clearLine(1);
  
  // On the Thingsboard, there is a schedule (interval 1 min) that send a RPC request (to refresh DHT11 - cmd: 000:2) to ESP8266
  // Then, ESP8266 will send the message to 8051 via UART. It will break the while loop below base on the icount
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
      //urx();// recuision until no RX 
      icount++;
    }
  }

  LED1 = 0;
}

void loop(void)
{
  // reset indicator
  // LED1 = 0;
  // LED2 = 0;

  // Refresh DHT11 sensor's data to eeprom
  Dht_Update();
  Delay_ms(ms1);

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