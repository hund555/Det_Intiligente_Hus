# Det_Intiligente_Hus
### Project beskrivelse
Meaga2560 har en DHT11, Display og en clock.
Mega2560 sættes op som master og bruger spi til at sende humidity og temperaturen til MKRWiFi1010 som getmmer temperatur og humidity i thingspeak.

Mega2560 printer så temperaturen ud til displayed så man kan se temperaturen.

Du kan trykke på en knap så du kan se hvad klokken er istedet for temperaturen som skrifter tilbage efter 10 sekunder.

hvis der er tid.
Der kan også læses et kort som hvis matcher åbner en dør.
ekstra mkrwifi1010 med DHT11 simulering af indørs og udendørs.
Test af retain.
Test af will.

## Flowchart
### Mega2560
![image](Billeder/Mega2560.PNG)
### MRKWiFi1010
![image](Billeder/MRKWiFi1010.PNG)

## Liste af librarys brugt
### Librarys på mega2560
* Arduino
* SPI
* Wire
* Adefruit_BusIO
* Adafruit_GFX_Library
* Adafruit_SSD1306
* Adafruit Unified Sensor
* DHT sensor library

### Librarys på MRKWiFi1010
* Arduino
* WiFiNINA
* ThingSpeak
* SercomSPISlave
