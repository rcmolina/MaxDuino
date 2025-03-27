#include "configs.h"
#include "isr.h"
#include "buffer.h"
#include "pinSetup.h"
#include "current_settings.h"
#include "TimerCounter.h"


//ISR Variables accessed/written by main loop
volatile byte isStopped=false;
volatile byte pinState=LOW;
volatile bool isPauseBlock = false;
volatile bool wasPauseBlock = false;

void reset_output_state() {
  // not really part of the ISR, just part of the output
  pinState=LOW;
  WRITE_LOW;
  wasPauseBlock=false;
  isPauseBlock=false;
}

void wave2() {
  //ISR Output routine
//  unsigned long zeroTime = micros();
  word workingPeriod = word(readBuffer[readpos], readBuffer[readpos+1]);
  byte pauseFlipBit = false;
  unsigned long newTime;
  #ifdef DIRECT_RECORDING
  static unsigned long directSampleLength;
  #endif
 
  if(isStopped)
  {
    newTime = 50000;
    goto _set_period;
  }

  if bitRead(workingPeriod, 15)          
  {
    bitClear(workingPeriod,15);         //Clear pause block flag

    //Handle special 'ending' pause
    if (bitRead(workingPeriod, 13)) {
      bitClear(workingPeriod,13);
      newTime = workingPeriod;
      //pinState = LOW;
      //WRITE_LOW;
      pinState = !pinState;
      if (pinState == LOW)
        WRITE_LOW; 
      else   
        WRITE_HIGH;
     
      goto _next;
    }   
 
    //If bit 15 of the current period is set we're about to run a pause
    //Pauses start with a 1.5ms where the output is untouched after which the output is set LOW
    //Pause block periods are stored in milliseconds not microseconds
    isPauseBlock = true;
    if (!wasPauseBlock)
      pauseFlipBit = true;
  }
  #ifdef DIRECT_RECORDING
  else if (bitRead(workingPeriod, 14))
  {
    if bitRead(workingPeriod, 13)
    {
      // this signifies the start of a direct recording block, where we encode the sample period
      directSampleLength = workingPeriod & 0x1fff;
      readpos += 2;
      if(readpos >= buffsize)                  //Swap buffer pages if we've reached the end
      {
        readpos = 0;
        // swap read and write buffers
        volatile byte * tmp = readBuffer;
        readBuffer = writeBuffer;
        writeBuffer = tmp;
        morebuff = true;                  //Request more data to fill inactive page
      } 
      workingPeriod = word(readBuffer[readpos], readBuffer[readpos+1]);
    }
    newTime = directSampleLength;

    // 010xxiiibbbbbbbb
    //         ^
    //         |
    //         +-- this is bit 7
    //
    if bitRead(workingPeriod, 7) {
      pinState = !LOW;
      WRITE_HIGH;
    } else {
      pinState = LOW;
      WRITE_LOW;
    }
    
    if (workingPeriod & 0x0700)
    {
      // more bits remaining
      // do two things at once:  shift the bbbbbbbb left one bit (we drop the lefthand bit
      // so we can actually just mask the lowest 7 bits)
      // and decrement the iii by 1 (and we knowing iii > 0 because we just checked that)
      // Decrementing iii by 1 is the same as decrementing workingPeriod by 0x0100
      // Please note: we write this back into the READ buffer = the buffer that the ISR reads from
      readBuffer[readpos] = (workingPeriod>>8)-1;
      readBuffer[readpos+1] = (workingPeriod&0x7f)<<1;
      goto _set_period;  // skips the part where we move readpos += 2 because we're using the same readpos now
    }
    else
    {
      goto _next;
    }
  }
  #endif
  else if (workingPeriod==0)
  {
    newTime = 1000; // Just in case we have a 0 in the buffer
    goto _next;
  }

  if (pauseFlipBit || !isPauseBlock)
    pinState = !pinState;

  if (pinState == LOW)
    WRITE_LOW;    
  else
    WRITE_HIGH;

  if (isPauseBlock)
  {
    if(pauseFlipBit)
    {
      newTime = 1500;                     //Set 1.5ms initial pause block
      pinState = TSXCONTROLzxpolarityUEFSWITCHPARITY;   
      // reduce pause by 1ms as we've already pause for 1.5ms
      workingPeriod = workingPeriod - 1;
      readBuffer[readpos] = workingPeriod /256;
      readBuffer[readpos+1] = workingPeriod  %256;                
      pauseFlipBit=false;
      goto _set_period;  // skips the part where we move readpos += 2 because we're using the same readpos now
    } else {
      newTime = ((unsigned long)workingPeriod)*1000ul; //Set pause length in microseconds
      isPauseBlock=false;
      wasPauseBlock=true;
    }
  }
  else
  {
    wasPauseBlock=false;
    newTime = workingPeriod;          //After all that, if it's not a pause block set the pulse period 
  }
  
_next:
  readpos += 2;
  if(readpos >= buffsize)                  //Swap buffer pages if we've reached the end
  {
    readpos = 0;
    // swap read and write buffers
    volatile byte * tmp = readBuffer;
    readBuffer = writeBuffer;
    writeBuffer = tmp;
    morebuff = true;                  //Request more data to fill inactive page
  } 

_set_period:
  Timer.setPeriod(newTime);                 //Finally set the next pulse length
}
