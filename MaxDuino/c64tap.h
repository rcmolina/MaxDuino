#ifndef C64TAP_H_INCLUDED
#define C64TAP_H_INCLUDED

#include "configs.h"

#ifdef Use_c64
#include "Arduino.h"

bool c64tap_is_header(const byte *header, unsigned long fileSize);
void c64tap_init();
void c64tap_process();
#endif

#endif // C64TAP_H_INCLUDED
