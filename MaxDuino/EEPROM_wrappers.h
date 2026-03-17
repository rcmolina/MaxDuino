// platform-independent wrapper for common eeprom interface

#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

#include "Arduino.h"

#if defined(__AVR__)
  #include <EEPROM.h>
  #define EEPROM_put EEPROM.put
  #define EEPROM_get EEPROM.get
#else
  uint8_t EEPROM_get(uint16_t address, byte &data);
  uint8_t EEPROM_put(uint16_t address, byte data);
#endif
void EEPROM_init(void);
  
#endif // EEPROM_H_INCLUDED
