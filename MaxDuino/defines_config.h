#ifndef DEFINES_CONFIG_H_INCLUDED
#define DEFINES_CONFIG_H_INCLUDED

// Some helpers for pre-processing command-line defines (-DBLAH)
// to simplify changing settings at compile time / build time
// e.g. via platformio.ini

#define __CONFIG_header(x) #x
#define _CONFIG_header(p,x) __CONFIG_header(p ## x.h)
#define CONFIG_header(p,x) _CONFIG_header(p,x)

// names of default config file suffixes  (blank means 'no config file suffix')
// You can just pass -DCONFIG_FILE to override these e.g. -DCONFIG_FILE 7 will load
// config file named userconfig{platform}7.h
// blank just means config file is named userconfig{platform}.h but you can still say
// -DCONFIG_FILE 0 to load userconfig{platform}0.h etc

// just syntactic sugar to make it clear that we're saying 'yes, this is the config file that has no suffix'
#define NO_SUFFIX

#ifndef _CONFIG_FILE_DEFAULT_ATMEGA328P
#define _CONFIG_FILE_DEFAULT_ATMEGA328P 7
#endif

#ifndef _CONFIG_FILE_DEFAULT_ATMEGA2560
#define _CONFIG_FILE_DEFAULT_ATMEGA2560 NO_SUFFIX
#endif

#ifndef _CONFIG_FILE_DEFAULT_EVERY
#define _CONFIG_FILE_DEFAULT_EVERY NO_SUFFIX
#endif

#ifndef _CONFIG_FILE_DEFAULT_STM32
#define _CONFIG_FILE_DEFAULT_STM32 NO_SUFFIX
#endif

#ifndef _CONFIG_FILE_DEFAULT_XIAO_M0
#define _CONFIG_FILE_DEFAULT_XIAO_M0 NO_SUFFIX
#endif

#ifndef _CONFIG_FILE_DEFAULT_XIAO_ESP32C3
#define _CONFIG_FILE_DEFAULT_XIAO_ESP32C3 NO_SUFFIX
#endif

#ifndef _CONFIG_FILE_DEFAULT_WEMOS_D1MINI_ESP8266
#define _CONFIG_FILE_DEFAULT_WEMOS_D1MINI_ESP8266 NO_SUFFIX
#endif

#endif // DEFINES_CONFIG_H_INCLUDED