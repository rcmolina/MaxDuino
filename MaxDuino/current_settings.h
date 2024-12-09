#include "Arduino.h" // for types

extern word BAUDRATE;
extern bool mselectMask;
extern bool TSXCONTROLzxpolarityUEFSWITCHPARITY;
extern bool skip2A;

#ifdef LOAD_EEPROM_SETTINGS
void updateEEPROM();
void loadEEPROM();
#endif
