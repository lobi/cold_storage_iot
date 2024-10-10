#include <stdio.h>
#include <string.h>

#include <reg51.h> 
//#include <REGX51.H>
#include "utils.h"
#include "delay.h"
#include "I2C.h"
#include "eeprom.h"
#include "UART.h"
#include "DataAccess.h"
#include "LCD8bit.h"
#include "dht11.h"

// Control:
unsigned char cfg_working_mode;
unsigned char temperature[2], humidity[2];
unsigned char config_dev1_threshold_on[4], cfg_dvc1_threshold_on[4];
unsigned char *ptr_config_dev1_threshold_on = &config_dev1_threshold_on;
unsigned char *ptr_cfg_dvc1_threshold_on = &cfg_dvc1_threshold_on;
//unsigned char cfg_4c[4]; // length == 4
//unsigned char *ptr_cfg_4c = &cfg_4c;


// UART configuration:
char uart_rx_str[16];
char *ptr_uart_rx_str = &uart_rx_str;
int uart_rx_str_size = 0;
// LCD:
//unsigned char arr_char_lcd[16];
//char *ptr_arr_char_lcd = &arr_char_lcd;
// Others:
int i = 0, ms0 = 4, ms1 = 100, ms2 = 200;


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
	cfg_working_mode = DA_GetWorkingMode();
	switch(cfg_working_mode)
	{
		case '1':
			displayText("auto");
			break;
		case '0':
			displayText("manual");
			break;
		default:
			displayChar(cfg_working_mode);
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


/*
Flow:
	- 1. Refresh DHT11 sensor's data to eeprom
	- 2. Read current temperature & humidity data from eeprom and send it to 8266 via UART-TX
	- 3. Control devices base on eeprom data
	- 4. UART-RX and proceed command if data is available
*/
void loop(void)
{
	// 1. Refresh DHT11 sensor's data to eeprom
  clearLine(0);
  displayText("11");
  Delay_ms(ms2);
  Dht_UpdateCurrentMetricsToEeprom();
  clearLine(0);
  displayText("22");
  Delay_ms(ms2);

  // 2.1 Read temperature/humidity
  DA_GetHumidity(&humidity);
  DA_GetTemperature(&temperature);
  clearLine(0);
  displayText("Temp: ");
  displayText(&temperature);
  clearLine(1);
  displayText("Hum: ");
  displayText(&humidity);
  Delay_ms(ms2);

  // 2.2 Send temperature/humidity to 8266(thingsboard) via UART-TX
  // send humidity
  stradd(ptr_uart_rx_str, "003:", 0); // 003: send humidity
  stradd(ptr_uart_rx_str, &humidity, 4);
  //sprintf(uart_rx_str, "003:%s", humidity);
  // UART_Init();
  // UART_TxStr(ptr_uart_rx_str, 5);
  Delay_ms(ms2);
  // send temperature
  stradd(ptr_uart_rx_str, "004:", 0); // 003: send humidity
  stradd(ptr_uart_rx_str, &temperature, 4);
  //sprintf(uart_rx_str, "004:%s", temperature);
  // UART_Init();
  // UART_TxStr(ptr_uart_rx_str, 5);

  //UART_TxString("hello uart 8051");

  // 3.1 Read data configuration to control devices
	// cooling fan:
	DA_GetDevice1TurnOnAt(ptr_config_dev1_threshold_on);
	DA_GetDevice1TurnOffAt(ptr_cfg_dvc1_threshold_on);
	
	// Read current temperature & humidity data from eeprom and send it to 8266 via UART-TX
	clearLine(0);
  clearLine(1);
  displayText("+++");
  Delay_ms(ms2);

  // 4.1 UART-RX and proceed command if data is available
  uart_rx_str_size = 0;
  UART_Init();
  if (1) // check if buffer is available
  {
    clearLine(0);
    displayText("RI:");
    displayChar(RI);
    
    //UART_Init();
    uart_rx_str_size = UART_RXString(ptr_uart_rx_str);
    Delay_ms(850);
  }
	
  clearLine(0);
  clearLine(1);
  displayText("---");
  
  if (uart_rx_str_size > 0)
	{
		// Identify the command
		char cmd[4];
		for (i = 0; i < 4; i++)
		{
			cmd[i] = uart_rx_str[i];
		}
		
		clearLine(0);
		displayText("UART-RX:");
		clearLine(1);
		displayText(ptr_uart_rx_str);
		
		if (strcmp(cmd, "001:") == 0)
		{
			// 001: set working mode
			
			// update to eeprom
			cfg_working_mode = uart_rx_str[4];
			DA_SetWorkingMode(cfg_working_mode); // save to eeprom
			cfg_working_mode = DA_GetWorkingMode(); // check again to make sure it saved to eeprom
			
			// generate response
			stradd(ptr_uart_rx_str, "r01:", 0); // response
			stradd(ptr_uart_rx_str, &cfg_working_mode, 4);
			//stradd(ptr_uart_rx_str, 'a', 5);
			
			// send reponse to confirm
			UART_Init();
			UART_TxStr(ptr_uart_rx_str, 5);
			
			//Delay_ms(ms2);
			//clearLine(1);
			//displayText("sent: 001:");
			//displayText(ptr_uart_rx_str);
			//Delay_ms(ms2);
		}
		else if (strcmp(cmd, "002:") == 0)
		{
			// 002: get working mode
			//cfg_working_mode = DA_GetWorkingMode();
			
			// generate response
			stradd(ptr_uart_rx_str, "r02:", 0); // response
			stradd(ptr_uart_rx_str, &cfg_working_mode, 4);
			//stradd(ptr_uart_rx_str, 'a', 5);
			
			// send current working mode to uart
			UART_Init();
			UART_TxString(ptr_uart_rx_str);
		}
		
		Delay_ms(ms0);
		//clearLine(1);
	}

  
  Delay_ms(ms2);
}

void main(void)
{
	init();
	while(1)
	{
		loop(); 
	}
	
	//setCursor(0, 1);
	//displayText(*config_dev1_threshold_on);
	
	// -----------------------------------
	/*
	//EepromEraseAll();
	//load_sample_data();
	
	char sName [] = "Circuits Today";
	char mode = '1';
	char * temparature = "26.5";
	char * humity = "14.4";
	
	char mem_mode = "2";
	char * ptr_mem_temparature;
	
	EepromEraseAll();
	
	// write mode
	EepromWriteByte(mode, 0, 0);
	// write temparature
	EepromWriteNBytes(0x01, temparature, sizeof(temparature), 0);
	// write humity
	EepromWriteNBytes(0x10, humity, sizeof(humity), 0);
	
	mem_mode = EepromReadByte(0x00, 0);
	///char memmode = EepromReadByte(0, 0);
	///char * ptr_mem_temparature = "00.0"; 
	
	initLCD();
	
	///char mem_mode = EepromReadByte(0, 0);
	///char memmode = EepromReadByte(0, 0);
	//char * ptr_mem_temparature = "00.0"; 
	EepromReadNBytes(0x01, ptr_mem_temparature, sizeof(temparature), 0);
	
	setCursor(0, 0);
	displayChar(mem_mode);
	if (mem_mode == '1')
		displayText("a");
	else
		displayText("b");
	Delay_ms(2000);
	displayText(ptr_mem_temparature);
	Delay_ms(2000);
	
	setCursor(0, 1);
	displayChar(EepromReadByte(0x01, 0));
	displayText(" done.");
	Delay_ms(2000);
	
	while(1);
	
	//setCursor(0, 0);
	//Delay_ms(2000);
	
	////- -----------------------------------------
	
	
	setCursor(0, 0);
	displayText("EEPROM 24C02.."); // initial display on LCD
	
	
	for(i =0; i<14; i++)
		EepromWriteByte(sName[i], i, 0);
	
  setCursor(0, 1);
	for(i =0; i<14; i++)
	{
		char s = EepromReadByte(i,0);

//		if (s == sName[i])
//		{
//			displayChar("'");
//		}
//		else
//		{
//			displayChar("-");
//		}
		
		displayChar(s);
		Delay_ms(500);
	}
	
	setCursor(0, 1);
	displayText("Done");
	*/
	// --------------------------------------------------------
	
	/*
	initLCD();
	setCursor(0, 0);
	displayText("EEPROM 24C02..");
	
	//unsigned char data[12];
  ser_init();
  i2c_send_string(0x00,"EmbeTronicX");
	
	setCursor(0, 1);
	displayText("sent");
	
	Delay_ms(2000);
	
	setCursor(0, 0);
  displayChar(i2c_read_string(0x00));
	
	Delay_ms(3000);
	
  tx_str(rec);
	setCursor(0, 1);
	displayText("Done.");
	*/
}