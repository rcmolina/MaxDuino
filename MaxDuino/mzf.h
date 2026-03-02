#ifndef MZF_H_INCLUDED
#define MZF_H_INCLUDED

#include "Arduino.h"
#include "MaxDuino.h" // for BLOCKID
#include "processing_state.h"

// Initialise internal state for MZF playback (called after file open, before first TZXLoop)
void mzf_init();

// Process one "tick" of MZF playback. Sets global currentPeriod and advances state.
void mzf_process();

#endif // MZF_H_INCLUDED
