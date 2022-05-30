#include <SPI.h>

// Define main pins for MAX6675
// SCLK -> D5 - GPIO14
// MISO -> D6 - GPIO12
// CS   -> D8 - GPIO15

int max_CS = 15;
float temp = 0;

double readTemperatureC(uint8_t CS) {
    //READ MAX6675 Temperature in Celsius using SPI Interface
    uint16_t v;

    digitalWrite(CS, LOW);
    v = SPI.transfer(0x00);
    v <<= 8;
    v |= SPI.transfer(0x00);
    digitalWrite(CS, HIGH);

    if (v & 0x4) {
        // No thermocouple attached, return 0
        return NAN; 
    }

    v >>= 3;

    return v*0.25;
}

void setup(){
    //Intiate SPI transaction
    SPI.begin();

    //Set max_CS Pin as OUTPUT and set to HIGH to Intiate MAX 6675
    pinMode(max_CS, OUTPUT);
    digitalWrite(max_CS, HIGH);

    Serial.begin(9600);
}

void loop(){
    //Read Temperature Data

    if (readTemperatureC(max_CS) == NAN)
    {
      Serial.print("Error: No Thermocouple connected!");
    } 
    else
    {
      temp = readTemperatureC(max_CS);
      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.print(" C");
    }
    Serial.print("\n");
    delay(1000);
}