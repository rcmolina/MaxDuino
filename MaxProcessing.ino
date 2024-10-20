word TickToUs(word ticks) {
  // returns (ticks/3.5)+0.5 
  /* Hagen Patzke optimization */
  return (word)((((long(ticks) << 2) + 7) >> 1) / 7);
}

void UniPlay(){
  // initialise scale and period based on current BAUDRATE
  // (although these could be overridden later e.g. during checkForEXT, depending on file type)
  setBaud();

  // on entry, currentFile is already pointing to the file entry you want to play
  // and fileName is already set
  if(!entry.open(currentDir, currentFile, O_RDONLY)) {
  //  printtextF(PSTR("Error Opening File"),0);
  }

#ifdef ID11CDTspeedup
  AMScdt = false;
#endif
  block=0;                                    // Initial block when starting
  currentBit=0;                               // fallo reproducción de .tap tras .tzx
  bytesRead=0;                                //start of file
  currentTask=TASK::GETFILEHEADER;                  //First task: search for header
  const char * filenameExt = strrchr(fileName,'.') + 1;
  checkForEXT(filenameExt);
  isStopped=false;
  
  clearBuffer();

#ifdef Use_CAS 
  if (casduino!=CASDUINO_FILETYPE::NONE) { // CAS or DRAGON
    cas_currentType=CAS_TYPE::Nothing;
    currentTask=TASK::GETFILEHEADER;
    fileStage=0;
    Timer.initialize(period);
    Timer.attachInterrupt(wave);
  }
  else 
#endif
  {
    currentBlockTask = BLOCKTASK::READPARAM;               //First block task is to read in parameters
    count = 255;                                //End of file buffer flush 
    EndOfFile=false;
    passforZero=2;
    passforOne=4;
    reset_output_state();
    Timer.initialize(100000); //100ms pause prevents anything bad happening before we're ready
    Timer.attachInterrupt(wave2);
  }
}

void reset_output_state() {
  pinState=LOW;
  WRITE_LOW;
  wasPauseBlock=false;
  isPauseBlock=false;
}

void TZXStop() {
  Timer.stop();
  isStopped=true;
  start=0;
  entry.close();                              //Close file
  seekFile(); 
  bytesRead=0;                                // reset read bytes PlayBytes
#ifdef AYPLAY
  AYPASS_hdrptr = AYPASS_STEP::HDRSTART; // reset AY flag
#endif
#ifdef Use_CAS
  casduino = CASDUINO_FILETYPE::NONE;
#endif
  reset_output_state();
}

void TZXPause() {
  isStopped=pauseOn;
}

void TZXLoop() {   
  noInterrupts();                           //Pause interrupts to prevent var reads and copy values out
  isStopped = pauseOn;
  if(morebuff)
  {
    //Buffer has swapped, start from the beginning of the new page
    writepos=0;
    morebuff=false;    
  }
  interrupts();

  if(currentBlockTask == BLOCKTASK::ID15_TDATA && writepos+16<=buffsize && bytesToRead>=16)
  {
    // shortcut for ID15 handler for performance
    // write 8 input bytes (=16 output bytes to buffer)
    // ALSO: skips the lcd updates (SHOW_CNTR, SHOW_PCT) entirely
    writeDataDirect16();
    return;
  }

  if(writepos<buffsize){                    // Keep filling until full
    TZXProcess();                           //generate the next period to add to the buffer
    if(currentPeriod>0) {
      //add period to the buffer
      const byte _b1 = currentPeriod /256;
      const byte _b2 = currentPeriod %256;
      volatile byte * _wb = writeBuffer+writepos;
      noInterrupts();                       //Pause interrupts while we add a period to the buffer
      *_wb = _b1;
      *(_wb+1) = _b2;
      interrupts();
      writepos+=2;
    }
  } else {
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

void TZXProcess() {
  if(currentBlockTask==BLOCKTASK::ID15_TDATA)
  {
    // shortcut for ID15 handler for performance
    writeDataDirect();
    return;
  }

  currentPeriod = 0;
  switch (currentTask) {
  
    case TASK::GETFILEHEADER:
      //grab 7 byte string
      ReadTZXHeader();
      //set current task to GETID
      currentTask = TASK::GETID;
      break;

  #ifdef AYPLAY
    case TASK::GETAYHEADER:
      //grab 8 byte string
      ReadAYHeader();
      //set current task to PROCESSID
      currentTask = TASK::PROCESSID;
      break;
  #endif

#ifdef Use_UEF
    case TASK::GETUEFHEADER:
      //grab 12 byte string
      ReadUEFHeader();
      //set current task to GETCHUNKID
      currentTask = TASK::GETCHUNKID;
      break;

    case TASK::GETCHUNKID:
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
      break;

    case TASK::PROCESSCHUNKID:
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
              /* stopBits = */ //stopBitPulses = 1;
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
                passforZero = 8;
                passforOne = 16;
                currentTask = TASK::GETCHUNKID;
              } else {
                passforZero = 2;
                passforOne =  4;              
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
      break;
#endif // Use_UEF
    
    case TASK::GETID:
      //grab 1 byte ID
      if(ReadByte()) {
        currentID = outByte;
      } else {
        currentID = BLOCKID::IDEOF;
      }
      //reset data block values
      currentBit=0;
      pass=0;
      //set current task to PROCESSID
      currentTask = TASK::PROCESSID;
      currentBlockTask = BLOCKTASK::READPARAM;
      break;

    case TASK::PROCESSID:
      //ID Processing
      switch(currentID) {
        case BLOCKID::ID10:
          //Process ID10 - Standard Block
          switch (currentBlockTask) {
            case BLOCKTASK::READPARAM:
              block_mem_oled();
              if(ReadWord()) {
                pauseLength = outWord;
              }
              if(ReadWord()) {
                bytesToRead = outWord +1;
              }
              if(ReadByte()) {
                if(outByte == 0) {
                  pilotPulses = PILOTNUMBERL;
                } else {
                  pilotPulses = PILOTNUMBERH;
                }
                bytesRead += -1;
              }
              pilotLength = PILOTLENGTH;
              sync1Length = SYNCFIRST;
              sync2Length = SYNCSECOND;
              zeroPulse = ZEROPULSE;
              onePulse = ONEPULSE;
              currentBlockTask = BLOCKTASK::PILOT;
              usedBitsInLastByte=8;
              break;
            
            default:
              StandardBlock();
              break;
          }
          break;
      
        case BLOCKID::ID11:
          //Process ID11 - Turbo Tape Block
          switch (currentBlockTask) {
            case BLOCKTASK::READPARAM:
              block_mem_oled();
                          
            #if not defined(ID11CDTspeedup)
              if(ReadWord()) {
                pilotLength = TickToUs(outWord);
              }
              if(ReadWord()) {
                sync1Length = TickToUs(outWord);
              }
              if(ReadWord()) {
                sync2Length = TickToUs(outWord);
              }          
              if(ReadWord()) {
                zeroPulse = TickToUs(outWord);
              }
              if(ReadWord()) {
                onePulse = TickToUs(outWord);
              }          
            #else    
              if (TSXCONTROLzxpolarityUEFSWITCHPARITY && AMScdt){ 
                bytesRead += 10;
                switch(BAUDRATE){
                  case 1200: // 1000 Normal baudrate
                    //zeroPulse = TickToUs(1167);
                    pilotLength = onePulse = 666;
                    sync1Length = sync2Length = zeroPulse = 333;                                           
                    break;
                  case 2400: // 2000 High baudrate
                    //zeroPulse = TickToUs(583);
                    pilotLength = onePulse = 334;
                    sync1Length = sync2Length = zeroPulse = 167; 
                    break;
                  case 3150:
                  case 3600: // 3500 Max ROM baudrate
                    //zeroPulse = TickToUs(333);
                    pilotLength = onePulse = 190;
                    sync1Length = sync2Length = zeroPulse = 95;                    
                    break;
                  case 3850: // 4000 Specvar loader baudrate
                    //zeroPulse = TickToUs(292);
                    pilotLength = onePulse = 166;
                    sync1Length = sync2Length = zeroPulse = 83;                      
                    break;
                }                                                                                                   
              } else {
                if(ReadWord()) {
                  pilotLength = TickToUs(outWord);
                }
                if(ReadWord()) {
                  sync1Length = TickToUs(outWord);
                }
                if(ReadWord()) {
                  sync2Length = TickToUs(outWord);
                }                             
                if(ReadWord()) {
                  zeroPulse = TickToUs(outWord);
                }
                if(ReadWord()) {
                  onePulse = TickToUs(outWord);
                }
              }    
            #endif
                            
              if(ReadWord()) {
                pilotPulses = outWord;
              }
              if(ReadByte()) {
                usedBitsInLastByte = outByte;
              }
              if(ReadWord()) {
                pauseLength = outWord;
              }
              if(ReadLong()) {
                bytesToRead = outLong +1;
              }
              currentBlockTask = BLOCKTASK::PILOT;
            break;
          
          default:
            StandardBlock();
            break;
          }
          break; // case ID11

        case BLOCKID::ID12:
          //Process ID12 - Pure Tone Block      
          if(currentBlockTask==BLOCKTASK::READPARAM){
            if(ReadWord()) {
                pilotLength = TickToUs(outWord);
            }
            if(ReadWord()) {
              pilotPulses = outWord;
              //DebugBlock("Pilot Pulses", pilotPulses);
            }
            currentBlockTask = BLOCKTASK::PILOT;
          } else {
            PureToneBlock();
          }
          break;

        case BLOCKID::ID13:
          //Process ID13 - Sequence of Pulses                 
          if(currentBlockTask==BLOCKTASK::READPARAM) {  
            if(ReadByte()) {
              seqPulses = outByte;
            }
            currentBlockTask = BLOCKTASK::TDATA;
          } else {
            PulseSequenceBlock();
          }
          break;

        case BLOCKID::ID14:
          //process ID14 - Pure Data Block             
          if(currentBlockTask==BLOCKTASK::READPARAM) {
            if(ReadWord()) {
              zeroPulse = TickToUs(outWord); 
            }
            if(ReadWord()) {
              onePulse = TickToUs(outWord); 
            }
            if(ReadByte()) {
              usedBitsInLastByte = outByte;
            }
            if(ReadWord()) {
              pauseLength = outWord; 
            }
            if(ReadLong()) {
              bytesToRead = outLong+1;
            }
            currentBlockTask=BLOCKTASK::TDATA;
          } else {
            PureDataBlock();
          }
          break;

    #ifdef DIRECT_RECORDING
      case BLOCKID::ID15:
        //process ID15 - Direct Recording          
        if(currentBlockTask==BLOCKTASK::READPARAM) {
          block_mem_oled();
          currentBit = 0;
          if(ReadWord()) {     
            SampleLength = TickToUs(outWord);
          }
          if(ReadWord()) {      
            //Pause after this block in milliseconds
            pauseLength = outWord;  
          }
          if(ReadByte()) {
            //Used bits in last byte (other bits should be 0)
            usedBitsInLastByte = outByte;
          }
          if(ReadLong()) {
            // Length of samples' data
            bytesToRead = outLong+1;
            // Uncomment next line for testing to force id error
            //currentID=BLOCKID::UNKNOWN;
          }            
          currentBlockTask=BLOCKTASK::ID15_TDATA;

            // and write the sample period information to the output using this format:
            // 011sssssssssssss  = 0x6000 + S
            // |||\___________/
            // |||     |
            // |||     +-- S = sampleLength
            // ||+-- always 1
            // |+-- always 1
            // +-- always 0

            currentPeriod = SampleLength | 0x6000;

          } else if(currentBlockTask==BLOCKTASK::PAUSE) {
            temppause = pauseLength;
            currentID = BLOCKID::IDPAUSE;                     
          } else {
            writeDataDirect();
          }
          break;
      #endif

        case BLOCKID::ID19:
          //Process ID19 - Generalized data block
          switch (currentBlockTask) {
            case BLOCKTASK::READPARAM:
              #ifdef BLOCKID19_IN      
                block_mem_oled();
              #endif

              if(ReadDword()) {
                #ifdef BLOCKID19_IN
                  bytesToRead = outLong;
                #endif
              }
              if(ReadWord()) {
                //Pause after this block in milliseconds
                pauseLength = outWord;
              }

              bytesRead += 86 ; // skip until DataStream filename
              #ifdef BLOCKID19_IN
                bytesToRead += -88; // pauseLength + SYMDEFs
              #endif
              //currentBlockTask=PAUSE;
              currentBlockTask=BLOCKTASK::TDATA;
              break;
          /*
            case PAUSE:
              currentPeriod = PAUSELENGTH;
              bitSet(currentPeriod, 15);
              currentBlockTask=BLOCKTASK::TDATA;
              break; 
          */               
            case BLOCKTASK::TDATA:
              ZX8081DataBlock();
              break;
          }  
          break; // case ID19

        case BLOCKID::ID20:
          //process ID20 - Pause Block          
          if(ReadWord()) {
            if(outWord>0) {
              forcePause0=false;          // pause0 FALSE
              temppause = outWord;
            } else {                    // If Pause duration is 0 ms then Stop The Tape
              forcePause0=true;          // pause0 TRUE
            }
            currentID = BLOCKID::IDPAUSE;         
          }
          break;

        case BLOCKID::ID21:
          //Process ID21 - Group Start
          #if defined(BLOCKID21_IN)
            block_mem_oled();
          #endif
                        
          if(ReadByte()) {
            bytesRead += outByte;
          }
          currentTask = TASK::GETID;
          break;

        case BLOCKID::ID22:
          //Process ID22 - Group End          
          currentTask = TASK::GETID;
          break;

        case BLOCKID::ID24:
          //Process ID24 - Loop Start          
          if(ReadWord()) {
            loopCount = outWord;
            loopStart = bytesRead;
          }
          currentTask = TASK::GETID;
          break;

        case BLOCKID::ID25:
          //Process ID25 - Loop End          
          loopCount += -1;
          if(loopCount!=0) {
            bytesRead = loopStart;
          } 
          currentTask = TASK::GETID;
          break;

        case BLOCKID::ID2A:
          //Skip//        
          bytesRead+=4;
          if (skip2A) currentTask = TASK::GETID;
          else {
            forcePause0 = true;
            currentID = BLOCKID::IDPAUSE;
          }        
          break;

        case BLOCKID::ID2B:
          //Skip//           
          bytesRead+=5;
          currentTask = TASK::GETID;
          break;
        
        case BLOCKID::ID30:
          //Process ID30 - Text Description         
          if(ReadByte()) {
            //Show info on screen - removed until bigger screen used
            //byte j = outByte;
            //for(byte i=0; i<j; i++) {
            //  if(ReadByte()) {
            //    lcd.print(char(outByte));
            //  }
            //}
            bytesRead += outByte;
          }
          currentTask = TASK::GETID;
          break;

        case BLOCKID::ID31:
          //Process ID31 - Message block         
          if(ReadByte()) {
            // dispayTime = outByte;
          }         
          if(ReadByte()) {
            bytesRead += outByte;
          }
          currentTask = TASK::GETID;
          break;

        case BLOCKID::ID32:
          //Process ID32 - Archive Info
          //Block Skipped until larger screen used          
          if(ReadWord()) {
            bytesRead += outWord;
          }
          currentTask = TASK::GETID;
          break;

        case BLOCKID::ID33:
          //Process ID33 - Machine Info
          //Block Skipped until larger screen used         
          if(ReadByte()) {
            bytesRead += (long(outByte) * 3);
          }
          currentTask = TASK::GETID;
          break;       

        case BLOCKID::ID35:
          //Process ID35 - Custom Info Block
          //Block Skipped          
          bytesRead += 0x10;
          if(ReadDword()) {
            bytesRead += outLong;
          }
          currentTask = TASK::GETID;
          break;
        
        case BLOCKID::ID4B:
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
          break; // Case_ID4B

        case BLOCKID::ID5A:
          // Glue block; nothing to do (skip it)
          bytesRead += 9;
          currentTask = TASK::GETID;
          break;
        
        case BLOCKID::TAP:
          //Pure Tap file block
          switch(currentBlockTask) {
            case BLOCKTASK::READPARAM:
              #if defined(BLOCKTAP_IN)
                block_mem_oled();
              #endif
                  
              pauseLength = PAUSELENGTH;
              if(ReadWord()) {
                bytesToRead = outWord+1;
              }
              if(ReadByte()) {
                if(outByte == 0) {
                  pilotPulses = PILOTNUMBERL + 1;
                } else {
                  pilotPulses = PILOTNUMBERH + 1;
                }
                bytesRead += -1;
              }
              pilotLength = PILOTLENGTH;
              sync1Length = SYNCFIRST;
              sync2Length = SYNCSECOND;
              zeroPulse = ZEROPULSE;
              onePulse = ONEPULSE;
              currentBlockTask = BLOCKTASK::PILOT;
              usedBitsInLastByte=8;
              break;

            default:
              StandardBlock();
              break;
          }
          break; // Case TAP

        case BLOCKID::ZXP:
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
          break; // Case ZXP
        
        case BLOCKID::ZXO:
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
          break; // Case ZXO
        
      #ifdef AYPLAY
        case BLOCKID::AYO:                           //AY File - Pure AY file block - no header, must emulate it
          switch(currentBlockTask) {
            case BLOCKTASK::READPARAM:
              pauseLength = PAUSELENGTH;  // Standard 1 sec pause
                                          // here we must generate the TAP header which in pure AY files is missing.
                                          // This was done with a DOS utility called FILE2TAP which does not work under recent 32bit OSs (only using DOSBOX).
                                          // TAPed AY files begin with a standard 0x13 0x00 header (0x13 bytes to follow) and contain the 
                                          // name of the AY file (max 10 bytes) which we will display as "ZXAYFile " followed by the 
                                          // length of the block (word), checksum plus 0xFF to indicate next block is DATA.
                                          // 13 00[00 03(5A 58 41 59 46 49 4C 45 2E 49)1A 0B 00 C0 00 80]21<->[1C 0B FF<AYFILE>CHK]
              pilotLength = PILOTLENGTH;
              sync1Length = SYNCFIRST;
              sync2Length = SYNCSECOND;
              zeroPulse = ZEROPULSE;
              onePulse = ONEPULSE;
              currentBlockTask = BLOCKTASK::PILOT;    // now send pilot, SYNC1, SYNC2 and TDATA (writeheader() from String Vector on 1st pass then writeData() on second)
              bitChecksum = 0;
              bytesRead = 0;
              if (AYPASS_hdrptr == AYPASS_STEP::HDRSTART){
                pilotPulses = PILOTNUMBERL + 1;
              } else {
                // already sent header, so now send data block (shorter pilot)
                pilotPulses = PILOTNUMBERH + 1;
                bytesToRead = filesize+5;   // set length of file to be read plus data byte and CHKSUM (and 2 block LEN bytes)
              }
              usedBitsInLastByte=8;
              break;

            default:
              StandardBlock();
              break;
          }  
          break; // Case AYO
      #endif

      #ifdef tapORIC
        case BLOCKID::ORIC:
          switch(currentBlockTask) {            
            case BLOCKTASK::READPARAM: // currentBit = 0 y count = 255
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
                  count--;
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
                if(count >0) {
                  currentByte=0x16;
                  currentBit = 11;
                  bitChecksum = 0;
                  lastByte=0;
                  count--;
                } else {
                  count=1;
                  currentBlockTask=BLOCKTASK::SYNCLAST;
                } //0x24 
              }
              break;
                
            case BLOCKTASK::SYNCLAST:   
              if(currentBit >0) {
                OricBitWrite();
              } else {
                if(count >0) {
                  currentByte=0x24;
                  currentBit = 11;
                  bitChecksum = 0;
                  lastByte=0;
                  count--;
                } 
                else {
                  count=9;
                  lastByte=0;
                  currentBlockTask=BLOCKTASK::NEWPARAM;
                }
              }
              break;
                      
            case BLOCKTASK::NEWPARAM:            
              if(currentBit >0) {
                OricBitWrite();
              } else {
                if (count >0) {
                  ReadByte();
                  currentByte=outByte;
                  currentBit = 11;
                  bitChecksum = 0;
                  lastByte=0;
                  if      (count == 5) bytesToRead = (unsigned int)(outByte<<8);
                  else if (count == 4) bytesToRead = (unsigned int)(bytesToRead + outByte +1) ;
                  else if (count == 3) bytesToRead = (unsigned int)(bytesToRead -(outByte<<8)) ;
                  else if (count == 2) bytesToRead = (unsigned int)(bytesToRead - outByte); 
                  count--;
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
                  count=1;
                  currentBit = 0;
                  currentBlockTask=BLOCKTASK::NAME00;
                }
              }               
              break;
              
            case BLOCKTASK::NAME00:
              if(currentBit >0) {
                OricBitWrite();
              } else {
                if (count >0) {
                  currentByte=0x00;
                  currentBit = 11;
                  bitChecksum = 0;
                  lastByte=0;
                  count--;
                } else {
                  count=100;
                  lastByte=0;
                  currentBlockTask=BLOCKTASK::GAP;
                }
              }
              break;

            case BLOCKTASK::GAP:
              if(count>0) {
                currentPeriod = ORICONEPULSE;
                count--;
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
          break; // Case ORIC

      #endif // tapORIC
                
        case BLOCKID::IDPAUSE:
          if(temppause>0) {
            if(temppause > MAXPAUSE_PERIOD) {
              currentPeriod = MAXPAUSE_PERIOD;
              temppause += -MAXPAUSE_PERIOD;    
            } else {
              currentPeriod = temppause;
              temppause = 0;
            }
            bitSet(currentPeriod, 15);
          } else {
            if (forcePause0) { // Stop the Tape
              if(!count==0) {
                currentPeriod = 32769;
                count += -1;
              } else {
                currentTask = TASK::GETID;
                count = 255;
                ForcePauseAfter0();
              }
            } else { 
              currentTask = TASK::GETID;
              if(EndOfFile) currentID=BLOCKID::IDEOF;
            }
          } 
          break;
    
        case BLOCKID::IDEOF:
          //Handle end of file
          if(!count==0) {
            currentPeriod = 10;
            bitSet(currentPeriod, 15);
            bitSet(currentPeriod, 13);
            count += -1;
          } else {
            stopFile();
            return;
          }       
          break; 
        
        default:
          //ID Not Recognised - Fall back if non TZX file or unrecognised ID occurs
          
          #ifdef LCDSCREEN16x2
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("ID? ");
            lcd.setCursor(4,0);
            //lcd.print(String(currentID, HEX));
            utoa(currentID,PlayBytes,16);
            lcd.print(PlayBytes);
            lcd.setCursor(0,1);
            //lcd.print(String(bytesRead,HEX) + " - L: " + String(loopCount, DEC));
            utoa(bytesRead,PlayBytes,16);
            lcd.print(PlayBytes) ;  lcd.print(" - L: "); lcd.print(loopCount);
          #endif

          #ifdef OLED1306
            utoa(bytesRead,PlayBytes,16);
            printtext(PlayBytes,lineaxy);

          #endif 
          
          #ifdef P8544             
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("ID? ");
            lcd.setCursor(4,0);
            //lcd.print(String(currentID, HEX));
            utoa(currentID,PlayBytes,16);
            lcd.print(PlayBytes);
            lcd.setCursor(0,1);
            //lcd.print(String(bytesRead,HEX) + " - L: " + String(loopCount, DEC));
            utoa(bytesRead,PlayBytes,16);
            lcd.print(PlayBytes) ;  lcd.print(" - L: "); lcd.print(loopCount);
          #endif

          noInterrupts();  
          while(!button_stop()) {
            //waits until the button Stop is pressed.
            //delay(50);
          }
          interrupts();
          stopFile();
          break;
      
      } // end of CurrentID switch statement
      break;
  } // end of CurrentTask switch statement
}

void block_mem_oled()
{
  #ifdef BLOCKID_INTO_MEM
    blockOffset[block%maxblock] = bytesRead;
    blockID[block%maxblock] = currentID;
  #endif
  #ifdef BLOCK_EEPROM_PUT
    EEPROM_put(BLOCK_EEPROM_START+5*block, bytesRead);
    EEPROM_put(BLOCK_EEPROM_START+4+5*block, currentID);
  #endif

  #if defined(OLED1306) && defined(OLEDPRINTBLOCK) 
    #ifdef XY
      setXY(7,2);
      sendChar(pgm_read_byte(HEX_CHAR+(currentID>>4)));sendChar(pgm_read_byte(HEX_CHAR+(currentID&0x0f)));
      setXY(14,2);
      if ((block%10) == 0) sendChar('0'+(block/10)%10);  
      setXY(15,2);
      sendChar('0'+block%10);
    #endif
    #if defined(XY2) && not defined(OLED1306_128_64)
      setXY(9,1);
      sendChar(pgm_read_byte(HEX_CHAR+(currentID>>4)));sendChar(pgm_read_byte(HEX_CHAR+(currentID&0x0f)));
      setXY(12,1);
      if ((block%10) == 0) sendChar('0'+(block/10)%10);
      setXY(13,1);sendChar('0'+block%10);
    #endif
    #if defined(XY2) && defined(OLED1306_128_64)
      #ifdef XY2force
        input[0]=pgm_read_byte(HEX_CHAR+(currentID>>4));
        input[1]=pgm_read_byte(HEX_CHAR+(currentID&0x0f));
        input[2]=0;
        sendStrXY((char *)input,7,4);
        if ((block%10) == 0) {
          utoa((block/10)%10,(char *)input,10);
          sendStrXY((char *)input,14,4);
        }
        input[0]='0'+block%10;
        input[1]=0;
        sendStrXY((char *)input,15,4);
      #else                      
        setXY(7,4);
        sendChar(pgm_read_byte(HEX_CHAR+(currentID>>4)));sendChar(pgm_read_byte(HEX_CHAR+(currentID&0x0f)));
        setXY(14,4);
        if ((block%10) == 0) sendChar('0'+(block/10)%10);
        setXY(15,4);
        sendChar('0'+block%10);
      #endif
    #endif                    
  #endif

  #if defined(BLOCKID_INTO_MEM)
    if (block < maxblock) block++;
    else block = 0;
  #endif
  #if defined(BLOCK_EEPROM_PUT) 
    if (block < 99) block++;
    else block = 0; 
  #endif
  #if defined(BLOCKID_NOMEM_SEARCH) 
    block++;
  #endif             
}

void StandardBlock() {
  //Standard Block Playback
  switch (currentBlockTask) {
    case BLOCKTASK::PILOT:
      //Start with Pilot Pulses
      if(!pilotPulses--) {
        currentBlockTask = BLOCKTASK::SYNC1;
      } else {
        currentPeriod = pilotLength;
      }
      break;
    
    case BLOCKTASK::SYNC1:
      //First Sync Pulse
      currentPeriod = sync1Length;
      currentBlockTask = BLOCKTASK::SYNC2;
      break;
    
    case BLOCKTASK::SYNC2:
      //Second Sync Pulse
      currentPeriod = sync2Length;
      currentBlockTask = BLOCKTASK::TDATA;
      break;
    
    case BLOCKTASK::TDATA:
      //Data Playback
#ifdef AYPLAY
      if (currentID==BLOCKID::AYO && AYPASS_hdrptr <= AYPASS_STEP::HDREND)
      {
        writeHeader2(); // write TAP Header data from String Vector
      }
      else
      {
        writeData();
      }
#else
      writeData();
#endif
      break;
    
    case BLOCKTASK::PAUSE:
      //Close block with a pause
      if((currentID!=BLOCKID::TAP)&&(currentID!=BLOCKID::AYO)) {                  // Check if we have !=AYO too
        temppause = pauseLength;
        currentID = BLOCKID::IDPAUSE;
      } else {
        currentPeriod = pauseLength;
        bitSet(currentPeriod, 15);
        currentBlockTask = BLOCKTASK::READPARAM;
      }

      if(EndOfFile) currentID=BLOCKID::IDEOF;
      break;
  }
}

#ifdef Use_UEF
void UEFCarrierToneBlock() {
  //Pure Tone Block - Long string of pulses with the same length
  if(!pilotPulses--) {
    currentTask = TASK::GETCHUNKID;
  } else {
    currentPeriod = pilotLength;
  }
}
#endif

void PureToneBlock() {
  //Pure Tone Block - Long string of pulses with the same length
  if(!pilotPulses--) {
    currentTask = TASK::GETID;
  } else {
    currentPeriod = pilotLength;
  }
}

void PulseSequenceBlock() {
  //Pulse Sequence Block - String of pulses each with a different length
  //Mainly used in speedload blocks
  if(!seqPulses--) {
    currentTask = TASK::GETID;
  } else {
    if(ReadWord()) {
      currentPeriod = TickToUs(outWord);    
    }
  }
}

void PureDataBlock() {
  //Pure Data Block - Data & pause only, no header, sync
  switch(currentBlockTask) {
    case BLOCKTASK::TDATA:
      writeData();          
    break;
    
    case BLOCKTASK::PAUSE:
      temppause = pauseLength;
      currentID = BLOCKID::IDPAUSE;
    break;
  }
}

/*
void KCSBlock() {
  //Kansas City Standard Block Playback (MSX specific)
  switch(currentBlockTask) {

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
*/

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

#ifdef Use_UEF
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
    if(pass==passforZero) {
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
    if(pass==passforOne) {
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
#endif // Use_UEF

bool getNextDataByte() {
  if(ReadByte()) {            //Read in a byte
    currentByte = outByte;
    #ifdef AYPLAY
    if (AYPASS_hdrptr == AYPASS_STEP::WRITE_FLAG_BYTE) {
      currentByte = 0xFF;                 // Only insert first DATA byte if sending AY TAP DATA Block and don't decrement counter
      AYPASS_hdrptr = AYPASS_STEP::WRITE_CHECKSUM; // set Checksum flag to be sent when EOF reached
      bytesRead += -1;                    // rollback ptr and compensate for dummy read byte
      bytesToRead += 2;                   // add 2 bytes to read as we send 0xFF (data flag header byte) and chksum at the end of the block
    } else {
    #endif
      bytesToRead += -1;
    #ifdef AYPLAY
    }
    bitChecksum ^= currentByte;    // keep calculating checksum
    #endif
    
    if(bytesToRead == 0) {                  //Check for end of data block
      pass = 0;
      bytesRead -= 1;                      //rewind a byte if we've reached the end
      
      if(pauseLength==0) {                  //Search for next ID if there is no pause
        currentTask = TASK::GETID;
      } else {
        currentBlockTask = BLOCKTASK::PAUSE;           //Otherwise start the pause
      }
      
      return false;  // exit
    }
  } else {                         // If we reached the EOF
    
  #ifdef AYPLAY
    // Check if need to send checksum
    if (AYPASS_hdrptr == AYPASS_STEP::WRITE_CHECKSUM)
    {
      currentByte = bitChecksum;          // send calculated chksum
      bytesToRead += 1;                   // add one byte to read
      AYPASS_hdrptr = AYPASS_STEP::FINISHED; // Reset flag to end block
    }
    else
  #endif
    {
      EndOfFile=true;  
  
      if(pauseLength==0) {
        currentTask = TASK::GETID;
      } else {
        currentBlockTask = BLOCKTASK::PAUSE;
      }
      return false;                           // return here if normal TAP or TZX  
    }
  }

  if(bytesToRead!=1) {                      //If we're not reading the last byte play all 8 bits
    currentBit=8;
  } else {
    currentBit=usedBitsInLastByte;          //Otherwise only play back the bits needed
  }

  return true;
}

void writeData() {
  //Convert byte from file into string of pulses.  One pulse per pass
  if(currentBit==0) { //Check for byte end/first byte
    if (!getNextDataByte()) // false == no more bytes
      return;
    pass=0;
  }

  if(currentByte&0x80){                       //Set next period depending on value of bit 0
    currentPeriod = onePulse;
  } else {
    currentPeriod = zeroPulse;
  }
  pass+=1;
  
  if(pass==2) {
    currentByte <<= 1;                        //Shift along to the next bit
    currentBit += -1;
    pass=0;  
  }    
}

#ifdef DIRECT_RECORDING
void writeDataDirect() {
  // Push byte from file into the buffer, with minimal processing.
  // One byte (8 bits) from file turns directly into one entry in the buffer
  // that encapsulates the DirectRecording mode (plus an extra entry at the
  // start to indicate the sample period))
  if (!getNextDataByte()) // false == no more bytes
  {
    currentPeriod = 0;
    return;
  }

  // 010xxiiibbbbbbbb = 0x4000 + (I<<8) + B
  // |||\/\_/\______/
  // ||| | |    |
  // ||| | |    +-- B = 8 bits (leftmost bit next)
  // ||| | +-- I = 0..7 = number of remaining b bits (minus 1). i.e. 7 means 'use all 8 bits', 0 means 'just use one bit'
  // ||| +-- X = unused
  // ||+-- always 0
  // |+-- always 1
  // +-- always 0

  // process 8 bits (or, all currentBit #bits), in one go, to reduce overhead here
  currentPeriod = (1<<14) + ((currentBit-1)<<8) + currentByte;
}

void writeDataDirect16() {
  // Push byte from file into the buffer, with even less processing.
  // This is equivalent to call writeDataDirect 8 times (which generates
  // 8 words i.e. 16 bytes in the wbuffer)
  // This actually bypasses several passes of the TZXLoop / TZXProcess steps
  // This will only be called if there are > 8 more bytes still to read AND
  // space in the output buffer to write 16 bytes without hitting buffsize
  for(byte iter=8; iter>0; --iter)
  {
    if (!getNextDataByte()) // false == no more bytes
    {
      currentPeriod = 0;
      return;
    }

    // process 8 bits in one go
    // 010xxiiibbbbbbbb = 0x4000 + (I<<8) + B
    //      \_/ <-- I = 0..7 = number of remaining b bits (minus 1). i.e. 7 means 'use all 8 bits'
    // (I must be 7 because we already checked we're not on the last byte)
    //
    // so:
    //     currentPeriod = (1<<14) + (7<<8) + currentByte;
    // and then:
    //     writeBuffer[writeppos] = currentPeriod /256;   //add period to the buffer
    //     writeBuffer[writepos+1] = currentPeriod %256;   //add period to the buffer
    //
    // but we don't even need to use currentPeriod variable for this, we can do directly:
    // Also note that the following code takes as much as possible out of the noInterrupts section;
    // on Nano328p there are only 2 assembly instructions between noInterrupts() and interrupts())
    // 5e7a:	f8 94       	cli
    // 5e7c:	20 82       	st	Z, r2
    // 5e7e:	81 83       	std	Z+1, r24	; 0x01
    // 5e80:	78 94       	sei

    const byte _b1 = currentByte;
    volatile byte * _wb = writeBuffer+writepos;
    noInterrupts();                       //Pause interrupts while we add a period to the buffer
    *_wb = 0x47; // = ((1<<14) + (7<<8))>>8
    *(_wb+1) = _b1;
    interrupts();
    writepos+=2;
  }
}
#endif

#ifdef tapORIC
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
      count =255;
      currentTask = TASK::GETID;
      return;
    }

    currentBit = 11;
    pass=0;
  }
  OricBitWrite();
}

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
    count = 255; 
    if(ReadByte()) { 
      bytesRead += -1;                      //rewind a byte if we've not reached the end           
      currentBlockTask = BLOCKTASK::PAUSE;
    }else {
      currentTask = TASK::GETID;
      return;
    }
  #else
    count = 255;
    currentBlockTask = BLOCKTASK::PAUSE;
  #endif
  }    
}

void FlushBuffer(long newcount) {
  if(count>0) {
    currentPeriod = ORICONEPULSE;
    count--;
  } else {   
    count= newcount;
    currentBlockTask = BLOCKTASK::SYNC1;
    #ifdef MenuBLK2A
      if (!skip2A) ForcePauseAfter0();
    #endif
  }             
  return;
}
#endif // tapORIC


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
      pinState = LOW;
      WRITE_LOW;
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

#ifdef AYPLAY
void writeHeader2() {
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
      if (AYPASS_hdrptr-AYPASS_STEP::FILENAME_START < strlen(fileName)) {
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
}  // End writeHeader2()
#endif

void clearBuffer()
{
#ifdef Use_CAS
  const byte fill = (casduino==CASDUINO_FILETYPE::NONE)?0:2;
#else
  const byte fill = 0;
#endif

  for(byte i=0;i<buffsize;i++)
  {
    wbuffer[0][i]=fill;
    wbuffer[1][i]=fill;
  } 
}

void UniSetup() {
  INIT_OUTPORT;
  isStopped=true;
  pinState=LOW;
  WRITE_LOW;
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


void uniLoop() {
 #ifdef Use_CAS
    if (casduino!=CASDUINO_FILETYPE::NONE)
    {
      casduinoLoop();  
    }
    else
 #endif
    {
      TZXLoop();
    }
}

byte ReadByte() {
  //Read a byte from the file, and move file position on one if successful
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(1, bytesRead)==1)
  {
    bytesRead += 1;
    outByte = filebuffer[0];
    return true;
  }
  return false;
}

byte ReadWord() {
  //Read 2 bytes from the file, and move file position on two if successful
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(2, bytesRead)==2)
  {
    bytesRead += 2;
    outWord = word(filebuffer[1], filebuffer[0]);
    return true;
  }
  return false;
}

byte ReadLong() {
  //Read 3 bytes from the file, and move file position on three if successful
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(3, bytesRead)==3)
  {
    bytesRead += 3;
    outLong = ((unsigned long) word(filebuffer[2], filebuffer[1]) << 8) | filebuffer[0];
    return true;
  }
  return false;
}

byte ReadDword() {
  //Read 4 bytes from the file, and move file position on four if successful  
  //Always reads from bytesRead, which is the current position in the file
  if(readfile(4, bytesRead)==4)
  {
    bytesRead += 4;
    outLong = ((unsigned long)word(filebuffer[3], filebuffer[2]) << 16) | word(filebuffer[1], filebuffer[0]);
    return true;
  }
  return false;
}

byte readfile(byte bytes, unsigned long p)
{
  byte i=0;
  if(entry.seekSet(p)) {
    i=entry.read(filebuffer, bytes);
  } 
  return i;
}

void ReadTZXHeader() {
  //Read and check first 10 bytes for a TZX header
  if(readfile(10, 0)==10 && memcmp_P(filebuffer, TZXTape, 7)==0) {
    bytesRead = 10;
    return;
  }

  HeaderFail();
}

void HeaderFail() {
  printtextF(PSTR("Not Valid File"), 0);
  delay(300);     
  TZXStop();
}

#ifdef AYPLAY
void ReadAYHeader() {
  //Read and check first 8 bytes for a TZX header
  if(readfile(8, 0)==8 && memcmp_P(filebuffer, AYFile, 8)==0) {
    bytesRead = 0;
    return;
  }

  HeaderFail();
}
#endif


#ifdef Use_UEF
void ReadUEFHeader() {
  //Read and check first 12 bytes for a UEF header
  if(readfile(9, 0)==9 && memcmp_P(filebuffer, UEFFile, 9)==0) {
    bytesRead = 12;
    return;
  }

  HeaderFail();
}
#endif

void ForcePauseAfter0() {
    pauseOn=true;
    printtext2F(PSTR("PAUSED* "),0);
    forcePause0=false;
    return;  
}
