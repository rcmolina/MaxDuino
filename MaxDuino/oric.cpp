#include "configs.h"
#include "compat.h"
#include "oric.h"
#include "processing_state.h"
#include "current_settings.h"
#include "MaxProcessing.h"
#include "file_utils.h"

#ifdef tapORIC
void OricBitWrite() {
  if (currentBit == 11) { //Start Bit
    #ifdef ORICSPEEDUP
    if (BAUDRATE <= 2400){
      if (pass==0) currentPeriod = ORICZEROLOWPULSE; 
      if (pass==1) currentPeriod = ORICZEROHIGHPULSE;
    } else {
      if (pass==0) currentPeriod = ORICTURBOZEROLOWPULSE; 
      if (pass==1) currentPeriod = ORICTURBOZEROHIGHPULSE;            
    }
    #else
      if (pass==0) currentPeriod = ORICZEROLOWPULSE; 
      if (pass==1) currentPeriod = ORICZEROHIGHPULSE;
    #endif         
  } else if (currentBit == 2) { // Paridad inversa i.e. Impar
    #ifdef ORICSPEEDUP
    if (BAUDRATE <= 2400){
      if (pass==0)  currentPeriod = bitChecksum ? ORICZEROLOWPULSE : ORICONEPULSE; 
      if (pass==1)  currentPeriod = bitChecksum ? ORICZEROHIGHPULSE : ORICONEPULSE;
    } else {
      if (pass==0)  currentPeriod = bitChecksum ? ORICTURBOZEROLOWPULSE : ORICTURBOONEPULSE; 
      if (pass==1)  currentPeriod = bitChecksum ? ORICTURBOZEROHIGHPULSE : ORICTURBOONEPULSE;                     
    }
    #else      
      if (pass==0)  currentPeriod = bitChecksum ? ORICZEROLOWPULSE : ORICONEPULSE; 
      if (pass==1)  currentPeriod = bitChecksum ? ORICZEROHIGHPULSE : ORICONEPULSE;
    #endif     
  } else if (currentBit == 1) {
    #ifdef ORICSPEEDUP
    if (BAUDRATE <= 2400){
      currentPeriod = ORICONEPULSE;       
    } else {
      currentPeriod = ORICTURBOONEPULSE;             
    }
    #else      
      currentPeriod = ORICONEPULSE; 
    #endif  
  } else {
    if(currentByte&0x01) {                       //Set next period depending on value of bit 0
      #ifdef ORICSPEEDUP
      if (BAUDRATE <= 2400){
        currentPeriod = ORICONEPULSE;
      } else{
        currentPeriod = ORICTURBOONEPULSE;          
      }
      #else      
      currentPeriod = ORICONEPULSE; 
      #endif        
    } else {
      #ifdef ORICSPEEDUP
      if (BAUDRATE <= 2400){
        if (pass==0)  currentPeriod = ORICZEROLOWPULSE; 
        if (pass==1)  currentPeriod = ORICZEROHIGHPULSE;
      }else{
        if (pass==0)  currentPeriod = ORICTURBOZEROLOWPULSE; 
        if (pass==1)  currentPeriod = ORICTURBOZEROHIGHPULSE;                 
      }
      #else       
      if (pass==0)  currentPeriod = ORICZEROLOWPULSE; 
      if (pass==1)  currentPeriod = ORICZEROHIGHPULSE;
      #endif        
    }
  }   

  pass+=1;      //Data is played as 2 x pulses for a zero, and 2 pulses for a one
  #ifdef ORICSPEEDUP
  if ((currentPeriod == ORICONEPULSE) || (currentPeriod == ORICTURBOONEPULSE)) { 
  #else  
  if (currentPeriod == ORICONEPULSE) {
  #endif

    if ((currentBit>2) && (currentBit<11) && (pass==2)) {
      bitChecksum ^= 1;
      currentByte >>= 1;                        //Shift along to the next bit
      currentBit += -1;
      pass=0;
    }
    if ((currentBit==1) && (pass==6)) {
      currentBit += -1;
      pass=0;
    }
    if (((currentBit==2) || (currentBit==11))  && (pass==2)) {
      currentBit += -1;
      pass=0;
    }
  }
  else {
    // must be a zero pulse
    if(pass==2) {
      if ((currentBit>2) && (currentBit<11)) {
        currentByte >>= 1;                        //Shift along to the next bit
      }
      currentBit += -1;
      pass=0; 
    }
  }

  if ((currentBit==0) && (lastByte)) {
  #ifdef MenuBLK2A 
    count_r = 255; 
    if(ReadByte()) { 
      bytesRead += -1;                      //rewind a byte if we've not reached the end           
      currentBlockTask = BLOCKTASK::PAUSE;
    }else {
      currentTask = TASK::GETID;
      return;
    }
  #else
    count_r = 255;
    currentBlockTask = BLOCKTASK::PAUSE;
  #endif
  }    
}

void OricDataBlock() {
  //Convert byte from file into string of pulses.  One pulse per pass
  if(currentBit==0) {                         //Check for byte end/first byte
    
    if(ReadByte()) {            //Read in a byte
      currentByte = outByte;
      bytesToRead += -1;
      bitChecksum = 0;
      if(bytesToRead == 0) {                  //Check for end of data block
        lastByte = 1;
      }
    } else {                         // If we reached the EOF
      count_r =255;
      currentTask = TASK::GETID;
      return;
    }

    currentBit = 11;
    pass=0;
  }
  OricBitWrite();
}

void FlushBuffer(long newcount) {
  if(count_r>0) {
    currentPeriod = ORICONEPULSE;
    count_r--;
  } else {   
    count_r= newcount;
    currentBlockTask = BLOCKTASK::SYNC1;
    #ifdef MenuBLK2A
      if (!skip2A) ForcePauseAfter0();
    #endif
  }             
  return;
}

void tzx_process_blockid_oric() {
  switch(currentBlockTask) {            
    case BLOCKTASK::READPARAM: // currentBit = 0 y count_r = 255
    case BLOCKTASK::SYNC1:
      if(currentBit >0) {
        OricBitWrite();
      } else {
        ReadByte();
        currentByte=outByte;
        currentBit = 11;
        bitChecksum = 0;
        lastByte=0;
        if (currentByte==0x16) {
            count_r--;
        } else {
            currentBit = 0;
            currentBlockTask=BLOCKTASK::SYNC2;
        } //0x24
      }          
      break;
    case BLOCKTASK::SYNC2:   
      if(currentBit >0) {
        OricBitWrite();
      } else {
        if(count_r >0) {
          currentByte=0x16;
          currentBit = 11;
          bitChecksum = 0;
          lastByte=0;
          count_r--;
        } else {
          count_r=1;
          currentBlockTask=BLOCKTASK::SYNCLAST;
        } //0x24 
      }
      break;
            
    case BLOCKTASK::SYNCLAST:   
      if(currentBit >0) {
        OricBitWrite();
      } else {
        if(count_r >0) {
          currentByte=0x24;
          currentBit = 11;
          bitChecksum = 0;
          lastByte=0;
          count_r--;
        } 
        else {
          count_r=9;
          lastByte=0;
          currentBlockTask=BLOCKTASK::NEWPARAM;
        }
      }
      break;
                    
    case BLOCKTASK::NEWPARAM:            
      if(currentBit >0) {
        OricBitWrite();
      } else {
        if (count_r >0) {
          ReadByte();
          currentByte=outByte;
          currentBit = 11;
          bitChecksum = 0;
          lastByte=0;
          if      (count_r == 5) bytesToRead = (unsigned int)(outByte<<8);
          else if (count_r == 4) bytesToRead = (unsigned int)(bytesToRead + outByte +1) ;
          else if (count_r == 3) bytesToRead = (unsigned int)(bytesToRead -(outByte<<8)) ;
          else if (count_r == 2) bytesToRead = (unsigned int)(bytesToRead - outByte); 
          count_r--;
        }
        else {
          currentBlockTask=BLOCKTASK::NAME;
        }
      }
      break;
            
    case BLOCKTASK::NAME:
      if(currentBit >0) {
        OricBitWrite();
      } else {
        ReadByte();
        currentByte=outByte;
        currentBit = 11;
        bitChecksum = 0;
        lastByte=0;
        if (currentByte==0x00) {
          count_r=1;
          currentBit = 0;
          currentBlockTask=BLOCKTASK::NAME00;
        }
      }               
      break;
            
    case BLOCKTASK::NAME00:
      if(currentBit >0) {
        OricBitWrite();
      } else {
        if (count_r >0) {
          currentByte=0x00;
          currentBit = 11;
          bitChecksum = 0;
          lastByte=0;
          count_r--;
        } else {
          count_r=100;
          lastByte=0;
          currentBlockTask=BLOCKTASK::GAP;
        }
      }
      break;

    case BLOCKTASK::GAP:
      if(count_r>0) {
        currentPeriod = ORICONEPULSE;
        count_r--;
      } else {   
        currentBlockTask=BLOCKTASK::TDATA;
      }             
      break;

      case BLOCKTASK::TDATA:
        OricDataBlock();
        break;
            
      case BLOCKTASK::PAUSE:
        FlushBuffer(100);
        break;                
  }
}

#endif // tapORIC
