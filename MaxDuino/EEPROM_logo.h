#ifndef EEPROM_logo_h_included
#define EEPROM_logo_h_included

#include "configs.h"
#include "Arduino.h" // for types

#ifdef RECORD_EEPROM_LOGO
void write_logo_to_eeprom(const byte* logoptr);
  #ifdef SDCARD_RECORD_EEPROM_LOGO
  #error We do not support writing start logo to eeprom from both firmware-embedded image and from sd card, in the same firmware type
  #endif
#elif defined(RECORD_EEPROM_LOGO_FROM_SDCARD)
bool handle_load_logo_file();
void read_display_sdcard_logo(byte invert, bool eeprom_write);
#endif

#endif // EEPROM_logo_h_included