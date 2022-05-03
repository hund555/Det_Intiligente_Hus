#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3231.h>
#include <DHT.h>

// Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// clock
DS3231 clock;
RTCDateTime dt;
bool showTime = false;


// DHT11
#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// DHT11 data
int h = 0;
int t = 0;


// delays
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis10 = 0;
int delay2Sec = 2000;

// funktion prodotyping
String printTime(void);
void buttonSwich(void);

void setup (void)
{
	Serial.begin(115200);												//set baud rate to 115200 for usart
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));	// Set configuration for SPI
	SPI.begin();
	
	clock.begin();														// Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
	
	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{ // Address for 128x64
		Serial.println(F("SSD1306 allocation failed"));
		for(;;); // Don't proceed, loop forever
	}
	
	// DHT11
	dht.begin();
	
	// interrupt for button
	pinMode(3, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(3), buttonSwich, CHANGE);
}

void loop (void)
{
	display.clearDisplay();
	
	display.setTextSize(2);			// Normal 1:1 pixel scale
	display.setTextColor(WHITE);	// Draw white text
	display.setCursor(0,0);			// Start at top-left corner
	if (showTime)
	{
		display.println(printTime());
	}
	else
	{
		if (millis() - previousMillis2 >= delay2Sec)
		{
			previousMillis2 = millis();
			
			h = round(dht.readHumidity());
			t = round(dht.readTemperature());
			
			
			byte data[] = {h+50, t+50, 255}; //255 is for data sorting on slave
			
			digitalWrite(SS, LOW);				// enable Slave Select
			
			SPI.transfer(data[0]);
			SPI.transfer(data[1]);
			SPI.transfer(data[2]);
			
			digitalWrite(SS, HIGH);				// disable Slave Select
		}
		
		String dstr = "Temp: ";					//dstr = Display show temperature rounded
		dstr += t;
		dstr += (char)247;
		
		display.println(dstr);
	}
	
	display.display();
}

String printTime()
{
	if (millis() - previousMillis1 >= (delay2Sec/2))
	{
		previousMillis1 = millis();
		dt = clock.getDateTime();
	}
	if (millis() - previousMillis10 >= (delay2Sec*5))
	{
		showTime = false;
	}
	
	String dsTime = "";			// dsTime = Display Show Time
	dsTime += dt.hour;
	dsTime += ":";
	dsTime += dt.minute;
	dsTime += ":";
	dsTime += dt.second;
	
	return dsTime;
}

void buttonSwich()
{
	previousMillis10 = millis();
	showTime = true;
	Serial.println("Button pressed");
}