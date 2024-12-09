#ifndef FILE_UTILS_H_INCLUDED
#define FILE_UTILS_H_INCLUDED

#include "Arduino.h" // for types
#include "configs.h"
#include "sdfat_config.h"
#include <SdFat.h>

extern SdFat sd; // the SD card 
extern SdBaseFile entry;  // SD card file
extern SdBaseFile *currentDir;  // SD card directory
extern SdBaseFile _tmpdirs[]; // internal to MaxDuino.ino really, but power.cpp needs to ensure they are closed when shutting down
extern unsigned long filesize;
extern unsigned long bytesRead;
extern uint16_t currentFile; //File index (per filesystem) of current file, relative to current directory (pointed to by currentDir)
extern char fileName[];

extern byte filebuffer[]; // used for small reads from files (readfile, ReadByte, etc use this), sized for the largest read of bytes (= TZX or MSX HEADER read)
extern byte lastByte;

byte readfile(byte nbytes, unsigned long p);
byte ReadByte();
byte ReadWord();
byte ReadLong();
byte ReadDword();

typedef union {
  byte outbyte;
  word outword;
  unsigned long outlong=0;
} _readout_type;

extern _readout_type readout;

#define outByte readout.outbyte
#define outWord readout.outword
#define outLong readout.outlong


#endif // FILE_UTILS_H_INCLUDED
