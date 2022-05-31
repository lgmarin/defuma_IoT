#include <SPI.h>
#include <TM1637Display.h>

// USING GPIO PINS FOR ESP12 Compatibility!
// D# Pins correspond to nodeMCU V1.2 pins
// TM1637 4d Display Pins
// SCLK -> D2 - GPIO4
// DIO  -> D7 - GPIO13
int tm_DIO = 13;
int tm_CLK = 4;

TM1637Display display(tm_CLK, tm_DIO);

// Define main pins for MAX6675
// SCLK -> D5 - GPIO14
// MISO -> D6 - GPIO12
// CS   -> D8 - GPIO15
int max_CS = 15;
float temp = 0;

// Buzzer PIN
// BUZZ -> D1 - GPIO5
int BUZZ = 5;

float temp_low = 24.00;
float temp_high = 26.00;

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

    // Set Buzzer PIN as Output
    pinMode(BUZZ, OUTPUT);

    // Prepare Display and Wait for MAX initialization
    uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
    display.setBrightness(0x0f);
    display.setSegments(data);

    delay(500);

    Serial.begin(9600);
    display.clear();
}

void loop(){
    //Read Temperature Data
    temp = readTemperatureC(max_CS);

    if (temp == NAN)
    {
      Serial.print("Error: No Thermocouple connected!");
    } 
    else
    {
      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.print(" C");
      display.showNumberDec(temp);
    }
    Serial.print("\n");

    if (temp != NAN && temp > temp_high) {
      tone(BUZZ, 523, 10);
    } else if (temp != NAN && temp < temp_low) {
      tone(BUZZ, 1000, 200);
    } else {
      noTone(BUZZ);
    }

    delay(1000);
    //noTone(BUZZ);
}