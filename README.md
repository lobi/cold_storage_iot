# Cold Storage Management
Project IOT - Semester 1 - FPT Jetking
## Introduction
This is an IOT project, help to control temperature & humidity in a Cold Storage via internet. Depend on the kind of storage (i.e. fruits, vaccine, food... ), the device can auto maintain temperature & humidity base on the pre-configuration.

## Featured
- Control over internet (Using Thingsboard)
- Switch between Auto & Manual working mode
    - Auto: Setup the temperature & humidity threshold base on the kind of storage, maintain proper temperature/humidity by auto control turn on/off Cooling Fan & Humidifier Machine
    - Manual: Fully control Cooling Fan & Humidifier Machine

## How it work?
![Flowchart](./docs/000.jpg)

Fully control over internet
- Sensor's metrics will be collected by 8051 and saved on EEPROM, 8051 also control the Cooling Fan & Humidifier Machine on or off.
- ESP8266 will help to connect to internet (Thingsboard), transmit data between 8051 & Thingsboard
8051 also help to saved the configurations from Thingsboard to EEPROM

## Thingsboard's dashboard controller
![Dashboard](./docs/003.png)
The metrics of Temperature & humidity will keeping sync-up from DHT11 sensor to Thingsboard.

The dashboard present real time Temperature/Humidity.

The dashboard support to switch between working mode (auto-manual).

Mode Auto: User can config the proper thresholds and the 8051 will auto maintain temperature/humdity by control Cooling Fan/Humidifier Machine.

Mode Manual: Fully control Cooling Fan/Humidifier Machine on or off

Download dashboard json [here](./esp8266/cold_storage_management.json).

## Hardwares
![Hardware](./docs/004.JPG)
- 8051 - MCU
- ESP8051 - Microcontroller
- DHT11 - Sensor
- LCD 16x2
- EEPROM 24C02

### Hardware Protocols used
- UART: transmit data between 8051 & ESP8266
- GPIO: LCD, LED indicators, control Cooling Fan & Humidifier Machine, DHT11 sensor
- I2C: integrate with EEPROM to read & write data

### Configurations
#### ESP8266
```ino
// wifi access
const char* WIFI_SSID = "Xiaomi 13T";
const char* WIFI_PASSWORD = "88888888";

// Thingsboards configuration
const char* THINGSBOARD_TOKEN = "MxkQrzcH3l79OPnAN8Q9";
constexpr char THINGSBOARD_SERVER[] = "thingsboard.cloud";

// UART
SoftwareSerial my_uart(13, 15);  // RX, TX
```

#### 8051
```cpp
// Control Cooling Fan & Humdifier Machine
sbit CTRL_T = P1 ^ 6; // Cooling Fan
sbit CTRL_H = P1 ^ 5; // Humidifier Machine

// LED Indicators
sbit LED1 = P1 ^ 4;   // UART RX indicator
sbit LED2 = P1 ^ 3;   // UART TX indicator
sbit LED_WM = P1 ^ 2; // Working Mode (auto/manual) indicator

// I2C
sbit SDA=P2^0;
sbit SCL=P2^1;

// DHT11 sensor
sbit DHT11 = P1^7;
```

#### UART Command Codes when communicate between 8051 & ESP8266
![UART Command Codes](./docs/005.png)

#### EEPROM Address - Where the data was storaged
![EEPROM Addressing](./docs/006.png)

## Video Demo
[![Cold Storage - IOT](https://img.youtube.com/vi/pO4Ov4dsSpQ/0.jpg)](https://www.youtube.com/watch?v=pO4Ov4dsSpQ)
