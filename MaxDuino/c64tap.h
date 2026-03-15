#ifndef C64TAP_H_INCLUDED
#define C64TAP_H_INCLUDED

#include "Arduino.h"

#ifdef Use_c64
bool c64tap_is_valid();
void c64tap_init();
void tzx_process_blockid_c64tap();
#endif

#endif // C64TAP_H_INCLUDED
