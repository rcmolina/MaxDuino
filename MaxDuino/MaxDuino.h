#ifndef MAXDUINO_H_INCLUDED
#define MAXDUINO_H_INCLUDED

#include "Arduino.h"

#define SHORT_SILENCE       122
#define LONG_SILENCE        SHORT_SILENCE*2

#define SHORT_HEADER        200
#define LONG_HEADER         800

// processing.cpp can call stopFile and seekFile, which are defined in MaxDuino.ino
void stopFile();
void seekFile();

//TZX block list - uncomment as supported
enum BLOCKID
{
  UNKNOWN = 0x00, // not a block ID / not initialized yet or trigger an error
  ID10 = 0x10,    //Standard speed data block
  ID11 = 0x11,    //Turbo speed data block
  ID12 = 0x12,    //Pure tone
  ID13 = 0x13,    //Sequence of pulses of various lengths
  ID14 = 0x14,    //Pure data block
  ID15 = 0x15,    //Direct recording block
//  ID18 = 0x18,    //CSW recording block
  ID19 = 0x19,    //Generalized data block (NB hacked for zx81 only, will not work for anything else)
  ID20 = 0x20,    //Pause (silence) or 'Stop the tape' command
  ID21 = 0x21,    //Group start
  ID22 = 0x22,    //Group end
  ID23 = 0x23,    //Jump to block
  ID24 = 0x24,    //Loop start
  ID25 = 0x25,    //Loop end
  ID26 = 0x26,    //Call sequence
  ID27 = 0x27,    //Return from sequence
  ID28 = 0x28,    //Select block
  ID2A = 0x2A,    //Stop the tape if in 48K mode
  ID2B = 0x2B,    //Set signal level
  ID30 = 0x30,    //Text description
  ID31 = 0x31,    //Message block
  ID32 = 0x32,    //Archive info
  ID33 = 0x33,    //Hardware type
  ID35 = 0x35,    //Custom info block
  ID4B = 0x4B,    //Kansas City block (MSX/BBC/Acorn/...)
  ID5A = 0x5A,    //Glue block (90 dec, ASCII Letter 'Z')
  IDPAUSE = 0x80, //Custom Pause processing
  JTAP = 0xF8,    //JUPITER ACE Tap File  
  UEF = 0xF9,     //UEF file
  ORIC = 0xFA,    //Oric Tap File
  AYO = 0xFB,     //AY file
  ZXO = 0xFC,     //ZX80 O file
  ZXP = 0xFD,     //ZX81 P File
  TAP = 0xFE,     //Tap File Mode
  IDEOF = 0xFF,   //End of file
};

enum class TASK : byte
{
  // basic initialisation when start playing a file
  INIT,

  //TZX File Tasks
  GETFILEHEADER,
  GETID,
  PROCESSID,
  GETAYHEADER,

  //TZX File Tasks for UEF
  GETUEFHEADER,
  GETCHUNKID,
  PROCESSCHUNKID,

  // CAS tasks
  CAS_lookType,       //looking for file type
  CAS_wHeader,        //Write Header
  CAS_wSilence,       //Write Silence
  CAS_wData,          //Write Data
  CAS_wClose,         //Write closing silence
  CAS_wSync,          //
  CAS_wNameFileBlk,   //
  CAS_lookLeader,
  CAS_wNewLeader,
};

enum class BLOCKTASK : byte
{
  //TZX ID Tasks
  READPARAM,
  PILOT,
  SYNC1,
  SYNC2,
  TDATA,
  PAUSE,
  ID15_TDATA, // special for ID15 performance reasons only

  // TZX tasks for ORIC
  NEWPARAM,
  NAME,
  GAP,
  SYNCLAST,
  NAME00,
};

//Spectrum Standards
#define PILOTLENGTH           619
#define SYNCFIRST             191
#define SYNCSECOND            210
#define ZEROPULSE             244
#define ONEPULSE              489
#define PILOTNUMBERL          8063
#define PILOTNUMBERH          3223
#define PAUSELENGTH           1000   

//Main Variables
extern bool pauseOn;                   //Pause state
extern byte start;                     //Currently playing flag

#ifdef BLKBIGSIZE
  extern word block;
#else
  extern byte block;
#endif

extern byte jblks;
extern byte oldMinBlock;
#ifdef BLOCK_EEPROM_PUT
  extern byte oldMaxBlock;
#else
  extern byte oldMaxBlock;
#endif

extern byte jtapflag;

#define DEBUG 0

void block_mem_oled();

#endif // MAXDUINO_H_INCLUDED
