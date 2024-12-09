#include "configs.h"
#include "compat.h"
#include "ayplay.h"
#include "processing_state.h"
#include "file_utils.h"
#include "MaxProcessing.h"

#ifdef AYPLAY

PROGMEM const byte TAPHdr[20] = {0x0,0x0,0x3,'Z','X','A','Y','E','M','U','L',' ',' ',0x1A,0xB,0x0,0xC0,0x0,0x80,0x6E}; // 
PROGMEM const byte * const AYFile = TAPHdr+3;  // added additional AY file header check

byte AYPASS_hdrptr = AYPASS_STEP::HDRSTART;

void WriteAYHeader() {
  //Convert byte from HDR Vector String into string of pulses and calculate checksum. One pulse per pass
  if(currentBit==0) {                         //Check for byte end/new byte                         
    if(AYPASS_hdrptr==AYPASS_STEP::HDREND) { // If we've reached end of header block send checksum byte
      currentByte = bitChecksum;
      AYPASS_hdrptr = AYPASS_STEP::WRITE_FLAG_BYTE; // for next TDATA section we'll write the 0xFF flag before the AY data
      currentBlockTask = BLOCKTASK::PAUSE;   // we've finished outputting the TAP header so now PAUSE and send DATA block normally from file
      return;
    }
    ++AYPASS_hdrptr;                 // increase header string vector pointer
    currentByte = pgm_read_byte(TAPHdr+AYPASS_hdrptr);
    if (AYPASS_hdrptr>=AYPASS_STEP::FILENAME_START && AYPASS_hdrptr<=AYPASS_STEP::FILENAME_END) {
      if (size_t(AYPASS_hdrptr-AYPASS_STEP::FILENAME_START) < strlen(fileName)) {
        currentByte = fileName[AYPASS_hdrptr-AYPASS_STEP::FILENAME_START];
        if (currentByte<0x20 || currentByte>0x7f) {
          currentByte = '?';
        }
      } else {
        currentByte = ' ';
      }
    }
    else if(AYPASS_hdrptr==AYPASS_STEP::LEN_LOW_BYTE){ // insert calculated block length minus LEN bytes
      currentByte = lowByte(filesize);
    } else if(AYPASS_hdrptr==AYPASS_STEP::LEN_HIGH_BYTE){
      currentByte = highByte(filesize);
    }
    bitChecksum ^= currentByte;    // Keep track of Chksum
    currentBit=8;
    pass=0; 
  } //End if currentBit == 0

  if(currentByte&0x80) {                       //Set next period depending on value of bit 0
    currentPeriod = onePulse;
  } else {
    currentPeriod = zeroPulse;
  }
  pass+=1;                                    //Data is played as 2 x pulses
  if(pass==2) {
    currentByte <<= 1;                        //Shift along to the next bit
    currentBit += -1;
    pass=0;  
  }    
}

void ReadAYHeader() {
  //Read and check first 8 bytes for a TZX header
  if(readfile(8, 0)==8 && memcmp_P(filebuffer, AYFile, 8)==0) {
    bytesRead = 0;
    return;
  }
  HeaderFail();
}

#endif
