#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include "Arduino.h" // for types
#include "configs.h"

/* With latest casprocessing logic, buffsize can be any multiple of 2.
*/
#ifdef LARGEBUFFER
  #define buffsize 254
#else
  #define buffsize 176
#endif

extern volatile bool morebuff;
extern byte readpos;
extern byte writepos;
extern volatile byte wbuffer[2][buffsize];
extern volatile byte * volatile writeBuffer;
extern volatile byte * readBuffer;
void clearBuffer(void);

#endif // BUFFER_H_INCLUDED
