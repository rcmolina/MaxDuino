// You can now just set -DCONFIG_FILE 7 (or any other number) in your platformio.ini
// or in your build settings.
// You can also just #define CONFIG_FILE in configs.h
// or #define one of the per-plaform CONFIG_FILE_DEFAULT settings instead (see #defines_config.h)
// or if you want you can still define it here - but this will only be used for ATMEGA328P boards
#ifndef CONFIGFILE
#define CONFIGFILE 7
#endif

