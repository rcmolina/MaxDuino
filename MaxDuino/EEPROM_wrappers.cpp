#include "EEPROM_wrappers.h"

#if defined(__arm__) && defined(__STM32F1__)
  #include <EEPROM.h>
  void EEPROM_init(void)
  {
    EEPROM.init();
  }

  uint8_t EEPROM_get(uint16_t address, byte &data) {
    data = (byte)(EEPROM.read(address) & 0xff);  
    return true;  
  } 
  uint8_t EEPROM_put(uint16_t address, byte data) {
    EEPROM.write(address, (uint16_t) data); 
    return true;    
  }
#elif defined(ESP8266)
  #include <ESP_EEPROM.h>
  void EEPROM_init(void)
  {
    // 1024 = size of flash we want to set aside as EEPROM emulation
    EEPROM.begin(1024);
  }

  uint8_t EEPROM_get(uint16_t address, byte &data) {
      EEPROM.get(address, data) ;  
      return true;
  }
  uint8_t EEPROM_put(uint16_t address, byte data) {
      EEPROM.put(address, data);
      EEPROM.commit();
      return true;  
  }
#else
  void EEPROM_init(void)
  {
    // nothing to do
  }
  #endif

