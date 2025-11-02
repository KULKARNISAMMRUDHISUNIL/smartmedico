#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

// --- WiFi Credentials ---
const char* ssid = "wifi_name";
const char* password = "password";

// --- ThingSpeak Info ---
String apiKey = "3HYCVRDJNHX7Z5Y0";
const char* server = "http://api.thingspeak.com/update";

// --- Sensor Pins ---
#define PIR_PIN 32
#define ONE_WIRE_BUS 33  // DS18B20 data pin

#define SDA_PIN 13       // MAX30102 SDA
#define SCL_PIN 14       // MAX30102 SCL

// --- DS18B20 Setup ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- MAX30102 Setup ---
MAX30105 particleSensor;

// --- Variables ---
int motionState = 0;
float temperatureC = 0.0;
int32_t heartRate = 0;
int32_t spo2 = 0;

const int bufferLength = 100;
uint32_t irBuffer[bufferLength];
uint32_t redBuffer[bufferLength];

void setup() {
  Serial.begin(115200);

  // Initialize PIR sensor pin
  pinMode(PIR_PIN, INPUT);

  // Initialize DS18B20
  sensors.begin();
  delay(1000);  // give sensor time to initialize

  // Initialize I2C for MAX30102 with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("Initializing MAX30102...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }
  
  // Sensor setup parameters
  byte ledBrightness = 60;
  byte sampleAverage = 4;
  byte ledMode = 2;  // Red + IR
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  Serial.println("MAX30102 ready.");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
}

void loop() {
  // Read PIR sensor
  motionState = digitalRead(PIR_PIN);
  Serial.print("🚶 Motion Detected: ");
  Serial.println(motionState);

  // Read DS18B20 temperature
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);
  Serial.print("🌡 Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  // Read MAX30102 sensor data
  for (int i = 0; i < bufferLength; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
    delay(20);
  }

  int8_t validSPO2;
  int8_t validHeartRate;
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  Serial.print("❤️ Heart Rate: ");
  if (validHeartRate) Serial.print(heartRate); else Serial.print("Invalid");
  Serial.print(" BPM, SpO2: ");
  if (validSPO2) Serial.print(spo2); else Serial.print("Invalid");
  Serial.println("%");

  // Send data to ThingSpeak if WiFi connected
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = server;
    url += "?api_key=" + apiKey;
    url += "&field1=" + String(motionState);
    url += "&field2=" + String(temperatureC);
    url += "&field3=" + (validHeartRate ? String(heartRate) : "0");
    url += "&field4=" + (validSPO2 ? String(spo2) : "0");

    http.begin(url);
    int httpCode = http.GET();
    http.end();

    if (httpCode > 0) {
      Serial.println("✅ Data sent to ThingSpeak!");
    } else {
      Serial.println("❌ Failed to send data.");
    }
  }

  delay(15000);  // ThingSpeak minimum delay
}
