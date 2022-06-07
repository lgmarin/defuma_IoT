#include <SPI.h>

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