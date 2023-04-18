word TickToUs(word ticks) {
  // returns (ticks/3.5)+0.5 
  /* Hagen Patzke optimization */
  return (word)((((long(ticks) << 2) + 7) >> 1) / 7);
}

void UniPlay(){
  setBaud();

  // on entry, currentFile is already pointing to the file entry you want to play
  // and fileName is already set
  if(!entry.open(&currentDir, currentFile, O_RDONLY)) {
  //  printtextF(PSTR("Error Opening File"),0);
  }

#ifdef ID11CDTspeedup
  AMScdt = 0;
#endif
  block=0;                                    // Initial block when starting
  currentBit=0;                               // fallo reproducción de .tap tras .tzx
  bytesRead=0;                                //start of file
  currentTask=GETFILEHEADER;                  //First task: search for header
  const char * filenameExt = strrchr(fileName,'.') + 1;
  checkForEXT(filenameExt);
  isStopped=false;
  
#ifdef Use_CAS 
  if (casduino) { // CAS or DRAGON
    currentType=typeNothing;
    currentTask=lookHeader;
    fileStage=0;
    clearBuffer();
    Timer.initialize(period);
    Timer.attachInterrupt(wave);
  }
  else 
#endif
  {
    currentBlockTask = READPARAM;               //First block task is to read in parameters
    clearBuffer2();                               // chick sound with CASDUINO clearBuffer()
    count = 255;                                //End of file buffer flush 
    EndOfFile=false;
    passforZero=2;
    passforOne=4;
    pinState=LOW;
    WRITE_LOW;
    Timer.initialize(1000); //100ms pause prevents anything bad happening before we're ready
    Timer.attachInterrupt(wave2);
  }
}

void TZXStop() {
  Timer.stop();
  isStopped=true;
  start=0;
  entry.close();                              //Close file
  seekFile(); 
  bytesRead=0;                                // reset read bytes PlayBytes
#ifdef AYPLAY
  blkchksum = 0;                              // reset block chksum byte for AY loading routine
  AYPASS = 0;                                 // reset AY flag
#endif
#ifdef Use_CAS
  casduino = CASDUINO_FILETYPE::NONE;
#endif
  pinState=LOW;
  WRITE_LOW;
}

void TZXPause() {
  isStopped=pauseOn;
}

void TZXLoop() {   
  noInterrupts();                           //Pause interrupts to prevent var reads and copy values out
  copybuff = morebuff;
  morebuff = LOW;
  isStopped = pauseOn;
  interrupts();
  if(copybuff==HIGH) {
    btemppos=0;                             //Buffer has swapped, start from the beginning of the new page
    copybuff=LOW;
  }

  if(btemppos<buffsize){                    // Keep filling until full
    TZXProcess();                           //generate the next period to add to the buffer
    if(currentPeriod>0) {
      noInterrupts();                       //Pause interrupts while we add a period to the buffer
      wbuffer[btemppos][workingBuffer ^ 1] = currentPeriod /256;   //add period to the buffer
      wbuffer[btemppos+1][workingBuffer ^ 1] = currentPeriod %256;   //add period to the buffer
      interrupts();
      btemppos+=2;        
    }
  } else {
    if (pauseOn == 0) {
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
  currentPeriod = 0;
  if(currentTask == GETFILEHEADER) {
    //grab 7 byte string
    ReadTZXHeader();
    //set current task to GETID
    currentTask = GETID;
  }

#ifdef AYPLAY
  if(currentTask == GETAYHEADER) {
    //grab 8 byte string
    ReadAYHeader();
    //set current task to PROCESSID
    currentTask = PROCESSID;
  }
#endif

#ifdef Use_UEF
  if (currentTask == GETUEFHEADER) {
    //grab 12 byte string
    ReadUEFHeader();
    //set current task to GETCHUNKID
    currentTask = GETCHUNKID;
  }
  if(currentTask == GETCHUNKID) {
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
        currentTask = PROCESSID;
        currentID =IDEOF;
        return;            
      }
    } else {
      currentTask = PROCESSID;
      currentID =IDEOF;
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
    currentTask = PROCESSCHUNKID;
    currentBlockTask = READPARAM;
    UEFPASS = 0;
  }

  if(currentTask == PROCESSCHUNKID) {
    //CHUNKID Processing

    switch(chunkID) {
      
      case ID0000:
        bytesRead+=bytesToRead;
        currentTask = GETCHUNKID;
        break;
        
      case ID0100:         
        writeUEFData();
        break;

      case ID0110:
        if(currentBlockTask==READPARAM){
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
          currentBlockTask = PILOT;
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
          currentTask = GETCHUNKID;    
        }
        break;
    #endif

    #if defined(Use_hqUEF)
      // HqUEF-specifc IDs are included here: ID0104, ID0111, ID0114, ID0116, ID0117
      case ID0111:
        if(currentBlockTask==READPARAM){
          if(ReadWord()) {             
              pilotPulses = UEFPILOTPULSES; // for TURBOBAUD1500 is outWord<<2
              pilotLength = UEFPILOTLENGTH;                      
          }
          currentBlockTask = PILOT;
          UEFPASS+=1;  
        } else if (UEFPASS == 1){
            UEFCarrierToneBlock();
            if(pilotPulses==0) {
              currentTask = PROCESSCHUNKID;
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
              currentTask = PROCESSCHUNKID;
              currentBlockTask = READPARAM;
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
          currentTask = GETCHUNKID;
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
              currentID = IDPAUSE;
              currentPeriod = temppause;
              bitSet(currentPeriod, 15);
              currentTask = GETCHUNKID;
            } else {
              currentTask = GETCHUNKID;
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
              currentTask = GETCHUNKID;
            } else {
              passforZero = 2;
              passforOne =  4;              
              currentTask = GETCHUNKID;
            }     
          }           
          break;
      #endif // Use_c117

    #endif // Use_hqUEF

      default:
        bytesRead+=bytesToRead;
        currentTask = GETCHUNKID;
        break;
      }
  }      
#endif // Use_UEF
    
  if(currentTask == GETID) {
    //grab 1 byte ID
    if(ReadByte()) {
      currentID = outByte;
    } else {
      currentID = IDEOF;
    }
    //reset data block values
    currentBit=0;
    pass=0;
    //set current task to PROCESSID
    currentTask = PROCESSID;
    currentBlockTask = READPARAM;  
  }
  if(currentTask == PROCESSID) {
    //ID Processing
    switch(currentID) {
      case ID10:
        //Process ID10 - Standard Block
        switch (currentBlockTask) {
          case READPARAM:
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
                sendChar('1');sendChar('0');
                setXY(14,2);
                if ((block%10) == 0) sendChar(48+(block/10)%10);  
                setXY(15,2);
                sendChar(48+block%10);   
              #endif
              #if defined(XY2) && not defined(OLED1306_128_64)
                setXY(9,1);
                sendChar('1');sendChar('0');                    
                setXY(12,1);
                if ((block%10) == 0) sendChar(48+(block/10)%10);
                setXY(13,1);sendChar(48+block%10);
              #endif
              #if defined(XY2) && defined(OLED1306_128_64)
                #ifdef XY2force
                  sendStrXY("10",7,4);
                  if ((block%10) == 0) {
                    utoa((block/10)%10,(char *)input,10);
                    sendStrXY((char *)input,14,4);
                  }
                  input[0]=48+block%10;
                  input[1]=0;
                  sendStrXY((char *)input,15,4);
                #else                      
                  setXY(7,4);
                  sendChar('1');sendChar('0');
                  setXY(14,4);
                  if ((block%10) == 0) sendChar(48+(block/10)%10);
                  setXY(15,4);
                  sendChar(48+block%10);
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
            currentBlockTask = PILOT;
            usedBitsInLastByte=8;
        break;
        
        default:
          StandardBlock();
        break;
        }

      break;
      
      case ID11:
        //Process ID11 - Turbo Tape Block
        switch (currentBlockTask) {
          case READPARAM:
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
                sendChar('1');sendChar('1');
                setXY(14,2);
                if ((block%10) == 0) sendChar(48+(block/10)%10);  
                setXY(15,2);
                sendChar(48+block%10);   
              #endif
              #if defined(XY2) && not defined(OLED1306_128_64)
                setXY(9,1);sendChar('1');sendChar('1');                      
                setXY(12,1);if ((block%10) == 0) sendChar(48+(block/10)%10);
                setXY(13,1);sendChar(48+block%10);
              #endif
              #if defined(XY2) && defined(OLED1306_128_64)
                #ifdef XY2force
                  sendStrXY("11",7,4);
                  if ((block%10) == 0) {utoa((block/10)%10,(char *)input,10);sendStrXY((char *)input,14,4);}
                  input[0]=48+block%10;input[1]=0;sendStrXY((char *)input,15,4);                
                #else
                  setXY(7,4);sendChar('1');sendChar('1');                    
                  setXY(14,4);if ((block%10) == 0) sendChar(48+(block/10)%10);
                  setXY(15,4);sendChar(48+block%10);
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
            if ((TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) && (AMScdt == 1)){ 
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
            currentBlockTask = PILOT;
          break;
        
        default:
          StandardBlock();
          break;
        }
        break; // case ID11

      case ID12:
        //Process ID12 - Pure Tone Block      
        if(currentBlockTask==READPARAM){
          if(ReadWord()) {
              pilotLength = TickToUs(outWord);
          }
          if(ReadWord()) {
            pilotPulses = outWord;
            //DebugBlock("Pilot Pulses", pilotPulses);
          }
          currentBlockTask = PILOT;
        } else {
          PureToneBlock();
        }
        break;

      case ID13:
        //Process ID13 - Sequence of Pulses                 
        if(currentBlockTask==READPARAM) {  
          if(ReadByte()) {
            seqPulses = outByte;
          }
          currentBlockTask = DATA;
        } else {
          PulseSequenceBlock();
        }
        break;

      case ID14:
        //process ID14 - Pure Data Block             
        if(currentBlockTask==READPARAM) {
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
          currentBlockTask=DATA;
        } else {
          PureDataBlock();
        }
        break;

    #ifdef DIRECT_RECORDING
      case ID15:
        //process ID15 - Direct Recording          
        if(currentBlockTask==READPARAM) {
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
            //currentID=0x9A;
          }            
          currentBlockTask=DATA;
        } else if(currentBlockTask==PAUSE) {
          temppause = pauseLength;
          currentID = IDPAUSE;                     
        } else {
          currentPeriod = SampleLength;
          bitSet(currentPeriod, 14);
          writeData();
        }
        break;
    #endif

      case ID19:
        //Process ID19 - Generalized data block
        switch (currentBlockTask) {
          case READPARAM:
            #ifdef BLOCKID19_IN      
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
                  sendChar('1');sendChar('9');
                  setXY(14,2);
                  if ((block%10) == 0) sendChar(48+(block/10)%10);  
                  setXY(15,2);
                  sendChar(48+block%10);   
                #endif
                #if defined(XY2) && not defined(OLED1306_128_64)
                  setXY(9,1);
                  sendChar('1');sendChar('9');                     
                  setXY(12,1);
                  if ((block%10) == 0) sendChar(48+(block/10)%10);
                  setXY(13,1);
                  sendChar(48+block%10);
                #endif
                #if defined(XY2) && defined(OLED1306_128_64)
                  #ifdef XY2force
                    sendStrXY("19",7,4);
                    if ((block%10) == 0) {
                      utoa((block/10)%10,(char *)input,10);
                      sendStrXY((char *)input,14,4);
                    }
                    input[0]=48+block%10;input[1]=0;sendStrXY((char *)input,15,4);                     
                  #else 
                    setXY(7,4);
                    sendChar('1');sendChar('9');                    
                    setXY(14,4);
                    if ((block%10) == 0) sendChar(48+(block/10)%10);
                    setXY(15,4);
                    sendChar(48+block%10);
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
            currentBlockTask=PAUSE;
            break;

          case PAUSE:
            currentPeriod = PAUSELENGTH;
            bitSet(currentPeriod, 15);
            currentBlockTask=DATA;
            break; 
                        
          case DATA:
            ZX8081DataBlock();
            break;
        }  
        break; // case ID19

      case ID20:
        //process ID20 - Pause Block          
        if(ReadWord()) {
          if(outWord>0) {
            forcePause0=0;          // pause0 FALSE
            temppause = outWord;
          } else {                    // If Pause duration is 0 ms then Stop The Tape
            forcePause0=1;          // pause0 TRUE
          }
          currentID = IDPAUSE;         
        }
        break;

      case ID21:
        //Process ID21 - Group Start
        #if defined(BLOCKID21_IN)
          #if defined(BLOCKID_INTO_MEM)
            blockOffset[block%maxblock] = bytesRead;
            blockID[block%maxblock] = currentID;
          #endif
          #if defined(BLOCK_EEPROM_PUT)
            EEPROM_put(BLOCK_EEPROM_START+5*block, bytesRead);
            EEPROM_put(BLOCK_EEPROM_START+4+5*block, currentID); 
          #endif
          #if defined(OLED1306) && defined(OLEDPRINTBLOCK)
            #ifdef XY
              setXY(7,2);
              sendChar('2');sendChar('1');
              setXY(14,2);
              if ((block%10) == 0) sendChar(48+(block/10)%10);  
              setXY(15,2);
              sendChar(48+block%10);   
            #endif
            #if defined(XY2) && not defined(OLED1306_128_64)
              setXY(9,1);
              sendChar('2');sendChar('1');                      
              setXY(12,1);
              if ((block%10) == 0) sendChar(48+(block/10)%10);
              setXY(13,1);
              sendChar(48+block%10);
            #endif
            #if defined(XY2) && defined(OLED1306_128_64)
              #ifdef XY2force
                sendStrXY("21",7,4);
                if ((block%10) == 0) {
                  utoa((block/10)%10,(char *)input,10);
                  sendStrXY((char *)input,14,4);
                }
                input[0]=48+block%10;
                input[1]=0;
                sendStrXY((char *)input,15,4);                     
              #else
                setXY(7,4);
                sendChar('2');sendChar('1');                    
                setXY(14,4);
                if ((block%10) == 0) sendChar(48+(block/10)%10);
                setXY(15,4);
                sendChar(48+block%10);
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
        #endif
                      
        if(ReadByte()) {
          bytesRead += outByte;
        }
        currentTask = GETID;
        break;

      case ID22:
        //Process ID22 - Group End          
        currentTask = GETID;
        break;

      case ID24:
        //Process ID24 - Loop Start          
        if(ReadWord()) {
          loopCount = outWord;
          loopStart = bytesRead;
        }
        currentTask = GETID;
        break;

      case ID25:
        //Process ID25 - Loop End          
        loopCount += -1;
        if(loopCount!=0) {
          bytesRead = loopStart;
        } 
        currentTask = GETID;
        break;

      case ID2A:
        //Skip//        
        bytesRead+=4;
        if (skip2A) currentTask = GETID;
        else {
          forcePause0 = 1;
          currentID = IDPAUSE;
        }        
        break;

      case ID2B:
        //Skip//           
        bytesRead+=5;
        currentTask = GETID;
        break;
      
      case ID30:
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
        currentTask = GETID;
        break;

      case ID31:
        //Process ID31 - Message block         
        if(ReadByte()) {
          // dispayTime = outByte;
        }         
        if(ReadByte()) {
          bytesRead += outByte;
        }
        currentTask = GETID;
        break;

      case ID32:
        //Process ID32 - Archive Info
        //Block Skipped until larger screen used          
        if(ReadWord()) {
          bytesRead += outWord;
        }
        currentTask = GETID;
        break;

      case ID33:
        //Process ID32 - Archive Info
        //Block Skipped until larger screen used         
        if(ReadByte()) {
          bytesRead += (long(outByte) * 3);
        }
        currentTask = GETID;
        break;       

      case ID35:
        //Process ID35 - Custom Info Block
        //Block Skipped          
        bytesRead += 0x10;
        if(ReadDword()) {
          bytesRead += outLong;
        }
        currentTask = GETID;
        break;
      
      case ID4B:
        //Process ID4B - Kansas City Block (MSX specific implementation only)
        switch(currentBlockTask) {
          case READPARAM:
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
                sendChar('4');sendChar('B');
                setXY(14,2);
                if ((block%10) == 0) sendChar(48+(block/10)%10);  
                setXY(15,2);
                sendChar(48+block%10);   
              #endif
              #if defined(XY2) && not defined(OLED1306_128_64)
                setXY(9,1);
                sendChar('4');sendChar('B');                     
                setXY(12,1);
                if ((block%10) == 0) sendChar(48+(block/10)%10);
                setXY(13,1);
                sendChar(48+block%10);
              #endif
              #if defined(XY2) && defined(OLED1306_128_64)
                #ifdef XY2force
                  sendStrXY("4B",7,4);
                  if ((block%10) == 0) {
                    utoa((block/10)%10,(char *)input,10);
                    sendStrXY((char *)input,14,4);
                  }
                  input[0]=48+block%10;
                  input[1]=0;
                  sendStrXY((char *)input,15,4);                     
                #else
                  setXY(7,4);
                  sendChar('4');sendChar('B');                    
                  setXY(14,4);
                  if ((block%10) == 0) sendChar(48+(block/10)%10);
                  setXY(15,4);
                  sendChar(48+block%10);
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

            if(ReadDword()) {  // Data size to read
              bytesToRead = outLong - 12;
            }
            if(ReadWord()) {  // Pause after block in ms
              pauseLength = outWord;
            }
            if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 0){
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
                  pilotLength = onePulse = TickToUs(729);
                  zeroPulse = TickToUs(1458);                       
                  break;                    
                case 2400:
                  pilotLength = onePulse = TickToUs(365);
                  zeroPulse = TickToUs(729);      
                  break;
                case 3600:
                  pilotLength = onePulse = TickToUs(243); //3600
                  zeroPulse = TickToUs(486);                    
                  break;
                case 3850:
                  pilotLength = onePulse = 65; //3850=1000000/(65*4)
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
            currentBlockTask = PILOT;
            break;

          case PILOT:
            //Start with Pilot Pulses
            if (!pilotPulses--) {
              currentBlockTask = DATA;
            } else {
              currentPeriod = pilotLength;
            }
            break;
      
          case DATA:
            //Data playback
            writeData4B();
            break;
          
          case PAUSE:
            //Close block with a pause
            temppause = pauseLength;
            currentID = IDPAUSE;
            break;
        }
        break; // Case_ID4B

      case TAP:
        //Pure Tap file block
        switch(currentBlockTask) {
          case READPARAM:
            #if defined(BLOCKTAP_IN)
              #if defined(BLOCKID_INTO_MEM)
                blockOffset[block%maxblock] = bytesRead;
                blockID[block%maxblock] = currentID;
              #endif
              #if defined(BLOCK_EEPROM_PUT)
                EEPROM_put(BLOCK_EEPROM_START+5*block, bytesRead);
                EEPROM_put(BLOCK_EEPROM_START+4+5*block, currentID); 
              #endif
              
              #if defined(OLED1306) && defined(OLEDPRINTBLOCK)
                #ifdef XY
                  setXY(7,2);
                  sendChar('F');sendChar('E');
                  setXY(14,2);
                  if ((block%10) == 0) sendChar(48+(block/10)%10);  
                  setXY(15,2);
                  sendChar(48+block%10);   
                #endif
                #if defined(XY2) && not defined(OLED1306_128_64)
                  setXY(9,1);
                  sendChar('F');sendChar('E');                      
                  setXY(12,1);
                  if ((block%10) == 0) sendChar(48+(block/10)%10);
                  setXY(13,1);
                  sendChar(48+block%10);
                #endif
                #if defined(XY2) && defined(OLED1306_128_64)
                  #ifdef XY2force
                    sendStrXY("FE",7,4);
                    if ((block%10) == 0) {
                      utoa((block/10)%10,(char *)input,10);
                      sendStrXY((char *)input,14,4);
                    }
                    input[0]=48+block%10;
                    input[1]=0;
                    sendStrXY((char *)input,15,4);                     
                  #else
                    setXY(7,4);
                    sendChar('F');sendChar('E');                    
                    setXY(14,4);
                    if ((block%10) == 0) sendChar(48+(block/10)%10);
                    setXY(15,4);
                    sendChar(48+block%10);
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
            currentBlockTask = PILOT;
            usedBitsInLastByte=8;
            break;

          default:
            StandardBlock();
            break;
        }
        break; // Case TAP

      case ZXP:
        switch(currentBlockTask) {
          case READPARAM:
            currentChar=0;
            // fallthrough->
          
          case PAUSE:
            currentBlockTask=PILOT;
          break; 
                      
          case PILOT:
            ZX81FilenameBlock();
          break;
          
          case DATA:
            ZX8081DataBlock();
          break;
        }
        break; // Case ZXP
      
      case ZXO:
        switch(currentBlockTask) {
          case READPARAM:
            currentChar=0;
            // fallthrough ->
                      
          case PAUSE:
            currentBlockTask=DATA;
          break; 
          
          case DATA:
            ZX8081DataBlock();
          break; 
        }
        break; // Case ZXO
      
    #ifdef AYPLAY
      case AYO:                           //AY File - Pure AY file block - no header, must emulate it
        switch(currentBlockTask) {
          case READPARAM:
            pauseLength = PAUSELENGTH;  // Standard 1 sec pause
                                        // here we must generate the TAP header which in pure AY files is missing.
                                        // This was done with a DOS utility called FILE2TAP which does not work under recent 32bit OSs (only using DOSBOX).
                                        // TAPed AY files begin with a standard 0x13 0x00 header (0x13 bytes to follow) and contain the 
                                        // name of the AY file (max 10 bytes) which we will display as "ZXAYFile " followed by the 
                                        // length of the block (word), checksum plus 0xFF to indicate next block is DATA.
                                        // 13 00[00 03(5A 58 41 59 46 49 4C 45 2E 49)1A 0B 00 C0 00 80]21<->[1C 0B FF<AYFILE>CHK]
            if(hdrptr==HDRSTART) {
              pilotPulses = PILOTNUMBERL + 1;
            }
            else {
              pilotPulses = PILOTNUMBERH + 1;
            }

            pilotLength = PILOTLENGTH;
            sync1Length = SYNCFIRST;
            sync2Length = SYNCSECOND;
            zeroPulse = ZEROPULSE;
            onePulse = ONEPULSE;
            currentBlockTask = PILOT;    // now send pilot, SYNC1, SYNC2 and DATA (writeheader() from String Vector on 1st pass then writeData() on second)
            if (hdrptr==HDRSTART) AYPASS = 1;     // Set AY TAP data read flag only if first run
            if (AYPASS == 2) {           // If we have already sent TAP header
              blkchksum = 0;  
              bytesRead = 0;
              bytesToRead = ayblklen+2;   // set length of file to be read plus data byte and CHKSUM (and 2 block LEN bytes)
              AYPASS = 5;                 // reset flag to read from file and output header 0xFF byte and end chksum
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
      case ORIC:
        switch(currentBlockTask) {            
          case READPARAM: // currentBit = 0 y count = 255
          case SYNC1:
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
                currentBlockTask=SYNC2;
              } //0x24
            }          
            break;
          case SYNC2:   
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
                currentBlockTask=SYNCLAST;
              } //0x24 
            }
            break;
              
          case SYNCLAST:   
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
                currentBlockTask=NEWPARAM;
              }
            }
            break;
                    
          case NEWPARAM:            
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
                currentBlockTask=NAME;
              }
            }
            break;
              
          case NAME:
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
                currentBlockTask=NAME00;
              }
            }               
            break;
            
            case NAME00:
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
                currentBlockTask=GAP;
              }
            }
            break;

          case GAP:
            if(count>0) {
              currentPeriod = ORICONEPULSE;
              count--;
            } else {   
              currentBlockTask=DATA;
            }             
            break;

          case DATA:
            OricDataBlock();
            break;
              
          case PAUSE:
            FlushBuffer(100);
            break;                
        }
        break; // Case ORIC

    #endif // tapORIC
              
      case IDPAUSE:
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
              currentTask = GETID;
              count = 255;
              ForcePauseAfter0();
            }
          } else { 
            currentTask = GETID;
            if(EndOfFile==true) currentID=IDEOF;
          }
        } 
        break;
  
      case IDEOF:
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

  }
}

void StandardBlock() {
  //Standard Block Playback
  switch (currentBlockTask) {
    case PILOT:
      //Start with Pilot Pulses
      if(!pilotPulses--) {
        currentBlockTask = SYNC1;
      } else {
        currentPeriod = pilotLength;
      }
      break;
    
    case SYNC1:
      //First Sync Pulse
      currentPeriod = sync1Length;
      currentBlockTask = SYNC2;
      break;
    
    case SYNC2:
      //Second Sync Pulse
      currentPeriod = sync2Length;
      currentBlockTask = DATA;
      break;
    
    case DATA:  
      //Data Playback
#ifdef AYPLAY
      if ((AYPASS==0)||(AYPASS==4)||(AYPASS==5))
      {
        writeData();   // Check if we are playing from file or Vector String and we need to send first 0xFF byte or checksum byte at EOF
      }
      else
      {
        writeHeader2();            // write TAP Header data from String Vector (AYPASS=1)
      }
#else
      writeData();
#endif
      break;
    
    case PAUSE:
      //Close block with a pause
      if((currentID!=TAP)&&(currentID!=AYO)) {                  // Check if we have !=AYO too
        temppause = pauseLength;
        currentID = IDPAUSE;
      } else {
        currentPeriod = pauseLength;
        bitSet(currentPeriod, 15);
        currentBlockTask = READPARAM;
      }

      if(EndOfFile==true) currentID=IDEOF;
      break;
  }
}

#ifdef Use_UEF
void UEFCarrierToneBlock() {
  //Pure Tone Block - Long string of pulses with the same length
  if(!pilotPulses--) {
    currentTask = GETCHUNKID;
  } else {
    currentPeriod = pilotLength;
  }
}
#endif

void PureToneBlock() {
  //Pure Tone Block - Long string of pulses with the same length
  if(!pilotPulses--) {
    currentTask = GETID;
  } else {
    currentPeriod = pilotLength;
  }
}

void PulseSequenceBlock() {
  //Pulse Sequence Block - String of pulses each with a different length
  //Mainly used in speedload blocks
  if(!seqPulses--) {
    currentTask = GETID;
  } else {
    if(ReadWord()) {
      currentPeriod = TickToUs(outWord);    
    }
  }
}

void PureDataBlock() {
  //Pure Data Block - Data & pause only, no header, sync
  switch(currentBlockTask) {
    case DATA:
      writeData();          
    break;
    
    case PAUSE:
      temppause = pauseLength;
      currentID = IDPAUSE;
    break;
  }
}

/*
void KCSBlock() {
  //Kansas City Standard Block Playback (MSX specific)
  switch(currentBlockTask) {

    case PILOT:
      //Start with Pilot Pulses
      if (!pilotPulses--) {
        currentBlockTask = DATA;
      } else {
        currentPeriod = pilotLength;
      }
      break;

    case DATA:
      //Data playback
      writeData4B();
      break;
    
    case PAUSE:
      //Close block with a pause
      temppause = pauseLength;
      currentID = IDPAUSE;
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
      currentID=IDEOF;
      return;
    }
  }

  //End of block?
  if (bytesToRead==0 && currentBit==0) {
    temppause = pauseLength;
    currentBlockTask = PAUSE;
  }
}

void ZX81FilenameBlock() {
  //output ZX81 filename data
  if(currentBit==0) {                         //Check for byte end/first byte
    currentByte = pgm_read_byte(ZX81Filename+currentChar);
    currentChar+=1;
    if(currentChar==10) {
      currentBlockTask = DATA;
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
      if((bytesToRead == -1) && (currentID == ID19)) {    
        bytesRead += -1;                      //rewind a byte if we've reached the end
        temppause = PAUSELENGTH;
        currentID = IDPAUSE;
      }                   
    #endif 
          
    } else {
      EndOfFile=true;
      temppause = pauseLength;
      currentID = IDPAUSE;
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
          currentTask = PROCESSCHUNKID;
        } else {
          currentBlockTask = PAUSE;           //Otherwise start the pause
        }
      }
    } else {                         // If we reached the EOF
      currentTask = GETCHUNKID;
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
        currentTask = GETCHUNKID;
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
        currentTask = GETCHUNKID;
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

void writeData() {
  //Convert byte from file into string of pulses.  One pulse per pass
  if(currentBit==0) {                         //Check for byte end/first byte
    if(ReadByte()) {            //Read in a byte
      currentByte = outByte;
      #ifdef AYPLAY 
      if (AYPASS==5) {
        currentByte = 0xFF;                 // Only insert first DATA byte if sending AY TAP DATA Block and don't decrement counter
        AYPASS = 4;                         // set Checksum flag to be sent when EOF reached
        bytesRead += -1;                    // rollback ptr and compensate for dummy read byte
        bytesToRead += 2;                   // add 2 bytes to read as we send 0xFF (data flag header byte) and chksum at the end of the block
      } else {
      #endif
        bytesToRead += -1;  
      #ifdef AYPLAY 
      }
      blkchksum = blkchksum ^ currentByte;    // keep calculating checksum
      #endif
      

      if(bytesToRead == 0) {                  //Check for end of data block
        bytesRead += -1;                      //rewind a byte if we've reached the end
        
        if(pauseLength==0) {                  //Search for next ID if there is no pause
          //if (bitRead(currentPeriod, 14) == 0) currentTask = GETID;
          currentTask = GETID;
        } else {
          currentBlockTask = PAUSE;           //Otherwise start the pause
        }
        
        return;                               // exit
      }
    } else {                         // If we reached the EOF
      
    #ifdef AYPLAY
      // Check if need to send checksum
      if (AYPASS==4)
      {
        currentByte = blkchksum;            // send calculated chksum
        bytesToRead += 1;                   // add one byte to read
        AYPASS = 0;                         // Reset flag to end block
      }
      else
    #endif
      {
        EndOfFile=true;  
    
        if(pauseLength==0) {
          currentTask = GETID;
        } else {
          currentBlockTask = PAUSE;
        }
        return;                           // return here if normal TAP or TZX  
      }
    }

    if(bytesToRead!=1) {                      //If we're not reading the last byte play all 8 bits
      currentBit=8;
    } else {
      currentBit=usedBitsInLastByte;          //Otherwise only play back the bits needed
    }
    pass=0;
  }

  #ifdef DIRECT_RECORDING
  if bitRead(currentPeriod, 14) {
    if(currentByte&0x80) bitSet(currentPeriod, 13);
    pass+=2;
  }
  else
  {
  #endif
    if(currentByte&0x80){                       //Set next period depending on value of bit 0
      currentPeriod = onePulse;
    } else {
      currentPeriod = zeroPulse;
    }
    pass+=1;
  #ifdef DIRECT_RECORDING
  }
  #endif
  
  if(pass==2) {
    currentByte <<= 1;                        //Shift along to the next bit
    currentBit += -1;
    pass=0;  
  }    
}

void DirectRecording() {
  //Direct Recording - Output bits based on specified sample rate (Ticks per clock) either 44.1KHz or 22.05

  if(currentBit==0) {                         //Check for byte end/first byte
    if(ReadByte()) {            //Read in a byte
      currentByte = outByte;
      bytesToRead += -1; 
    }  
      
    if(bytesToRead == 0) {                  //Check for end of data block
      bytesRead += -1;                      //rewind a byte if we've reached the end
      if(pauseLength==0) {                  //Search for next ID if there is no pause
        //currentTask = GETID;
      } else {
        currentBlockTask = PAUSE;           //Otherwise start the pause
      }
      return;                               // exit
    }

    if(bytesToRead!=1) {                      //If we're not reading the last byte play all 8 bits
      currentBit=8;
    } else {
      currentBit=usedBitsInLastByte;          //Otherwise only play back the bits needed
    }
  } 
 
  if(currentByte&0x80) {                       //Set next period depending on value of bit 0
    bitSet(currentPeriod, 13);
  }

  currentByte <<= 1;                        //Shift along to the next bit
  currentBit += -1;               
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
      count =255;
      currentTask = GETID;
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
      currentBlockTask = PAUSE;
    }else {
      currentTask = GETID;
      return;
    }
  #else
    count = 255;
    currentBlockTask = PAUSE;
  #endif
  }    
}

void wave2() {
  //ISR Output routine
  word workingPeriod = word(wbuffer[pos][workingBuffer], wbuffer[pos+1][workingBuffer]);  
  byte pauseFlipBit = false;
  unsigned long newTime=1;
  intError = false;
 
  if(isStopped==0 && workingPeriod >= 1)
  {
    if bitRead(workingPeriod, 15)          
    {
      //If bit 15 of the current period is set we're about to run a pause
      //Pauses start with a 1.5ms where the output is untouched after which the output is set LOW
      //Pause block periods are stored in milliseconds not microseconds
      isPauseBlock = true;
      bitClear(workingPeriod,15);         //Clear pause block flag
      pauseFlipBit = true;
    }
    
    #ifdef DIRECT_RECORDING
    if (bitRead(workingPeriod, 14)== 0) {
    #endif
      pinState = !pinState;
      if (pinState == LOW)     WRITE_LOW;    
      else  WRITE_HIGH;
    #ifdef DIRECT_RECORDING
    } else {
      if (bitRead(workingPeriod, 13) == 0)     WRITE_LOW;    
      else  {WRITE_HIGH; bitClear(workingPeriod,13);}     
      bitClear(workingPeriod,14);         //Clear ID15 flag
      workingPeriod = SampleLength;
    }
    #endif

    if(pauseFlipBit==true) {
      newTime = 1500;                     //Set 1.5ms initial pause block
      pinState = TSXCONTROLzxpolarityUEFSWITCHPARITY;
      workingPeriod = workingPeriod - 1; 
      if (bitRead(workingPeriod, 13)) {
        bitClear(workingPeriod,13);                       
        pinState = LOW;          
        newTime = workingPeriod;                             
        workingPeriod = 0;           
      }

      wbuffer[pos][workingBuffer] = workingPeriod /256;  //reduce pause by 1ms as we've already pause for 1.5ms
      wbuffer[pos+1][workingBuffer] = workingPeriod  %256;  //reduce pause by 1ms as we've already pause for 1.5ms                 
      pauseFlipBit=false;
    } else {
      if(isPauseBlock==true) {
        newTime = long(workingPeriod)*1000; //Set pause length in microseconds
        isPauseBlock = false;
      } else {
        newTime = workingPeriod;          //After all that, if it's not a pause block set the pulse period 
      }
      //pos += 1;
      pos += 2;
      if(pos >= buffsize)                  //Swap buffer pages if we've reached the end
      {
        pos = 0;
        workingBuffer^=1;
        morebuff = HIGH;                  //Request more data to fill inactive page
      } 
    }

  } else if (isStopped==0) {  
    newTime = 1000;                         //Just in case we have a 0 in the buffer
    pos += 2;
    if(pos >= buffsize) {
      pos = 0;
      workingBuffer ^= 1;
      morebuff = HIGH;
    }
  } else {
    newTime = 50000;                         //Just in case we have a 0 in the buffer    
  }
  
  newTime += 6; // why 6? STM32F1 code used to do newTime += 2.  Why 2?
  Timer.setPeriod(newTime); //Finally set the next pulse length
}

#ifdef AYPLAY
void writeHeader2() {
  //Convert byte from HDR Vector String into string of pulses and calculate checksum. One pulse per pass
  if(currentBit==0) {                         //Check for byte end/new byte                         
    if(hdrptr==19) {              // If we've reached end of header block send checksum byte
      currentByte = blkchksum;
      AYPASS = 2;                 // set flag to Stop playing from header in RAM 
      currentBlockTask = PAUSE;   // we've finished outputting the TAP header so now PAUSE and send DATA block normally from file
      return;
    }
    hdrptr += 1;                   // increase header string vector pointer
    if(hdrptr<20) {                     //Read a byte until we reach end of tap header
      currentByte = pgm_read_byte(TAPHdr+hdrptr);
      if (hdrptr>=3 && hdrptr<=12) {
        if (hdrptr-3 < strlen(fileName)) {
          currentByte = fileName[hdrptr-3];
          if (currentByte<0x20 || currentByte>0x7f) {
            currentByte = '?';
          }
        } else {
          currentByte = ' ';
        }
      }
      else if(hdrptr==13){                           // insert calculated block length minus LEN bytes
        currentByte = lowByte(ayblklen-3);
      } else if(hdrptr==14){
        currentByte = highByte(ayblklen);
      }
      blkchksum = blkchksum ^ currentByte;    // Keep track of Chksum
      currentBit=8;
    } else {
      currentBit=usedBitsInLastByte;          //Otherwise only play back the bits needed
    }   
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

void clearBuffer2()
{
  for(int i=0;i<buffsize;i++)
  {
    wbuffer[i][0]=0;
    wbuffer[i][1]=0;
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
    case 3600:
      //scale=2713/1200;
      scale=2;
      //period=93; //2700 baudios
      //period = TickToUs(243);
      period=70;
      break;      
    case 3850:
      scale=2;
      period = 68; //3675 baudios con period=68, 3760 con period=66.5
      break;
  }
  Timer.stop();
}


void uniLoop() {
 #ifdef Use_CAS
    if (casduino)
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

void DelayedStop() {
  if(!count==0) {
    currentPeriod = 10;
    bitSet(currentPeriod, 15); 
    bitSet(currentPeriod, 13);
    count += -1;
  } else {
    stopFile();
    return;
  }       
}

void FlushBuffer(long newcount) {
  if(count>0) {
    currentPeriod = ORICONEPULSE;
    count--;
  } else {   
    count= newcount;
    currentBlockTask=SYNC1;
    #ifdef MenuBLK2A
      if (!skip2A) ForcePauseAfter0();
    #endif
  }             
  return;
}

void ForcePauseAfter0() {
    pauseOn=1;
    printtext2F(PSTR("PAUSED* "),0);
    forcePause0=0;
    return;  
}
