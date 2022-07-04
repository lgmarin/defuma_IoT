#ifndef readTemp_h
#define readTemp_h

/**
 * Read MAX6675 temperature in ºC using SPI interface.
 *
 * @param CS GPIO CS pin used by the MAX6675.
 * @return Temperature value in ºC or NAN if no probe connected.
 */
double readTemperatureC(uint8_t CS);

#endif