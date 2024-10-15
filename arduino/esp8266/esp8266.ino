#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

/* Wifi configuration */
const char* WIFI_SSID = "BALOI";           //"FAI'S EXAM 2.4GHz"
const char* WIFI_PASSWORD = "0909286456";  //"Fptacademy@2023"

/* Thingsboards configuration */
const char* THINGSBOARD_TOKEN = "MxkQrzcH3l79OPnAN8Q9";
constexpr char THINGSBOARD_SERVER[] = "thingsboard.cloud";
constexpr uint16_t THINGSBOARD_PORT = 1883U;
constexpr char CONNECTING_MSG[] = "Connecting to: (%s) with token (%s)\n";
//constexpr char TEMPERATURE_KEY[] = "temperature";
//constexpr char HUMIDITY_KEY[] = "humidity";
String TEMPERATURE_KEY = "temperature";
String HUMIDITY_KEY = "humidity";

WiFiClient espClient;
PubSubClient client(espClient);


/*  UART - to transmit & receive serial data with 8051 */
#include <SoftwareSerial.h>
//SoftwareSerial my_uart(3, 1); // RX, TX
SoftwareSerial my_uart(13, 15);  // RX, TX


/* DHT for testing first */
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 4  // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;


/* Generaal configuration */
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;


void init_wifi() {
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.println("Connected to AP");
    Serial.println("ESP8266's IP address: ");
    Serial.print(WiFi.localIP());
  }
  Serial.println(" connected!");
}

bool reconnect_wifi() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  init_wifi();
  return true;
}

bool reconnect_thingsboard() {
  // Reconnect to the ThingsBoard server,
  // if a connection was disrupted or has not yet been established
  Serial.printf(CONNECTING_MSG, THINGSBOARD_SERVER, THINGSBOARD_TOKEN);
  if (!client.connect("ESP8266Client", THINGSBOARD_TOKEN, NULL)) {
    Serial.println("Failed to connect");
    return false;
  }

  // subcribe RPC
  client.subscribe("v1/devices/me/rpc/request/+");
  Serial.println("subcribed to thingsboard.io");
  return true;
}

void init_dht11() {
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("°C"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("°C"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("%"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("%"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
}

void send_sample_dht_metrics() {
  sensors_event_t event;

  // Get temperature event and print its value.
  // Serial.println("Sending temperature data...");
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    //tb.sendTelemetryData(TEMPERATURE_KEY, event.temperature);
    String payload = "{";
    payload += "\"" + TEMPERATURE_KEY;
    payload += "\":";
    payload += event.temperature;
    payload += "}";
    char telemetry[150];
    payload.toCharArray(telemetry, 100);
    client.publish("v1/devices/me/telemetry", telemetry);
    // Serial.print(F("Temperature: "));
    // Serial.print(event.temperature);
    // Serial.println(F("°C"));
  }

  // Get humidity event and print its value.
  // Serial.println("Sending humidity data...");
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    //tb.sendTelemetryData(HUMIDITY_KEY, event.relative_humidity);
    String payload = "{";
    payload += "\"" + HUMIDITY_KEY;
    payload += "\":";
    payload += event.relative_humidity;
    payload += "}";
    char telemetry[150];
    payload.toCharArray(telemetry, 100);
    client.publish("v1/devices/me/telemetry", telemetry);
    // Serial.print(F("Humidity: "));
    // Serial.print(event.relative_humidity);
    // Serial.println(F("%"));
  }
  //Serial.println("dht sent.");
}

void send_metrics(String m_key, String m_val) {
  String payload = "{";
    payload += "\"" + m_key;
    payload += "\":";
    payload += m_val;
    payload += "}";
    char telemetry[150];
    payload.toCharArray(telemetry, 100);
    client.publish("v1/devices/me/telemetry", telemetry);
}

/*
  8051 <-uart-> 8266 <-wifi-> Thingsboard MQTT
  Thingsboard MQTT: Attributes API & RPC API
*/

/*
  Data interchange format between UARTs, 
  Command code vs. Thingsboard's methodName convention

  8051 Control Mode (auto/manual)
  - 001: setControlMode
  - 002: getControlMode
*/

// Thingsboard, the callback for when a PUBLISH message is received from the server.
void on_message(const char* topic, byte* payload, unsigned int length) {

  Serial.println("On message");

  char json[length + 1];
  strncpy(json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(json);

  // Decode JSON request
  // Docs ref.: https://arduinojson.org/
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.println("deserializeJson() returned: ");
    Serial.print(error.c_str());
    return;
  }

  deserializeJson(doc, json);

  // Check request method
  String methodName = String((const char*)doc["method"]);
  Serial.print("parsed methodName: ");
  Serial.println(methodName);

  if (methodName.equals("setControlMode")) {
    // Reply with GPIO status
    //String responseTopic = String(topic);
    //responseTopic.replace("request", "response");
    //client.publish(responseTopic.c_str(), get_gpio_status().c_str());

    char params = doc["params"] ? '1' : '0';
    //String params = doc["params"];
    send_to_uart_8051("001", &params, 1);
  } else if (methodName.equals("getControlMode")) {
    // Update GPIO status and reply
    //set_gpio_status(data["params"]["pin"], data["params"]["enabled"]);
    //String responseTopic = String(topic);
    //responseTopic.replace("request", "response");
    //client.publish(responseTopic.c_str(), get_gpio_status().c_str());
    //client.publish("v1/devices/me/attributes", get_gpio_status().c_str());

    // send command to 8015. the response & action from  8051 will proceed in listen_on_uart_8051()
    char params = ' ';
    send_to_uart_8051("002", &params, 1);
  }
}

// Listen to 8051 (UART - RX)
void listen_on_uart_8051() {
  int incomingByte = 0;
  if (my_uart.available()) {
    Serial.print("Received from 8051 via UART: ");

    String str_rx = my_uart.readString();
    String cmd = str_rx.substring(0, 3);
    String val = str_rx.substring(4, 5);
    
    Serial.println("uart cmd: " + cmd);
    if (cmd == "003:") {
      send_metrics(HUMIDITY_KEY, val);
      Serial.println("telemetry HUMIDITY_KEY was sent: " + cmd);
    }

    // while (my_uart.available())
    // {
    //   Serial.print(" ");
    //   Serial.print(my_uart.read());
    // }
    // Serial.println("");
  }
}

// Send data to 8051 (UART - TX)
void send_to_uart_8051(const char* method, char* data, unsigned int length) {
  //if (method == "001") {
  // 001: setControlMode

  // 100: for method, 2: for 2 other characters[':', '/']
  int payload_length = 100 + length + 2;

  String payload = method;
  payload += ":";
  payload += data;
  payload += "/";  // ending indicator

  Serial.print("sending to uart... ");
  Serial.print(payload);

  // convert payload to char[]
  char char_payload[payload_length];
  payload.toCharArray(char_payload, payload_length);

  // send
  my_uart.write(char_payload);

  Serial.println(" done!");
  //}
}

void setup() {
  // Initalize serial connection for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);

  // wifi
  init_wifi();

  /* UART */
  my_uart.begin(9600);  //same with 8051
  my_uart.println("Hello, world?");

  /* DHT11 on ESP8266 - just for testing. In fact, we receive temperature from 8051 via UART */
  init_dht11();

  // Thingsboard callback
  client.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);
  client.subscribe("v1/devices/me/rpc/request/+");
  client.setCallback(on_message);
  Serial.println("Initial subcribed to thingsboard!");
}

void loop() {
  // put your main code here, to run repeatedly:

  /* connect to wifi */
  if (!reconnect_wifi()) {
    return;
  }

  /* connect to thingsboard */
  if (!client.connected()) {
    if (!reconnect_thingsboard()) {
      return;
    }
  }

  /* listen 8051 via UART (TX) */
  listen_on_uart_8051();

  /* Just for practice DHT11 on ESP8266, 
    remove this when use metrics from 8051 */
  //send_sample_dht_metrics();

  // required when using MQTT
  client.loop();

  delay(2000);
}
