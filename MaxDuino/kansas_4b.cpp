#include "configs.h"
#include "Arduino.h"
#include "kansas_4b.h"
#include "MaxProcessing.h"
#include "processing_state.h"
#include "file_utils.h"
#include "MaxDuino.h" // for block_mem_oled and such
#include "current_settings.h"

byte oneBitPulses = 4;
byte zeroBitPulses = 2;
byte startBitPulses = 2;
byte startBitValue = 0;
byte stopBitPulses = 8;
byte stopBitValue = 1;
byte endianness = 0;      //0:LSb 1:MSb (default:0)

void writeData4B() {
  //Convert byte (4B Block) from file into string of pulses.  One pulse per pass
  byte dataBit;

  if (currentBit>0) {

    //Start bits
    if (currentBit==10 && startBitPulses) {
      currentPeriod = startBitValue ? onePulse : zeroPulse;
      pass+=1;
      if ( pass==startBitPulses) {
        currentBit += -1;
        pass = 0;
      }
    }
    //Stop bits
    else if (currentBit==1 && stopBitPulses) {
      currentPeriod = stopBitValue ? onePulse : zeroPulse;
      pass+=1;
      if ( pass==stopBitPulses )  {
        currentBit += -1;
        pass = 0;
      }
    }
    //Data bits
    else
    {
      if (currentBit==10 && !startBitPulses) currentBit = 9;
      dataBit = (currentByte >> (endianness ? (currentBit - 2) : (9 - currentBit))) & 1;
      currentPeriod = dataBit ? onePulse : zeroPulse;
      pass+=1;
      if (pass==(dataBit ? oneBitPulses : zeroBitPulses)) {
        currentBit += -1;
        pass = 0;
      }
      if (currentBit==1 && !stopBitPulses) currentBit = 0;
    }
  }
  else if (currentBit==0 && bytesToRead!=0)
  {
    //Read new byte
    if (ReadByte()) {
      bytesToRead += -1;
      currentByte = outByte;
      currentBit = 10;
      pass = 0;
    } else {
      //End of file
      currentID=BLOCKID::IDEOF;
      return;
    }
  }

  //End of block?
  if (bytesToRead==0 && currentBit==0) {
    temppause = pauseLength;
    currentBlockTask = BLOCKTASK::PAUSE;
  }
}

void tzx_process_blockid_kansas_4b() {
  //Process ID4B - Kansas City Block (MSX specific implementation only)
  switch(currentBlockTask) {
    case BLOCKTASK::READPARAM:
      block_mem_oled();

      if(ReadDword()) {  // Data size to read
        bytesToRead = outLong - 12;
      }
      if(ReadWord()) {  // Pause after block in ms
        pauseLength = outWord;
      }
      if (!TSXCONTROLzxpolarityUEFSWITCHPARITY){
        if(ReadWord()) {  // T-states each pilot pulse
          pilotLength = TickToUs(outWord);
        }
        if(ReadWord()) {  // Number of pilot pulses
          pilotPulses = outWord;
        }
        if(ReadWord()) {  // T-states 0 bit pulse
          zeroPulse = TickToUs(outWord);
        }
        if(ReadWord()) {  // T-states 1 bit pulse
          onePulse = TickToUs(outWord);
        }
      } else {
        //Begin of TSX_SPEEDUP: Fixed speedup baudrate, reduced pilot duration
        pilotPulses = BAUDRATE/1200*5000;
        bytesRead += 8;
        switch(BAUDRATE){
          case 1200:
            //pilotLength = onePulse = TickToUs(729);
            //zeroPulse = TickToUs(1458);
            pilotLength = onePulse = 208;
            zeroPulse = 417;                                              
            break;                    
          case 2400:
            //pilotLength = onePulse = TickToUs(365);
            //zeroPulse = TickToUs(729);
            pilotLength = onePulse = 104;
            zeroPulse = 208;                        
            break;
          case 3150:
            pilotLength = onePulse = 81; //3125=1000000/(80*4), one=81 y zero=160
            zeroPulse = 160;
            break;                      
          case 3600:
            //pilotLength = onePulse = TickToUs(243); // onePulse= 69 (68 para 3675 y en CAS lo tengo a 70)
            //zeroPulse = TickToUs(486);              // zeroPulse= 139 
            pilotLength = onePulse = 70; //3571=1000000/4/70
            zeroPulse = 140;                                              
            break;
          case 3850:
            pilotLength = onePulse = 65; //3846=1000000/(65*4), 66 funciona tb con 3787 bauds
            zeroPulse = 130;                  
            break;
        }
      } //End of TSX_SPEEDUP

      if(ReadByte()) {  // BitCfg
        oneBitPulses =  outByte & 0x0f;       //(default:4)
        zeroBitPulses = outByte >> 4;         //(default:2)
        if (!oneBitPulses) oneBitPulses = 16;
        if (!zeroBitPulses) zeroBitPulses = 16;
      }
      if(ReadByte()) {  // ByteCfg
        //Start Bits Cfg
        startBitValue = (outByte >> 5) & 1;   //(default:0)
        /*startBits = */startBitPulses = (outByte >> 6) & 3;  //(default:1)
        startBitPulses *= startBitValue ? oneBitPulses : zeroBitPulses;
        //Stop Bits Cfg
        stopBitValue = (outByte >> 2) & 1;    //(default:1)
        /*stopBits = */stopBitPulses = (outByte >> 3) & 3;   //(default:2)
        stopBitPulses *= stopBitValue ? oneBitPulses : zeroBitPulses;
        //Endianness
        endianness = outByte & 1;             //0:LSb 1:MSb (default:0)
      }
      currentBlockTask = BLOCKTASK::PILOT;
      break;

    case BLOCKTASK::PILOT:
      //Start with Pilot Pulses
      if (!pilotPulses--) {
        currentBlockTask = BLOCKTASK::TDATA;
      } else {
        currentPeriod = pilotLength;
      }
      break;

    case BLOCKTASK::TDATA:
      //Data playback
      writeData4B();
      break;
    
    case BLOCKTASK::PAUSE:
      //Close block with a pause
      temppause = pauseLength;
      currentID = BLOCKID::IDPAUSE;
      break;
  }
}
