#ifndef MAX_PROCESSING_H_INCLUDED
#define MAX_PROCESSING_H_INCLUDED

#include "Arduino.h"

void UniSetup();
void UniLoop();
void UniPlay();
void UniStop();

void HeaderFail();
void ForcePauseAfter0();
word TickToUs(word ticks);

extern word currentPeriod;
extern word pauseLength;
extern unsigned long bytesToRead;
extern bool EndOfFile;

extern word temppause;
extern byte currentByte;
extern byte bitChecksum;

extern word pilotPulses;
extern word pilotLength;
extern word sync1Length;
extern word sync2Length;
extern word zeroPulse;
extern word onePulse;
extern byte uefpassforZero;
//extern byte passforOne;


#ifdef ID11CDTspeedup
extern bool AMScdt;
#endif


#endif // MAX_PROCESSING_H_INCLUDED
