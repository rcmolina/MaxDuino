#ifndef CAQ_H
#define CAQ_H

#include "Arduino.h"

// Mattel Aquarius CAQ (cassette image) playback
// Streams bytes from the CAQ file and encodes them as Aquarius cassette serial waveform on outputPin.
void caq_init();
void caq_process();

#endif
