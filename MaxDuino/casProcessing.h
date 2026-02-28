#ifndef CAS_PROCESSING_H_INCLUDED
#define CAS_PROCESSING_H_INCLUDED

#ifdef Use_CAS

#include "constants.h"

/* Header Definitions */
extern PROGMEM const byte HEADER[8];
const byte CAS_ASCII = 0xEA;
const byte CAS_BINF = 0xD0;
const byte CAS_BASIC = 0xD3;

// cas processing state:
extern CASDUINO_FILETYPE casduino;
extern CAS_TYPE cas_currentType;
extern byte fileStage;
extern bool invert;

void casduinoLoop();

void setCASBaud();
extern byte cas_scale; // gets set when you call setCAsBaud
extern byte cas_period; // gets set when you call setCASBaud

#endif // Use_CAS

#endif // CAS_PROCESSING_H_INCLUDED
