#include <Arduino.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <MQTT.h>
#include <Servo.h>

#define DEBUG // comment this line out to not print debug data on the serial bus

byte buff[2];
int arrayindex = 0;
volatile boolean process = false;

// Servo
Servo minservo;

// MQTT
MQTTClient mqttclient;

// Wifi
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

// delay
const int delay30Sek = 30000;
unsigned long previusMillis30 = 0;
const int delay1Sek = 1000;
unsigned long previusMillis1 = 0;

void messageReceived(String &topic, String &payload) 
{
  Serial.println("incoming: " + topic + " - " + payload);
  
  if (payload == "1")
  {
    minservo.write(180);
  }
  if (payload == "0")
  {
    minservo.write(0);
  }
  
  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void connect() {
  delay(1000);
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nconnecting...");
  while (!mqttclient.connect("Allan", "guest", "guest")) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nconnected!");

  mqttclient.subscribe("remote/servo", 0);
}

void setup()
{  
  Serial.begin(115200);  // Initialize serial
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  WiFi.begin(ssid, pass);

  // Servo
  minservo.attach(14);

  // MQTT
  mqttclient.begin("10.135.16.166", client);
  mqttclient.onMessage(messageReceived);

  connect();
}

void loop()
{
  mqttclient.loop();
  if (!mqttclient.connected()) 
  {
    connect();
  }

  if (process)
	{
		process = false;		// reset the process
    Serial.print("Humidity: ");
		Serial.println (random(20, 30));
    Serial.print("Temp: ");
    Serial.println(random(40, 60));
	}

  if (millis() - previusMillis30 > delay1Sek) 
  {
    String payload = "Test Humidity=";
		payload += random(40, 60);
		payload += ",Temperature=";
    payload += random(20, 30);

    Serial.println(payload);
    mqttclient.publish("influx", payload, false, 0);
    previusMillis30 = millis();
  }
}