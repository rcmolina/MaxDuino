#include "configs.h"
#include "compat.h"
#include "uef.h"
#include "MaxDuino.h"
#include "MaxProcessing.h"
#include "processing_state.h"
#include "file_utils.h"


#ifdef Use_UEF
PROGMEM const char UEFFile[9] = {'U','E','F',' ','F','i','l','e','!'};

word chunkID = 0;
byte UEFPASS = 0;
byte parity = 0 ;        //0:NoParity 1:ParityOdd 2:ParityEven (default:0)

#ifdef Use_c116
float outFloat;
#endif

void UEFCarrierToneBlock() {
  //Pure Tone Block - Long string of pulses with the same length
  if(!pilotPulses--) {
    currentTask = TASK::GETCHUNKID;
  } else {
    currentPeriod = pilotLength;
  }
}

void writeUEFData() {
  #ifdef DEBUG
  //Serial.println(F("WriteUEFData"));
  #endif
  //Convert byte from file into string of pulses.  One pulse per pass
  if(currentBit==0) {                         //Check for byte end/first byte
    #ifdef DEBUG
    //Serial.println(F("currentBit==0"));
    #endif
    
    if(ReadByte()) {            //Read in a byte
      currentByte = outByte;
      bytesToRead += -1;
      bitChecksum = 0; 
      #ifdef DEBUG
      //Serial.print(F("  bytesToRead after decrement: "));
      //Serial.println(bytesToRead,DEC);
      #endif
      if(bytesToRead == 0) {                  //Check for end of data block
        lastByte = 1;
        if(pauseLength==0) {                  //Search for next ID if there is no pause
          currentTask = TASK::PROCESSCHUNKID;
        } else {
          currentBlockTask = BLOCKTASK::PAUSE;           //Otherwise start the pause
        }
      }
    } else {                         // If we reached the EOF
      currentTask = TASK::GETCHUNKID;
    }

    currentBit = 11;
    pass=0;
  }

  if ((currentBit == 2) && (parity == 0)) currentBit = 1; // parity N
  if (currentBit == 11) {
    currentPeriod = zeroPulse;
  } else if (currentBit == 2) {
    currentPeriod = (bitChecksum ^ (parity & 0x01)) ? onePulse : zeroPulse; 
  } else if (currentBit == 1) {
    currentPeriod = onePulse;    
  } else {
    if(currentByte&0x01) {                       //Set next period depending on value of bit 0
      currentPeriod = onePulse;
    } else {
      currentPeriod = zeroPulse;
    }
  }

  pass+=1;      //Data is played as 2 x pulses for a zero, and 4 pulses for a one when speed is 1200

  if (currentPeriod == zeroPulse) {
    if(pass==uefpassforZero) {
      if ((currentBit>1) && (currentBit<11)) {
        currentByte >>= 1;                        //Shift along to the next bit
      }
      currentBit += -1;
      pass=0; 
      if ((lastByte) && (currentBit==0)) {
        currentTask = TASK::GETCHUNKID;
      } 
    }
  } else {
    // must be a one pulse
    if(pass==2*uefpassforZero) {
      if ((currentBit>1) && (currentBit<11)) {
        bitChecksum ^= 1;
        currentByte >>= 1;                        //Shift along to the next bit
      }

      currentBit += -1;
      pass=0; 
      if ((lastByte) && (currentBit==0)) {
        currentTask = TASK::GETCHUNKID;
      } 
    }    
  }

  #ifdef DEBUG
  //Serial.print(F("currentBit = "));
  //Serial.println(currentBit,DEC);
  //Serial.print(F("currentPeriod = "));
  //Serial.println(currentPeriod,DEC);
  #endif
}

void ReadUEFHeader() {
  //Read and check first 12 bytes for a UEF header
  if(readfile(9, 0)==9 && memcmp_P(filebuffer, UEFFile, 9)==0) {
    bytesRead = 12;
    return;
  }

  HeaderFail();
}

void tzx_process_taskid_uef_getchunkid() {
  //grab 2 byte ID
  if(ReadWord()) {
    chunkID = outWord;
    if(ReadDword()) {
      bytesToRead = outLong;
      parity = 0;  
          
      #if defined(Use_hqUEF) && defined(Use_c104)          
        if (chunkID == ID0104) {
          bytesToRead+= -3;
          bytesRead+= 1;
          //grab 1 byte Parity
          if(ReadByte()) {
            if (outByte == 'O') parity = TSXCONTROLzxpolarityUEFSWITCHPARITY ? 2 : 1;
            else if (outByte == 'E') parity = TSXCONTROLzxpolarityUEFSWITCHPARITY ? 1 : 2;
            else parity = 0 ;  // 'N'
          }
          bytesRead+= 1;                                         
        }
      #endif
    } else {
      currentTask = TASK::PROCESSID;
      currentID = BLOCKID::IDEOF;
      return;            
    }
  } else {
    currentTask = TASK::PROCESSID;
    currentID = BLOCKID::IDEOF;
    return;      
  }

  if (BAUDRATE == 1200) {
    zeroPulse = UEFZEROPULSE;
    onePulse = UEFONEPULSE;
  } else {
    zeroPulse = UEFTURBOZEROPULSE;
    onePulse = UEFTURBOONEPULSE;
  }  
  lastByte=0;
  
  //reset data block values
  currentBit=0;
  pass=0;
  //set current task to PROCESSCHUNKID
  currentTask = TASK::PROCESSCHUNKID;
  currentBlockTask = BLOCKTASK::READPARAM;
  UEFPASS = 0;
}

void tzx_process_taskid_uef_processchunkid() {
  //CHUNKID Processing
  switch(chunkID) {
    
    case ID0000:
      bytesRead+=bytesToRead;
      currentTask = TASK::GETCHUNKID;
      break;
      
    case ID0100:         
      writeUEFData();
      break;

    case ID0110:
      if(currentBlockTask==BLOCKTASK::READPARAM){
        if(ReadWord()) {

          if (BAUDRATE == 1200) {                     
              pilotPulses = UEFPILOTPULSES;
              pilotLength = UEFPILOTLENGTH;
          } else {
            // turbo mode    
              pilotPulses = UEFTURBOPILOTPULSES;
              pilotLength = UEFTURBOPILOTLENGTH;
          } 
        }
        currentBlockTask = BLOCKTASK::PILOT;
      } 
      else {
        UEFCarrierToneBlock();
      }
      break;

  #if defined(Use_c112)
    case ID0112:
      if(ReadWord()) {
        if (outWord>0) {
          temppause = outWord*2;
          currentPeriod = temppause;
          bitSet(currentPeriod, 15);
        } 
        currentTask = TASK::GETCHUNKID;    
      }
      break;
  #endif

  #if defined(Use_hqUEF)
    // HqUEF-specifc IDs are included here: ID0104, ID0111, ID0114, ID0116, ID0117
    case ID0111:
      if(currentBlockTask==BLOCKTASK::READPARAM){
        if(ReadWord()) {             
            pilotPulses = UEFPILOTPULSES; // for TURBOBAUD1500 is outWord<<2
            pilotLength = UEFPILOTLENGTH;                      
        }
        currentBlockTask = BLOCKTASK::PILOT;
        UEFPASS+=1;  
      } else if (UEFPASS == 1){
          UEFCarrierToneBlock();
          if(pilotPulses==0) {
            currentTask = TASK::PROCESSCHUNKID;
            currentByte = 0xAA;
            lastByte = 1;
            currentBit = 11;
            pass=0;
            UEFPASS = 2;
          }
      } else if (UEFPASS == 2){
          parity = 0; // NoParity
          writeUEFData();
          if (currentBit==0) {
            currentTask = TASK::PROCESSCHUNKID;
            currentBlockTask = BLOCKTASK::READPARAM;
          }          
      } else if (UEFPASS == 3){
        UEFCarrierToneBlock();
      }
      break;

    #if defined(Use_c104) // still inside defined(Use_hqUEF) block 
      case ID0104:          
        writeUEFData();
        break; 
    #endif // Use_c104

    #if defined(Use_c114) // still inside defined(Use_hqUEF) block
      case ID0114: 
        if(ReadWord()) {
          pilotPulses = UEFPILOTPULSES;
          bytesRead-=2; 
        }
        UEFCarrierToneBlock();
        bytesRead+=bytesToRead;
        currentTask = TASK::GETCHUNKID;
        break;          
    #endif // Use_c114

    #if defined(Use_c116) // still inside defined(Use_hqUEF) block
      case ID0116:
        if(ReadDword()) {
          byte * FloatB = (byte *) &outLong;
          outWord = (((*(FloatB+2)&0x80) >> 7) | (*(FloatB+3)&0x7f) << 1) + 10;
          outWord = *FloatB | (*(FloatB+1))<<8  | ((outWord&1)<<7)<<16 | (outWord>>1)<<24  ;
          outFloat = *((float *) &outWord);
          outWord= (int) outFloat;
          
          if (outWord>0) {
            temppause = outWord;
            currentID = BLOCKID::IDPAUSE;
            currentPeriod = temppause;
            bitSet(currentPeriod, 15);
            currentTask = TASK::GETCHUNKID;
          } else {
            currentTask = TASK::GETCHUNKID;
          }     
        }
        break;
    #endif // Use_c116

    #if defined(Use_c117) // still inside defined(Use_hqUEF) block
      case ID0117:
        if(ReadWord()) {
          if (outWord == 300) {
            uefpassforZero = 8;
            //passforOne = 16;
            currentTask = TASK::GETCHUNKID;
          } else {
            uefpassforZero = 2;
            //passforOne =  4;              
            currentTask = TASK::GETCHUNKID;
          }     
        }           
        break;
    #endif // Use_c117

  #endif // Use_hqUEF

    default:
      bytesRead+=bytesToRead;
      currentTask = TASK::GETCHUNKID;
      break;
  }
}

#endif
