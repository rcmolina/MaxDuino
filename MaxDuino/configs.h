// includes the corresponding user config.h file based on target platform

#ifndef CONFIGS_H_INCLUDED
#define CONFIGS_H_INCLUDED

#include "defines_config.h"

// IF YOU REALLY WANT TO DEFINE YOUR CONFIG VERSION IN THIS HEADER FILE,
// YOU CAN, BY UNCOMMENTING AND CHANGING THE FOLLOWING LINE
// BUT DOING IT VIA BUILD SETTINGS MIGHT BE EASIER
// #define CONFIG_FILE 7
// ALTERNATIVELY YOU CAN DEFINE A DEFAULT FOR A PLATFORM WITHOUT CHANGING
// THE DEFAULTS FOR OTHER PLATFORMS
// #define _CONFIG_FILE_DEFAULT_ATMEGA328P 7


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*   OPTION TO HAVE ALL CONFIG PARAMETERS DEFINED ENTIRELY VIA -D / platformio.ini                                      */
#if defined(CONFIGFILE) && CONFIGFILE == -1

  #include "userconfig_blank.h"

#else
  #if defined(__AVR_ATmega2560__)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_ATMEGA2560
    #endif
    #define CONFIG_PATH userMAXconfig
  #elif defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_EVERY
    #endif
    #define CONFIG_PATH userEVERYconfig
  #elif defined(__arm__) && defined(__STM32F1__)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_STM32
    #endif
    #define CONFIG_PATH userSTM32config
  #elif defined(SEEED_XIAO_M0)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_XIAO_M0
    #endif
    #define CONFIG_PATH userSEEEDUINO_XIAO_M0config
  #elif defined(ARDUINO_XIAO_ESP32C3)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_XIAO_ESP32C3
    #endif
    #define CONFIG_PATH userSEEEDUINO_XIAO_ESP32C3
  #elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_WEMOS_D1MINI_ESP8266
    #endif
    #define CONFIG_PATH userARDUINO_ESP8266_WEMOS_D1MINI
  #else //__AVR_ATmega328P__
    #include "userconfig.h" // legacy
    #ifndef CONFIGFILE
    #define CONFIGFILE _CONFIG_FILE_DEFAULT_ATMEGA328P
    #endif
    #define CONFIG_PATH userconfig
  #endif

  #include CONFIG_header(CONFIG_PATH, CONFIGFILE)

#endif

#endif // CONFIGS_H_INCLUDED
