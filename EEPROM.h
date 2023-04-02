// platform-independent wrapper for common eeprom interface

#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

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
#endif

#endif // EEPROM_H_INCLUDED
