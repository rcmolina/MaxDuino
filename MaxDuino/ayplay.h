#ifndef AYPLAY_H_INCLUDED
#define AYPLAY_H_INCLUDED

#include "Arduino.h"

#ifdef AYPLAY
void WriteAYHeader();
void ReadAYHeader();
enum AYPASS_STEP : byte {
  HDRSTART = 0,
  FILENAME_START = 3,
  FILENAME_END = 12,
  LEN_LOW_BYTE = 13,
  LEN_HIGH_BYTE = 14,
  HDREND = 19,
  WRITE_FLAG_BYTE = 20,
  WRITE_CHECKSUM = 21,
  FINISHED = 22,
};
extern byte AYPASS_hdrptr;
#endif

#endif // AYPLAY_H_INCLUDED
