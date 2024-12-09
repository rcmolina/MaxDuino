#include "configs.h"
#include "zx8081.h"
#include "processing_state.h"
#include "file_utils.h"
#include "MaxProcessing.h"
#include "current_settings.h"

//ZX81 Pulse Patterns - Zero Bit  - HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, GAP
//                    - One Bit   - HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, GAP

PROGMEM const byte ZX81Filename[9] = {'T','Z','X','D','U','I','N','O',0x9D};

// currentChar is somehow a unique concept to the zx80/81 handling here
// can maybe find an equivalent existing state variable to reuse instead.
byte currentChar=0;

void ZX80ByteWrite(){
  currentPeriod = ZX80PULSE;
#ifdef ZX81SPEEDUP
  if (BAUDRATE != 1200) currentPeriod = ZX80TURBOPULSE;
#endif
  if(pass==1) {
    currentPeriod=ZX80BITGAP;
  #ifdef ZX81SPEEDUP
    if (BAUDRATE != 1200) currentPeriod = ZX80TURBOBITGAP;
  #endif   
  }
  if(pass==0) {
    if(currentByte&0x80) {                       //Set next period depending on value of bit 0
      pass=19;
    } else {
      pass=9;
    }
    currentByte <<= 1;                        //Shift along to the next bit
    currentBit += -1;
    currentPeriod=0;
  }
  pass+=-1;    
}

void ZX81FilenameBlock() {
  //output ZX81 filename data
  if(currentBit==0) {                         //Check for byte end/first byte
    currentByte = pgm_read_byte(ZX81Filename+currentChar);
    currentChar+=1;
    if(currentChar==10) {
      currentBlockTask = BLOCKTASK::TDATA;
      return;
    }
    currentBit=9;
    pass=0;
  }
  ZX80ByteWrite();
}

void ZX8081DataBlock() {
  if(currentBit==0) {                         //Check for byte end/first byte
    if(ReadByte()) {            //Read in a byte
      currentByte = outByte;
    #ifdef BLOCKID19_IN        
      bytesToRead += -1;
      if((bytesToRead == -1) && (currentID == BLOCKID::ID19)) {    
        bytesRead += -1;                      //rewind a byte if we've reached the end
        temppause = PAUSELENGTH;
        currentID = BLOCKID::IDPAUSE;
      }                   
    #endif 
          
    } else {
      EndOfFile=true;
      //temppause = pauseLength;
      temppause = PAUSELENGTH;
      currentID = BLOCKID::IDPAUSE;
      return;
    }
    currentBit=9;
    pass=0;
  }
  
  ZX80ByteWrite();
}

void tzx_process_blockid_zx8081_zxp() {
  switch(currentBlockTask) {
    case BLOCKTASK::READPARAM:
      currentChar=0;
      // fallthrough->
    
    case BLOCKTASK::PAUSE:
      currentBlockTask=BLOCKTASK::PILOT;
    break; 
                
    case BLOCKTASK::PILOT:
      ZX81FilenameBlock();
    break;
    
    case BLOCKTASK::TDATA:
      ZX8081DataBlock();
    break;
  }
}

void tzx_process_blockid_zx8081_zxo() {
  switch(currentBlockTask) {
    case BLOCKTASK::READPARAM:
      currentChar=0;
      // fallthrough ->
                
    case BLOCKTASK::PAUSE:
      currentBlockTask=BLOCKTASK::TDATA;
    break; 
    
    case BLOCKTASK::TDATA:
      ZX8081DataBlock();
    break; 
  }
}
