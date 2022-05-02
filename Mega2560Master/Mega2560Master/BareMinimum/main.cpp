#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3231.h>

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
	
	pinMode(3, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(3), buttonSwich, CHANGE);
}

void loop (void)
{
	display.clearDisplay();
	
	display.setTextSize(1);			// Normal 1:1 pixel scale
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
			digitalWrite(SS, LOW);				// enable Slave Select
			
			// send test string
			int* temperature = 0;
			SPI.transfer(*temperature);
			Serial.println(*temperature);
			
			digitalWrite(SS, HIGH);				// disable Slave Select
			
			String dstr = "Temp: ";				//dstr = Display show temperature rounded
			dstr += *temperature;
			dstr += (char)247;
			
			display.println(dstr);
		}
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
	showTime = true;
}