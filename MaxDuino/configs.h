// includes the corresponding user config.h file based on target platform

#ifndef CONFIGS_H_INCLUDED
#define CONFIGS_H_INCLUDED

#if defined(__AVR_ATmega2560__)
  #include "userMAXconfig.h"
#elif defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__)
  #include "userEVERYconfig.h"
#elif defined(__arm__) && defined(__STM32F1__)
  #include "userSTM32config.h"  
#elif defined(SEEED_XIAO_M0)
  #include "userSEEEDUINO_XIAO_M0config.h"
#elif defined(ARDUINO_XIAO_ESP32C3)
  #include "userSEEEDUINO_XIAO_ESP32C3.h"
#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
  #include "userARDUINO_ESP8266_WEMOS_D1MINI.h"
#else //__AVR_ATmega328P__
  #include "userconfig.h"
#endif

#endif // CONFIGS_H_INCLUDED
