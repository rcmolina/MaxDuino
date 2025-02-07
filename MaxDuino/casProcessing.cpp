#include "configs.h"
#include "Arduino.h"
#include "compat.h"
#include "CounterPercent.h"
#include "constants.h"
#include "file_utils.h"
#include "isr.h"
#include "casProcessing.h"
#include "buffer.h"
#include "processing_state.h"
#include "current_settings.h"
#include "TimerCounter.h"

#ifdef Use_CAS

word bitword;
byte fileStage=0;
CASDUINO_FILETYPE casduino = CASDUINO_FILETYPE::NONE;
bool invert=false;
CAS_TYPE cas_currentType = CAS_TYPE::Nothing;

byte scale; // gets set when you call setBaud
byte period; // gets set when you call setBaud

PROGMEM const byte HEADER[8] = { 0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74 };

bool cas_file_match(const byte matchval)
{
  // simply return true if all bytes between filebuffer and filebuffer+9
  // match the matchval
  for (byte i=9; i>=0; --i)
    if (filebuffer[i] != matchval) return false;
  return true;
}

void bits_to_pulses()
{
  // converts from a packed representation of bits to output (bitword)
  // into a sequence of high and low output levels to emit
  //
  // needs some explanation:
  // bitword will either be 0x8000 to indicate "output silence"
  // or a set of bits (usually 8 or 11 bits), LSB first,
  // e.g. xxxxx11101010100
  //
  // A bit (0 or 1) will be mapped to a set of pulses:
  // 0: a "0" is output as one long pulse        ___/"""
  // 1: a "1" is output as two short pulses      _/"\_/"
  //    or if file is a DRAGONMODE file
  //    then ONE short pulse                     _/"
  //
  // However, note that all the above are inverted for DRAGNOMODE
  // 0:                                          """\___
  // 1:                                          "\_
  //
  // lastly, silence (instead of a bit) is output as a continuous low value
  // repeated for 4 half-periods:                ________
  //
  // or, for DRAGONMODE, as a high value:        """"""""
  //
  //
  // This function packs the output levels as 1s and 0s (highs and lows respectively)
  // into the same format used by the ID15/Direct mode of wave2:
  //
  // 010xxiiibbbbbbbb = 0x4000 + (I<<8) + B
  // |||\/\_/\______/
  // ||| | |    |
  // ||| | |    +-- B = 8 bits (leftmost bit next)
  // ||| | +-- I = 0..7 = number of remaining b bits (minus 1). i.e. 7 means 'use all 8 bits', 0 means 'just use one bit'
  // ||| +-- X = unused, set to zero
  // ||+-- always 0
  // |+-- always 1
  // +-- always 0

  byte bits = 0;
  byte bitpos = 1<<7;
  byte nbits = 0;
  while(bitpos>0 && currentBit!=0)
  {
    while (pass<4 && bitpos>0)
    {
      if (bitword & 0x8000)
      {
        // signifies 'force low', rather than a bit to be output as a pattern
        if (invert) bits |= bitpos;
      }
      else
      {
        switch (bitword & 0x0001) {
          case 0:
            if(pass==0 || pass==1) {
              if (invert) bits |= bitpos;
            } else {
              if (!invert) bits |= bitpos;
            }
            break;

          case 1:
            if(pass==0 || pass==2) {
              if (invert) bits |= bitpos;
            } else {
              if (!invert) bits |= bitpos;
            }
            #if defined(Use_DRAGON)
            if(casduino == CASDUINO_FILETYPE::DRAGONMODE && pass == 1) {
              pass=3;
            }
            #endif
            break;
        }
      }

      bitpos>>=1;
      nbits++;
      pass++;
    }

    if(pass==4)
    {
      pass=0;
      if (currentBit)
      {
        currentBit--;
        if (!(bitword & 0x8000))
          bitword >>= 1;
      }
    }
  }

  if (nbits)
  {
    // we have >0 (at most 8) pulse levels for some bits(s), so put this in the output buffer
    // and move on to the next bit(s)
    volatile byte * _wb = writeBuffer+writepos;
    noInterrupts();                       //Pause interrupts while we add a period to the buffer
    *_wb = 0x40 + (nbits-1); // = (1<<14)>>8;
    *(_wb+1) = bits;
    interrupts();
    writepos+=2;
  }
}

void writeByte(byte b)
{
  // writeByte, writeSilence and writeHeader all put some packed data into
  // a single word, using the format:
  // bitword = 1000000000000000   // means "output only silence"
  // bitword = 0bbbbbbbbbbbbbbb   // means "output the long/short pulses corresponding to bits bbbb"
  //                          ^
  //                          |
  //                           bits bbbb are little-endian , so this lowest bit will be output next
  //
  // The variable "casduino" indicates how many bits in bitword are remaining to emit
  // bitword then gets shifted right when generating pulses (see bits_to_pulses)

  // 'casduino' holds the number of bits: 8 (DRAGON) or 11 (CAS)
  currentBit = (byte)casduino;
#if defined(Use_DRAGON)
  if(casduino == CASDUINO_FILETYPE::CASDUINO)
#endif
  {
    // bitword = xxxxx11bbbbbbbb0
    //                ^^\______/^
    //                 |  data  |
    //                 |        |
    //                 2 stop   1 start
    //                   bits     bit
    //                   = 11     = 0
    //
    bitword = (((word)b)<<1) + 0x0600;
  }
#if defined(Use_DRAGON)
  else
  {
    // bitword = xxxxxxxxbbbbbbbb
    //                   \______/
    //                     data
    bitword = b;
  }
#endif
}

void writeSilence()
{
  // write a 'low' output
  bitword = 0x8000;
  currentBit = (byte)casduino;
}

void writeHeader()
{
  // write a header of 1 bits
  bitword = 0x07ff;  // 11 1's i.e. 0b0000011111111111
  currentBit = (byte)casduino;
}

void process()
{
  if(cas_currentType==CAS_TYPE::typeEOF)
  {
    if(count_r) {
      writeSilence();
      count_r--;
    } else stopFile();    
    return;
  }
  if(currentTask==TASK::GETFILEHEADER || currentTask==TASK::CAS_wData)
  {
    if(readfile(8,bytesRead))
    {
      if(!memcmp_P(filebuffer, HEADER,8)) {
        if(fileStage==0) 
        {
          currentTask = TASK::CAS_lookType;
        } else
        {
          currentTask = TASK::CAS_wSilence;
          count_r=SHORT_SILENCE*scale;
        }
        if(cas_currentType==CAS_TYPE::Nothing) fileStage=1;
        bytesRead+=8;
      }
    } else
    {
      cas_currentType=CAS_TYPE::typeEOF;
      currentTask=TASK::CAS_wClose;
      count_r=LONG_SILENCE*scale;
    }
     
  }

  if(currentTask==TASK::CAS_lookType)
  {
    currentTask = TASK::CAS_wSilence;
    count_r = LONG_SILENCE*scale;
    fileStage=1;       
    cas_currentType = CAS_TYPE::Unknown;
//    if((r=readfile(10,bytesRead))==10)
    if((readfile(10,bytesRead))==10)
    {
      if(cas_file_match(CAS_ASCII))
      {
        cas_currentType = CAS_TYPE::Ascii;
      }else if(cas_file_match(CAS_BINF))
      {
        cas_currentType = CAS_TYPE::Binf;
      }else if(cas_file_match(CAS_BASIC))
      {
        cas_currentType = CAS_TYPE::Basic;
      }
    }
  }

  if(currentTask==TASK::CAS_wSilence)
  {
    if(count_r)
    {
      writeSilence();
      count_r--;
    } else 
    {
      currentTask=TASK::CAS_wHeader;
      if(fileStage==1) 
      {
        //count_r=LONG_HEADER*scale;
        count_r=LONG_HEADER; 
        fileStage+=1;
      } else 
      {
        count_r=SHORT_HEADER*scale;
        //count_r=SHORT_HEADER;
        if(cas_currentType==CAS_TYPE::Ascii) {
          fileStage+=1;
        } else {
          fileStage=0;
        }
      }
    }
  }
  if(currentTask==TASK::CAS_wHeader)
  {
    if(count_r)
    {
      writeHeader();
      count_r--;
    } else
    {
      currentTask=TASK::CAS_wData;
      return;
    }
  }
  if(currentTask==TASK::CAS_wData)
  {
    writeByte(filebuffer[0]);
    if(filebuffer[0]==0x1a && cas_currentType==CAS_TYPE::Ascii) 
    {
      fileStage=0;
    }
  }
  if(currentTask==TASK::GETFILEHEADER || currentTask==TASK::CAS_lookType || currentTask==TASK::CAS_wData) bytesRead+=1; 
}

#if defined(Use_DRAGON)
void processDragon()
{
  lastByte=filebuffer[0];
  if((readfile(1,bytesRead))==1)
  {
    #if defined(Use_Dragon_sLeader) && not defined(Expand_All)
    if(currentTask==TASK::GETFILEHEADER) {
      if(filebuffer[0] == 0x55) {
       writeByte(0x55); 
       bytesRead+=1;
       count_r--;
      } else {
        //currentTask=TASK::CAS_wHeader;
        if(count_r>=0) {
          writeByte(0x55);
          count_r--;
        } else {    
          if (fileStage > 0) currentTask=TASK::CAS_wData;
          else {
            count_r =19;
            currentTask=TASK::CAS_wNameFileBlk;
          }
        }       
      }
    } else if(currentTask==TASK::CAS_wNameFileBlk) {
      if(count_r) {
        writeByte(filebuffer[0]);
        bytesRead+=1;
        count_r--;            
      } else {            
        fileStage=1;
        currentTask=TASK::GETFILEHEADER;
        count_r=255;
      }
    }
    else {              // data block
      //currentTask=TASK::CAS_wData;
      writeByte(filebuffer[0]);
      bytesRead+=1;             // increase pointer to read next byte (not the same)
    }    
  #endif
    
  #if defined(Use_Dragon_sLeader) && defined(Expand_All)
    if(currentTask==TASK::GETFILEHEADER) {
      if(filebuffer[0] == 0x55) {
       writeByte(0x55); 
       bytesRead+=1;
       count_r--;
      } else {
       //currentTask=TASK::CAS_wHeader;
        if(count_r>=0) {
          writeByte(0x55);
          count_r--;
        } else {
          //count_r= 119;
          count_r = 2;      
          currentTask=TASK::CAS_wSync;
        }       
      }
    } else if(currentTask==TASK::CAS_wSync) { 
      if(count_r) {
        writeByte(filebuffer[0]);
        bytesRead+=1;
        count_r--;
      } else {
        writeByte(filebuffer[0]);            // block length
        bytesRead+=1;
        currentTask=TASK::CAS_wNameFileBlk;
        count_r=filebuffer[0]++;                   
      }
    } else if(currentTask==TASK::CAS_wNameFileBlk) { 
      if(count_r) {
        writeByte(filebuffer[0]);
        bytesRead+=1;
        count_r--;
      } else {
        writeByte(filebuffer[0]);            //Si no cierras el FileNmae block con el primer 0x55 se desincroniza
        bytesRead+=1;            
        currentTask=TASK::CAS_lookLeader;
        count_r=255;                 
      }
    } else if(currentTask==TASK::CAS_lookLeader) { 
      if(filebuffer[0] == 0x55) {
        writeByte(0x55); 
        bytesRead+=1;
        count_r--;
      } else {
        //currentTask=TASK::CAS_wNewLeader;
        if(count_r>=0) {
          writeByte(0x55);
          count_r--;
        } else {   
          currentTask=TASK::CAS_wData;
        }
      }
    }
    else {              // data block
      //currentTask=TASK::CAS_wData;
      writeByte(filebuffer[0]);
      bytesRead+=1;             // increase pointer to read next byte (not the same)
    }    
  #endif
  
  #if not defined(Use_Dragon_sLeader)
      // data block
      //currentTask=TASK::CAS_wData;
      writeByte(filebuffer[0]);
      bytesRead+=1;             // increase pointer to read next byte (not the same)
  #endif  

  } else {          // readfile !=1 , ending
    if(currentTask==TASK::CAS_wData) {
      if(lastByte != 0x55) {
        writeByte(0x55);
      }      
      count_r = 54;
      currentTask=TASK::CAS_wSilence;
    }    
    if(currentTask==TASK::CAS_wSilence) {
      if(count_r) {
        writeSilence();
        count_r--;
      } else {
        stopFile();
      }
    }
  }
}
#endif // defined(Use_DRAGON)

void casduinoLoop()
{
  if (writepos<buffsize)
  {
    // first time, set sample period (like ID15)
    if(currentTask==TASK::INIT)
    {
      currentTask=TASK::GETFILEHEADER;

      const word _currentPeriod = period | 0x6000;
      const byte _b1 = _currentPeriod /256;
      const byte _b2 = _currentPeriod %256;
      volatile byte * _wb = writeBuffer+writepos;
      noInterrupts();                       //Pause interrupts while we add a period to the buffer
      *_wb = _b1;
      *(_wb+1) = _b2;
      interrupts();
      writepos+=2;
    }
    else
    {
      if(currentBit==0)
      { 
    #if defined(Use_DRAGON)
        if(casduino == CASDUINO_FILETYPE::DRAGONMODE) {
          processDragon();
        }
        else
    #endif
        {
          process();      
        }
      }
  
      bits_to_pulses();
    }
  }
  else
  {
    if (!pauseOn) {      
    #if defined(SHOW_CNTR)
      lcdTime();          
    #endif
    #if defined(SHOW_PCT)          
      lcdPercent();
    #endif        
    }
  } 
}

void setBaud()
{
  switch(BAUDRATE) {
    case 1200:
      scale=1;
      period=208;
      break;
    case 2400:
      scale=1;
      period=104;
      break;
    case 3150:
      scale=1;
      period=80;
      break;
    case 3600:
      //scale=2713/1200;
      scale=2;
      //period=93; //2700 baudios
      //period = TickToUs(243);
      period=70; //3571 bauds=1000000/4/70 con period 70us, 3675 baudios con period=68 
      break;      
    case 3850:
      scale=2;
      period = 65; //3850 baudios con period=65
      break;
  }
  Timer.stop();
}

#else

void setBaud()
{
  // a no-op, since only CasProcessing uses this
}

#endif
