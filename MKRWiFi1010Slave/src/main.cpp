#include <Arduino.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <MQTT.h>
#include <Servo.h>

#include <SercomSPISlave.h>
SercomSPISlave SPISlave;

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

void SERCOM1_Handler(void);

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
  while (!mqttclient.connect("HSo1FxEWNzU6NhAAKxwIFgM", "HSo1FxEWNzU6NhAAKxwIFgM", "d689s9sY2O2QN0t+pY1LHlVQ")) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nconnected!");

  mqttclient.subscribe("channels/1718632/subscribe/fields/field3", 0);
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
  mqttclient.begin("mqtt3.thingspeak.com", client);
  mqttclient.onMessage(messageReceived);

  

  // Sercom
  SPISlave.Sercom1init(); // Sercom 1 but with PA18 changed to PA21
  Serial.println("Sercom1 SPI slave initialized");

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
		Serial.println (buff[0]);
    Serial.print("Temp: ");
    Serial.println(buff[1]);
	}

  if (millis() - previusMillis30 > delay30Sek) 
  {
    String payload = "field1=";
		payload += buff[0];
		payload += "&field2=";
    payload += buff[1];
    mqttclient.publish("channels/1718632/publish", payload, false, 0);
    previusMillis30 = millis();
  }
}

void SERCOM1_Handler() // 25.7 Register Summary, page 454 atmel 42181, samd21
{
  #ifdef DEBUG
    Serial.println("In SPI Interrupt");
  #endif
  uint8_t data = 0;
  uint8_t interrupts = SERCOM1->SPI.INTFLAG.reg; //Read SPI interrupt register
  #ifdef DEBUG
    Serial.print("Interrupt: "); Serial.println(interrupts);
  #endif
  
  if(interrupts & (1<<3)) // 8 = 1000 = SSL
  {
    #ifdef DEBUG
      Serial.println("SPI SSL Interupt");
    #endif
    SERCOM1->SPI.INTFLAG.bit.SSL = 1; //clear slave select interrupt
    //data = SERCOM1->SPI.DATA.reg; //Read data register
    #ifdef DEBUG
      Serial.print("DATA: "); Serial.println(data);
    #endif
    //SERCOM1->SPI.INTFLAG.bit.RXC = 1; //clear receive complete interrupt
  }
  
  // This is where data is received, and is written to a buffer, which is used in the main loop
  if(interrupts & (1<<2)) // 4 = 0100 = RXC
  {
    #ifdef DEBUG
      Serial.println("SPI Data Received Complete Interrupt");
    #endif
    data = SERCOM1->SPI.DATA.reg; //Read data register
    
    if (data == 255)
	  {
		  arrayindex = 0;
      process = true;
	  }
    else
    {
      buff[arrayindex] = data-50;
      arrayindex++;
      if (arrayindex >= 2)
      {
        arrayindex = 0;
      }
    }
    #ifdef DEBUG
      Serial.print("DATA: ");
      Serial.println(data);
    #endif
    SERCOM1->SPI.INTFLAG.bit.RXC = 1; //clear receive complete interrupt
  }
  
  if(interrupts & (1<<1)) // 2 = 0010 = TXC
  {
    #ifdef DEBUG
      Serial.println("SPI Data Transmit Complete Interrupt");
    #endif
    SERCOM1->SPI.INTFLAG.bit.TXC = 1; //clear receive complete interrupt
  }
  
  if(interrupts & (1<<0)) // 1 = 0001 = DRE
  {
    #ifdef DEBUG
      Serial.println("SPI Data Register Empty Interrupt");
    #endif
    SERCOM1->SPI.DATA.reg = 0xAA;
    //SERCOM1->SPI.INTFLAG.bit.DRE = 1;
  }
  
  #ifdef DEBUG
    Serial.println("----------");
  #endif
}