#ifndef ISR_H_INCLUDED
#define ISR_H_INCLUDED

#include "Arduino.h"

void wave2();

//ISR Variables
extern volatile byte isStopped;
extern volatile byte pinState;
extern volatile bool isPauseBlock;
extern volatile bool wasPauseBlock;

void reset_output_state();

#endif // ISR_H_INCLUDED
