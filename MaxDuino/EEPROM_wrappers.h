// platform-independent wrapper for common eeprom interface

#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

#include "Arduino.h"

#if defined(__AVR__)
  #include <EEPROM.h>
  #define EEPROM_put EEPROM.put
  #define EEPROM_get EEPROM.get

#elif defined(__arm__) && defined(__STM32F1__)
  #include <EEPROM.h>
  uint8_t EEPROM_get(uint16_t address, byte &data) {
    if (EEPROM.init()==EEPROM_OK) {
      data = (byte)(EEPROM.read(address) & 0xff);  
      return true;  
    } else 
      return false;
  } 
  uint8_t EEPROM_put(uint16_t address, byte data) {
    if (EEPROM.init()==EEPROM_OK) {
      EEPROM.write(address, (uint16_t) data); 
      return true;    
    } else
      return false;
  }
  
#elif defined(ESP8266)
  #include <ESP_EEPROM.h>
  uint8_t EEPROM_get(uint16_t address, byte &data) {
      EEPROM.begin(512);
      EEPROM.get(address, data) ;  
      return true;     
  }
  uint8_t EEPROM_put(uint16_t address, byte data) {
      EEPROM.begin(512);
      EEPROM.put(address, data); 
      EEPROM.commit();
      return true;      
  }
#endif
  
#endif // EEPROM_H_INCLUDED
