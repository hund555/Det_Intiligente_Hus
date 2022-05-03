#include <Arduino.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

#include <SercomSPISlave.h>
SercomSPISlave SPISlave;

#define DEBUG // comment this line out to not print debug data on the serial bus

byte buff[2];
int arrayindex = 0;
volatile boolean process = false;


// Wifi
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

// ThingSpeak
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;


// delay
const int delay20Sek = 20000;
unsigned long previusMillis20 = 0;

void SERCOM1_Handler(void);

void setup()
{  
  Serial.begin(115200);  // Initialize serial
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.0.0") 
  {
    Serial.println("Please upgrade the firmware");
  }

  ThingSpeak.begin(client);  //Initialize ThingSpeak

  SPISlave.Sercom1init(); // Sercom 1 but with PA18 changed to PA21
  Serial.println("Sercom1 SPI slave initialized");
}

void loop()
{
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  if (process)
	{
		process = false;		// reset the process
    Serial.print("Humidity: ");
		Serial.println (buff[0]);
    Serial.print("Temp: ");
    Serial.println(buff[1]);
	}

  if (millis() - previusMillis20 > delay20Sek) 
  {
    //int hr = round(h); // Humidity rounded
		//int tr = round(t); // Temperature rounded
    
    // set the fields with the values
    ThingSpeak.setField(1, buff[0]);
    ThingSpeak.setField(2, buff[1]);

    // set the status
    ThingSpeak.setStatus("Data has been sent");

    previusMillis20 = millis();

    // write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200)
    {
      Serial.println("Channel update successful.");
    }
    else 
    {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
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