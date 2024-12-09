#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "Arduino.h"

#ifndef XXTSR
    #define XXSTR(a) XSTR(a)
    #define XSTR(a) #a
#endif

extern byte input[];
extern byte filebuffer[];
extern char PlayBytes[];

#endif // UTILS_H_INCLUDED
