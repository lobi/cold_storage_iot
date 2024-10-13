#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

#define DHT11_PIN 5 // Pin D1 cho cảm biến DHT11
DHT dht(DHT11_PIN, DHT11); // Khởi tạo cảm biến DHT

const char* ssid = "FAI'S EXAM 2.4GHz";
const char* password = "Fptacademy@2023";
const char* token = "i8vQ3OtHcr3hCdmDlrtG";

const int soilMoisturePin = A0; // Pin kết nối cảm biến độ ẩm đất A0
const int lightPin1 = 2; // Pin điều khiển đèn 1 D5
const int lightPin2 = 14; // Pin điều khiển đèn 2
const int pinrelay = 12;
const int led = 0;
const int photoresistorPin = 16; // Pin kết nối cảm biến quang trở

int percent;

bool controlMode = true;
bool lightState1 = false;
bool lightState2 = false;

#include <SoftwareSerial.h> // UART - 8051
SoftwareSerial s(13, 15); // UART - 8051

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSend;

void setup_wifi() {
    Serial.begin(115200);
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String payloadStr;
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, payloadStr);
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }
    bool state = doc["params"];
    String method = doc["method"];
    if (method == "changeMode2") {
        controlMode = state;
        Serial.print("Control mode changed to: ");
        Serial.println(controlMode);
        sendControlModeToThingsBoard();
        return;
    }

    if (controlMode) {
        // Chế độ tự động
        if (payloadStr.indexOf("setLight1.1") > -1) {
            lightState1 = payloadStr.indexOf("\"lightState1.1\":true") > -1;
            digitalWrite(lightPin1, lightState1 ? HIGH : LOW);
            Serial.print("Received setLight1.1 command, lightState1: ");
            Serial.println(lightState1 ? "true" : "false");
            String msg = "{\"value1\": " + String(lightState1 ? "true" : "false") + "}";
            client.publish("v1/devices/me/attributes", msg.c_str());
        }
        if (payloadStr.indexOf("setLight2.1") > -1) {
            lightState2 = payloadStr.indexOf("\"lightState2.1\":true") > -1;
            digitalWrite(lightPin2, lightState2 ? HIGH : LOW);
            Serial.print("Received setLight2.1 command, lightState2: ");
            Serial.println(lightState2 ? "true" : "false");
            String msg = "{\"value2\": " + String(lightState2 ? "true" : "false") + "}";
            client.publish("v1/devices/me/attributes", msg.c_str());
        }
    } else {
        // Chế độ thủ công
        if (method == "setLight1") {
            lightState1 = state;
            digitalWrite(lightPin1, lightState1 ? LOW : HIGH);
            Serial.println(lightState1 ? "Light 1 ON" : "Light 1 OFF");

            //s.write(lightState1 ? 0x01 : 0x00); // Gửi tín hiệu điều khiển đèn 1 qua UART đến 8051

            String msg = "{\"value1\": " + String(lightState1 ? "true" : "false") + "}";
            client.publish("v1/devices/me/attributes", msg.c_str());
        }
        if (method == "setLight2") {
            lightState2 = state;
            digitalWrite(lightPin2, lightState2 ? HIGH : LOW);
            Serial.println(lightState2 ? "Light 2 ON" : "Light 2 OFF");

            //s.write(lightState2 ? 0x02 : 0x03); // Gửi tín hiệu điều khiển đèn 2 qua UART đến 8051

            String msg = "{\"value2\": " + String(lightState2 ? "true" : "false") + "}";
            client.publish("v1/devices/me/attributes", msg.c_str());
        }
    }
}

void sendControlModeToThingsBoard() {
    String msg = "{\"controlMode\": \"" + String(controlMode ? "true" : "false") + "\"}";
    client.publish("v1/devices/me/attributes", msg.c_str());
}

void setup() {
    pinMode(lightPin1, OUTPUT);
    pinMode(lightPin2, OUTPUT);
    pinMode(pinrelay, OUTPUT);
    pinMode(photoresistorPin, INPUT_PULLUP);  
    pinMode(led, OUTPUT);
    
    s.begin(9600); // UART - 8051
    //Serial.begin(9600);
    
    setup_wifi();
    dht.begin(); // Khởi động cảm biến DHT
    client.setServer("thingsboard.cloud", 1883);
    client.setCallback(callback);
    lastSend = 0;
}

void loop() {
    int sensorphotoresistor = digitalRead(photoresistorPin); // Đọc giá trị cảm biến quang trở
    if(sensorphotoresistor == 1)
    {
        digitalWrite(led, HIGH);
         s.write(0x01);
    }
    else 
    {
        digitalWrite(led, LOW);
        s.write(0x00);
    }
    if (percent < 25) // Đọc cảm biến độ ẩm đất
    {
        digitalWrite(pinrelay, HIGH);
        s.write(0x03);
    }
    else
    {
        digitalWrite(pinrelay, LOW);
        s.write(0x02);
    }
    // Gửi giá trị cảm biến quang trở qua UART
    //s.write(sensorphotoresistor == 1 ? 0x01 : 0x00); 

    if (!client.connected()) {
        reconnect();
    }
    if (millis() - lastSend > 2000) {
        getAndSendSensorData();
        lastSend = millis();
    }
    client.loop();
}

void getAndSendSensorData() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    char buffer[50];
 
    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    int soilMoistureValue = analogRead(soilMoisturePin); // Đọc giá trị từ cảm biến độ ẩm đất
    percent = map(soilMoistureValue, 350, 1023, 0, 100); // Chuyển đổi giá trị sang phần trăm
    percent = 100 - percent; // Tính giá trị phần trăm thực

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    Serial.print("Soil Moisture: ");
    Serial.println(percent);

    // Gửi giá trị cảm biến nhiệt độ, độ ẩm, độ ẩm đất qua UART đến 8051
    sprintf(buffer, "T:%.1fH:%.1fSM:%d", t, h, percent);
    s.print(buffer);

    String payload = "{";
    payload += "\"temperature\":"; payload += t; payload += ",";
    payload += "\"humidity\":"; payload += h; payload += ",";
    payload += "\"soil moisture\":"; payload += percent;
    payload += "}";

    char telemetry[150];
    payload.toCharArray(telemetry, 150);
    client.publish("v1/devices/me/telemetry", telemetry);
    Serial.println(telemetry);

    // Gửi trạng thái chế độ và các đèn lên ThingsBoard
    sendControlModeToThingsBoard();
    String msg = "{\"value1\": " + String(lightState1 ? "true" : "false") + ", \"value2\": " + String(lightState2 ? "true" : "false") + "}";
    client.publish("v1/devices/me/attributes", msg.c_str());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP8266Client", token, NULL)) {
            Serial.println("connected");
            client.subscribe("v1/devices/me/rpc/request/+");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}