#include <Arduino.h>
#include <SPI.h>

void setup (void)
{
	Serial.begin(115200);				//set baud rate to 115200 for usart
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));	// Set configuration for SPI
	SPI.begin();						// Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
}

void loop (void)
{
	digitalWrite(SS, LOW);				// enable Slave Select
	
	// send test string
	for (const char* p = "Hello, world!\r" ; *p != '\0'; p++)
	{
		SPI.transfer (*p);
		Serial.print(*p);
	}
	
	digitalWrite(SS, HIGH);				// disable Slave Select
	delay(2000);
}