
word TickToUs(word ticks) {
  return (word) ((((float) ticks)/3.5)+0.5);
}

void UniPlay(char *filename){
  Timer1.stop();                              //Stop timer interrupt
#ifdef SDFat
  if(!entry.open(filename,O_READ)) {
  //  printtextF(PSTR("Error Opening File"),0);
  }
#else
  if(!SD.open(filename,O_READ)) {
  //  printtextF(PSTR("Error Opening File"),0);
  }
#endif
  block=0;                                    // Initial block when starting
//  currpct = 100;
//  newpct = 0;
/*
  for(int i=0;i<maxblock;i++)
  {
    blockOffset[i] = 0;
    blockID[i] = 0;
  }
*/
  currentBit=0;                               // fallo reproducción de .tap tras .tzx
  bytesRead=0;                                //start of file
  currentTask=GETFILEHEADER;                  //First task: search for header
  checkForEXT (filename);
  if (!casduino) {
    currentBlockTask = READPARAM;               //First block task is to read in parameters
    clearBuffer2();                               // chick sound with CASDUINO clearBuffer()
    isStopped=false;
    count = 255;                                //End of file buffer flush
    EndOfFile=false;
    passforZero=2;
    passforOne=4;
   // pinState=LOW;                               //Always Start on a LOW output for simplicity
   // digitalWrite(outputPin, pinState);
//    digitalWrite(outputPin, LOW);
    WRITE_LOW;    
    Timer1.initialize(1000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);

//    Timer1.setPeriod(1000);                     //set 1ms wait at start of a file.
  }
#ifdef Use_CAS
  else {
    Timer1.initialize(period);
    Timer1.attachInterrupt(wave);    
  }
#endif
}

/*
void UniPlay(char *filename) {
  Timer1.stop();                              //Stop timer interrupt
  if(!entry.open(filename,O_READ)) {          //open file and check for errors
    printtextF(PSTR("Error Opening File"),0);
    //lcd_clearline(0);
    //lcd.print(F("Error Opening File"));    
  } 
  bytesRead=0;                                //start of file
  currentTask=GETFILEHEADER;                  //First task: search for header
  checkForEXT (filename);
  currentBlockTask = READPARAM;               //First block task is to read in parameters
  clearBuffer();
  isStopped=false;
  pinState=LOW;                               //Always Start on a LOW output for simplicity
  count = 255;                                //End of file buffer flush
  EndOfFile=false;
  digitalWrite(outputPin, pinState);
  Timer1.setPeriod(1000);                     //set 1ms wait at start of a file.
}
*/

void TZXStop() {
  Timer1.stop();                              //Stop timer
  isStopped=true;
  start=0;
  entry.close();                              //Close file                                                                                // DEBUGGING Stuff
  //lcd.setCursor(0,1);
  //lcd.print(blkchksum,HEX); lcd.print(F("ck ")); lcd.print(bytesRead); lcd.print(F(" ")); lcd.print(ayblklen);
  REWIND=1;   
  seekFile(currentFile); 
  bytesRead=0;                                // reset read bytes PlayBytes
  blkchksum = 0;                              // reset block chksum byte for AY loading routine
  AYPASS = 0;                                 // reset AY flag
  casduino=0;
/*  Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
  Timer1.attachInterrupt(wave2);
  Timer1.stop();     */                       //Stop the timer until we're ready   
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

    if(btemppos<=buffsize){                    // Keep filling until full
      TZXProcess();                           //generate the next period to add to the buffer
      if(currentPeriod>0) {
        noInterrupts();                       //Pause interrupts while we add a period to the buffer
        //wbuffer[btemppos][workingBuffer ^ 1] = currentPeriod;   //add period to the buffer
        wbuffer[btemppos][workingBuffer ^ 1] = currentPeriod /256;   //add period to the buffer
        //wbuffer[btemppos][workingBuffer ^ 1] = highByte(currentPeriod);
        wbuffer[btemppos+1][workingBuffer ^ 1] = currentPeriod %256;   //add period to the buffer
        //wbuffer[btemppos+1][workingBuffer ^ 1] = lowByte(currentPeriod);   
        interrupts();
        //btemppos+=1;
        btemppos+=2;        
      }
    } else {
         //lcdSpinner();
         if (pauseOn == 0) {
          lcdTime();
          lcdPercent();
         }
    } 
}

void TZXProcess() {
    byte r = 0;
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
      //Serial.println(F("GETCHUNKID"));
      //Serial.print(F("offset:"));
      //Serial.println(bytesRead,HEX);
      //grab 2 byte ID
      if(r=ReadWord(bytesRead)==2) {
         chunkID = outWord;
         if(r=ReadDword(bytesRead)==4) {
            bytesToRead = outLong;
            //Serial.print(F("bytesToRead: "));
            //Serial.println(bytesToRead,DEC);
            parity = 0;  
            
 #if defined(Use_UEF) && defined(Use_hqUEF) && defined(Use_c104)          
            if (chunkID == ID0104) {
              //bytesRead+= 3;
              bytesToRead+= -3;
              bytesRead+= 1;
              //grab 1 byte Parity
              if(ReadByte(bytesRead)==1) {
                if (outByte == 'O') parity = TSXCONTROLzxpolarityUEFSWITCHPARITY ? 2 : 1;
                else if (outByte == 'E') parity = TSXCONTROLzxpolarityUEFSWITCHPARITY ? 1 : 2;
                else parity = 0 ;  // 'N'
              }
              bytesRead+= 1;                                         
            }
 #endif
         } else {
             chunkID = IDCHUNKEOF;
         }
      } else {
        chunkID = IDCHUNKEOF;
      }
      //if (!(TSXCONTROLzxpolarityUEFSWITCHPARITY)) {
      if ( BAUDRATE == 1200) {
         zeroPulse = UEFZEROPULSE;
         onePulse = UEFONEPULSE;
      } else {
         zeroPulse = UEFTURBOZEROPULSE;
         onePulse = UEFTURBOONEPULSE;
      }  
         //zeroPulse = (BAUDRATE == 1200)? UEFZEROPULSE : UEFTURBOZEROPULSE;
         //onePulse = (BAUDRATE == 1200)? UEFONEPULSE : UEFTURBOONEPULSE;

      //zeroPulse = UEFZEROPULSE;
      //onePulse = UEFONEPULSE;
       
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
          //bytesRead+=bytesToRead;
          //parity = 0; // NoParity
          /* stopBits = */ //stopBitPulses = 1;
          writeUEFData();
          break;
 #if defined(Use_UEF) && defined(Use_hqUEF) && defined(Use_c104)         
        case ID0104:          
          //parity = 1; // ParityOdd i.e complete with value to get Odd number of ones
          /* stopBits = */ //stopBitPulses = 1;
          writeUEFData();
          //bytesRead+=bytesToRead;
          break; 
 #endif               
        case ID0110:
          if(currentBlockTask==READPARAM){
            if(r=ReadWord(bytesRead)==2) {
              //if (!(TSXCONTROLzxpolarityUEFSWITCHPARITY)) {
              if (BAUDRATE == 1200) {                     
                 pilotPulses = UEFPILOTPULSES;
                 pilotLength = UEFPILOTLENGTH;
              } else {
                // turbo mode    
                 pilotPulses = UEFTURBOPILOTPULSES;
                 pilotLength = UEFTURBOPILOTLENGTH;
              } 
                 //pilotPulses = (BAUDRATE == 1200)? UEFPILOTPULSES : UEFTURBOPILOTPULSES;
                 //pilotLength = (BAUDRATE == 1200)? UEFPILOTLENGTH : UEFTURBOPILOTLENGTH;

              
                //pilotPulses = UEFPILOTPULSES;
                //pilotLength = UEFPILOTLENGTH;                     
            }
            currentBlockTask = PILOT;
          } 
          else {
            UEFCarrierToneBlock();
          }
       //   UEFCarrierToneBlock();
          //bytesRead+=bytesToRead;
          //currentTask = GETCHUNKID;
          break;
#endif

#if defined(Use_UEF) && defined(Use_hqUEF)
        case ID0111:
          if(currentBlockTask==READPARAM){
            if(r=ReadWord(bytesRead)==2) {             
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

          
       //   UEFCarrierToneBlock();
          //bytesRead+=bytesToRead;
          //currentTask = GETCHUNKID;
                 
          break;
#endif


#if defined(Use_UEF) && defined(Use_c112)         
        case ID0112:
//          if(currentBlockTask==READPARAM){
            if(r=ReadWord(bytesRead)==2) {
              if (outWord>0) {
                //Serial.print(F("delay="));
                //Serial.println(outWord,DEC);
                temppause = outWord;
                
                currentID = IDPAUSE;
                currentPeriod = temppause;
                bitSet(currentPeriod, 15);
                currentTask = GETCHUNKID;
              } else {
                currentTask = GETCHUNKID;
              }     
            }
//          } 
          break;
#endif

#if defined(Use_UEF) && defined(Use_hqUEF) && defined(Use_c114)
        case ID0114: 
          if(r=ReadWord(bytesRead)==2) {
            pilotPulses = UEFPILOTPULSES;
            //pilotLength = UEFPILOTLENGTH;
            bytesRead-=2; 
          }
          UEFCarrierToneBlock();
          bytesRead+=bytesToRead;
          currentTask = GETCHUNKID;
          break;          
#endif

#if defined(Use_UEF) && defined(Use_hqUEF) && defined(Use_c116)
        case ID0116:
            if(r=ReadDword(bytesRead)==4) {
              byte * FloatB = (byte *) &outLong;
              //memcpy(&outFloat,&outLong,4);
              //outWord = (int)(outFloat+.9)*1000;
              //outWord = int(outFloat+.9) <<10;
            //outWord = *((float *)&outLong);
              //outWord = (((*(&outLong+2)&0x80) >> 7) | (*(&outLong+3)&0x7f) << 1) + 10;
              //outWord = *(&outLong) | (*(&outLong+1))<<8  | ((outWord&1)<<7)<<16 | (outWord>>1)<<24  ;
              outWord = (((*(FloatB+2)&0x80) >> 7) | (*(FloatB+3)&0x7f) << 1) + 10;
              outWord = *FloatB | (*(FloatB+1))<<8  | ((outWord&1)<<7)<<16 | (outWord>>1)<<24  ;
              outFloat = *((float *) &outWord);
              outWord= (int) outFloat;
              
              if (outWord>0) {
                //Serial.print(F("delay="));
                //Serial.println(outWord,DEC);
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
#endif

#if defined(Use_UEF) && defined(Use_hqUEF) && defined(Use_c117)
        case ID0117:
            if(r=ReadWord(bytesRead)==2) {
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

#endif


#ifdef Use_UEF
        case IDCHUNKEOF:
          //Serial.println(F("IDCHUNKEOF"));
          bytesRead+=bytesToRead;
          stopFile();
          return;
        default:
          //Serial.print(F("Skip id "));
          //Serial.print(chunkID);
          bytesRead+=bytesToRead;
          currentTask = GETCHUNKID;
          break;
            
      }
      //currentTask = GETCHUNKID;
    }      
#endif
    
    if(currentTask == GETID) {
      //grab 1 byte ID
      if(ReadByte(bytesRead)==1) {
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
                EEPROM.put(BLOCK_EEPROM_START+5*block, bytesRead);
                EEPROM.put(BLOCK_EEPROM_START+4+5*block, currentID);
              #endif
         
              #ifdef OLED1306
                    #ifdef XY
                      setXY(7,2);
                      sendChar('1');sendChar('0');
                      setXY(14,2);
                  //    if (block == 0) sendChar('0');
                  //    if (block == 10) sendChar('1');
                      if ((block%10) == 0) sendChar(48+block/10);  
                      setXY(15,2);
                      sendChar(48+block%10);   
                    #endif
                    #ifdef XY2
                      setXY(11,1);
                      if ((block%10) == 0) sendChar(48+block/10);
                      setXY(12,1);
                      sendChar(48+block%10);
                    #endif
              #endif
              #ifdef BLOCKID_INTO_MEM
                if (block < maxblock) block++;
                else block = 0; 
              #endif
              #ifdef BLOCK_EEPROM_PUT       
                if (block < 99) block++;
                else block = 0; 
              #endif              
              if(r=ReadWord(bytesRead)==2) {
                pauseLength = outWord;
              }
              if(r=ReadWord(bytesRead)==2) {
                bytesToRead = outWord +1;
              }
              if(r=ReadByte(bytesRead)==1) {
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
                EEPROM.put(BLOCK_EEPROM_START+5*block, bytesRead);
                EEPROM.put(BLOCK_EEPROM_START+4+5*block, currentID);
              #endif
              
              #ifdef OLED1306
                    #ifdef XY
                      setXY(7,2);
                      sendChar('1');sendChar('1');
                      setXY(14,2);
                  //    if (block == 0) sendChar('0');
                  //    if (block == 10) sendChar('1');
                      if ((block%10) == 0) sendChar(48+block/10);  
                      setXY(15,2);
                      sendChar(48+block%10);   
                    #endif
                    #ifdef XY2
                      setXY(11,1);
                      if ((block%10) == 0) sendChar(48+block/10);
                      setXY(12,1);
                      sendChar(48+block%10);
                    #endif
              #endif   
              #ifdef BLOCKID_INTO_MEM
                if (block < maxblock) block++;
                else block = 0; 
              #endif
              #ifdef BLOCK_EEPROM_PUT       
                if (block < 99) block++;
                else block = 0; 
              #endif            
              if(r=ReadWord(bytesRead)==2) {
                pilotLength = TickToUs(outWord);
              }
              if(r=ReadWord(bytesRead)==2) {
                sync1Length = TickToUs(outWord);
              }
              if(r=ReadWord(bytesRead)==2) {
                sync2Length = TickToUs(outWord);
              }
              if(r=ReadWord(bytesRead)==2) {
                zeroPulse = TickToUs(outWord);
              }
              if(r=ReadWord(bytesRead)==2) {
                onePulse = TickToUs(outWord);
              }              
              if(r=ReadWord(bytesRead)==2) {
                pilotPulses = outWord;
              }
              if(r=ReadByte(bytesRead)==1) {
                usedBitsInLastByte = outByte;
              }
              if(r=ReadWord(bytesRead)==2) {
                pauseLength = outWord;
              }
              if(r=ReadLong(bytesRead)==3) {
                bytesToRead = outLong +1;
              }
              currentBlockTask = PILOT;
          break;
          
          default:
            StandardBlock();
          break;
          }

        break;
        case ID12:
        //Process ID12 - Pure Tone Block
          if(currentBlockTask==READPARAM){
            if(r=ReadWord(bytesRead)==2) {
               pilotLength = TickToUs(outWord);
            }
            if(r=ReadWord(bytesRead)==2) {
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
            if(r=ReadByte(bytesRead)==1) {
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
            if(r=ReadWord(bytesRead)==2) {
              zeroPulse = TickToUs(outWord); 
            }
            if(r=ReadWord(bytesRead)==2) {
              onePulse = TickToUs(outWord); 
            }
            if(r=ReadByte(bytesRead)==1) {
              usedBitsInLastByte = outByte;
            }
            if(r=ReadWord(bytesRead)==2) {
              pauseLength = outWord; 
            }
            if(r=ReadLong(bytesRead)==3) {
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
            if(r=ReadWord(bytesRead)==2) {     
              //Number of T-states per sample (bit of data) 79 or 158 - 22.6757uS for 44.1KHz
              //SampleLength = TickToUs(outWord) + TickToUs(outWord)/9;
              //SampleLength = TickToUs(outWord + (outWord /8));
              SampleLength = TickToUs(outWord);
            }
            if(r=ReadWord(bytesRead)==2) {      
              //Pause after this block in milliseconds
              pauseLength = outWord;  
            }
            if(r=ReadByte(bytesRead)==1) {
            //Used bits in last byte (other bits should be 0)
              usedBitsInLastByte = outByte;
            }
            if(r=ReadLong(bytesRead)==3) {
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
            //DirectRecording();
            writeData();
          }
        break;
        #endif

        case ID19:
          //Process ID19 - Generalized data block
          switch (currentBlockTask) {
            case READPARAM:
        #ifdef ID19REW      
              #ifdef BLOCKID_INTO_MEM
                    blockOffset[block%maxblock] = bytesRead;
                    blockID[block%maxblock] = currentID;
              #endif
              #ifdef BLOCK_EEPROM_PUT
                    EEPROM.put(BLOCK_EEPROM_START+5*block, bytesRead);
                    EEPROM.put(BLOCK_EEPROM_START+4+5*block, currentID);
              #endif
        //#ifdef ID19REW                
              #ifdef OLED1306
                    #ifdef XY
                      setXY(7,2);
                      sendChar('1');sendChar('9');
                      setXY(14,2);
                      if ((block%10) == 0) sendChar(48+block/10);  
                      setXY(15,2);
                      sendChar(48+block%10);   
                    #endif
                    #ifdef XY2
                      setXY(11,1);
                      if ((block%10) == 0) sendChar(48+block/10);
                      setXY(12,1);
                      sendChar(48+block%10);
                    #endif
              #endif
        //#endif              
              #ifdef BLOCKID_INTO_MEM
                if (block < maxblock) block++;
                else block = 0; 
              #endif
              #ifdef BLOCK_EEPROM_PUT       
                if (block < 99) block++;
                else block = 0; 
              #endif 
        #endif
              if(r=ReadDword(bytesRead)==4) {
                #ifdef ID19REW
                  bytesToRead = outLong;
                #endif
              }
              if(r=ReadWord(bytesRead)==2) {
                //Pause after this block in milliseconds
                pauseLength = outWord;
                #ifdef ID19REW
                  //bytesToRead += -2;
                #endif              
              }
              bytesRead += 86 ;  // skip until DataStream filename
              #ifdef ID19REW
                //bytesToRead += -86 ; // skip SYMDEF bytes inside block ID                       
                bytesToRead += -88 ;    // pauseLength + SYMDEFs
              #endif
              //currentChar=0;
              currentBlockTask=PAUSE;
            break;
        /*    
            case PILOT:
              //ZX81FilenameBlock();
              bytesRead += 3 ;  // skip until DataStream .P file offset
              currentBlockTask = DATA;             
            break;
        */ 
            case PAUSE:
              currentPeriod = PAUSELENGTH;
              bitSet(currentPeriod, 15);
              currentBlockTask=DATA;
            break; 
                         
            case DATA:
              //if (block <= 10) {
                //bytesRead += bytesToRead;
                //bytesToRead = 0;
                //currentTask = GETID;
              //} else {
              ZX8081DataBlock();
              //}
            break;
          }  
        break;

      /*  //Old ID20
        case ID20:
          //process ID20 - Pause Block
          if(r=ReadWord(bytesRead)==2) {
            if(outWord>0) {
              currentPeriod = pauseLength;
              bitSet(currentPeriod, 15);
            }
            currentTask=GETID;
          }
        break; */     
       
        case ID20:
          //process ID20 - Pause Block
          if(r=ReadWord(bytesRead)==2) {
            if(outWord>0) {
              forcePause0=0;          // pause0 FALSE
              temppause = outWord;
       //       currentID = IDPAUSE;
            } else {                    // If Pause duration is 0 ms then Stop The Tape
              forcePause0=1;          // pause0 TRUE
        //      currentTask = GETID;
            }
            currentID = IDPAUSE;         
          }
        break;

        case ID21:
          //Process ID21 - Group Start
          if(r=ReadByte(bytesRead)==1) {
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
          if(r=ReadWord(bytesRead)==2) {
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
          if(r=ReadByte(bytesRead)==1) {
            //Show info on screen - removed until bigger screen used
            //byte j = outByte;
            //for(byte i=0; i<j; i++) {
            //  if(ReadByte(bytesRead)==1) {
            //    lcd.print(char(outByte));
            //  }
            //}
            bytesRead += outByte;
          }
          currentTask = GETID;
        break;

        case ID31:
          //Process ID31 - Message block
           if(r=ReadByte(bytesRead)==1) {
            // dispayTime = outByte;
          }         
          if(r=ReadByte(bytesRead)==1) {
            bytesRead += outByte;
          }
          currentTask = GETID;
        break;

        case ID32:
          //Process ID32 - Archive Info
          //Block Skipped until larger screen used
          if(ReadWord(bytesRead)==2) {
            bytesRead += outWord;
          }
          currentTask = GETID;
        break;

        case ID33:
          //Process ID32 - Archive Info
          //Block Skipped until larger screen used
          if(ReadByte(bytesRead)==1) {
            bytesRead += (long(outByte) * 3);
          }
          currentTask = GETID;
        break;       

        case ID35:
          //Process ID35 - Custom Info Block
          //Block Skipped
          bytesRead += 0x10;
          if(r=ReadDword(bytesRead)==4) {
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
                EEPROM.put(BLOCK_EEPROM_START+5*block, bytesRead);
                EEPROM.put(BLOCK_EEPROM_START+4+5*block, currentID);
              #endif
              
              #ifdef OLED1306
                    #ifdef XY
                      setXY(7,2);
                      sendChar('4');sendChar('B');
                      setXY(14,2);
                  //    if (block == 0) sendChar('0');
                  //    if (block == 10) sendChar('1');
                      if ((block%10) == 0) sendChar(48+block/10);  
                      setXY(15,2);
                      sendChar(48+block%10);   
                    #endif
                    #ifdef XY2
                      setXY(11,1);
                      if ((block%10) == 0) sendChar(48+block/10);
                      setXY(12,1);
                      sendChar(48+block%10);
                    #endif
              #endif     
              #ifdef BLOCKID_INTO_MEM
                if (block < maxblock) block++;
                else block = 0; 
              #endif
              #ifdef BLOCK_EEPROM_PUT       
                if (block < 99) block++;
                else block = 0; 
              #endif 
              if(r=ReadDword(bytesRead)==4) {  // Data size to read
                bytesToRead = outLong - 12;
              }
              if(r=ReadWord(bytesRead)==2) {  // Pause after block in ms
                pauseLength = outWord;
              }
              if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 0){
                  if(r=ReadWord(bytesRead)==2) {  // T-states each pilot pulse
                    pilotLength = TickToUs(outWord);
                  }
                  if(r=ReadWord(bytesRead)==2) {  // Number of pilot pulses
                    pilotPulses = outWord;
                  }
                  if(r=ReadWord(bytesRead)==2) {  // T-states 0 bit pulse
                    zeroPulse = TickToUs(outWord);
                  }
                  if(r=ReadWord(bytesRead)==2) {  // T-states 1 bit pulse
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
                    //  pilotLength = onePulse = TickToUs(319); //2800
                    //  zeroPulse = TickToUs(628);
                      /* pilotLength = onePulse = TickToUs(252); //3500
                      zeroPulse = TickToUs(504); */
                      pilotLength = onePulse = TickToUs(243); //3600
                      zeroPulse = TickToUs(486);                    
                      break;
                    case 3850:
                      /* pilotLength = onePulse = TickToUs(238); //3675
                      zeroPulse = TickToUs(476);   */    
                      /* pilotLength = onePulse = TickToUs(236); //3700
                      zeroPulse = TickToUs(472);   */                    
                //      pilotLength = onePulse = TickToUs(233); //3760
                //      zeroPulse = TickToUs(466);
                      pilotLength = onePulse = 65; //3850=1000000/(65*4)
                      zeroPulse = 130;                  
                      //pilotLength = onePulse = TickToUs(225); //3900
                      //zeroPulse = TickToUs(450);                                                                                           
                      break;
                  }
               
              } //End of TSX_SPEEDUP

              if(r=ReadByte(bytesRead)==1) {  // BitCfg
                oneBitPulses =  outByte & 0x0f;       //(default:4)
                zeroBitPulses = outByte >> 4;         //(default:2)
                if (!oneBitPulses) oneBitPulses = 16;
                if (!zeroBitPulses) zeroBitPulses = 16;
              }
              if(r=ReadByte(bytesRead)==1) {  // ByteCfg
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
        break;

        case TAP:
          //Pure Tap file block
          switch(currentBlockTask) {
            case READPARAM:
              #ifdef BLOCKID_INTO_MEM
                blockOffset[block%maxblock] = bytesRead;
                blockID[block%maxblock] = currentID;
              #endif
              #ifdef BLOCK_EEPROM_PUT
                EEPROM.put(BLOCK_EEPROM_START+5*block, bytesRead);
                EEPROM.put(BLOCK_EEPROM_START+4+5*block, currentID);
              #endif
              
              #ifdef OLED1306
                    #ifdef XY
                      setXY(7,2);
                      sendChar('F');sendChar('E');
                      setXY(14,2);
                  //    if (block == 0) sendChar('0');
                  //    if (block == 10) sendChar('1');
                      if ((block%10) == 0) sendChar(48+block/10);  
                      setXY(15,2);
                      sendChar(48+block%10);   
                    #endif
                    #ifdef XY2
                      setXY(11,1);
                      if ((block%10) == 0) sendChar(48+block/10);
                      setXY(12,1);
                      sendChar(48+block%10);
                    #endif
              #endif     
              #ifdef BLOCKID_INTO_MEM
                if (block < maxblock) block++;
                else block = 0; 
              #endif
              #ifdef BLOCK_EEPROM_PUT       
                if (block < 99) block++;
                else block = 0; 
              #endif   
              pauseLength = PAUSELENGTH;
              if(r=ReadWord(bytesRead)==2) {
                    bytesToRead = outWord+1;
              }
              if(r=ReadByte(bytesRead)==1) {
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
        break;

        case ZXP:
          switch(currentBlockTask) {
            case READPARAM:
              //pauseLength = PAUSELENGTH*5;
              pauseLength = PAUSELENGTH;
              currentChar=0;
              currentBlockTask=PAUSE;
            break;
            
            case PAUSE:
              currentPeriod = PAUSELENGTH;
              bitSet(currentPeriod, 15);
              currentBlockTask=PILOT;
            break; 
                        
            case PILOT:
              ZX81FilenameBlock();
            break;
            
            case DATA:
              ZX8081DataBlock();
            break;
          }
        break;

        case ZXO:
          switch(currentBlockTask) {
            case READPARAM:
              pauseLength = PAUSELENGTH*5;
              currentBlockTask=PAUSE;
            break;
                        
            case PAUSE:
              currentPeriod = PAUSELENGTH;
              bitSet(currentPeriod, 15);
              currentBlockTask=PILOT;
            break; 
            
            case DATA:
              ZX8081DataBlock();
            break; 
            
          }
        break;
        
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
              //if(hdrptr==1) {
              //bytesToRead = 0x13-2; // 0x13 0x0 - TAP Header minus 2 (FLAG and CHKSUM bytes) 17 bytes total 
              //}
              if(hdrptr==HDRSTART) {
              //if (!AYPASS) {
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
        break;
      #endif

      #ifdef tapORIC
        case ORIC:
          switch(currentBlockTask) {
            case READPARAM:
              pauseLength = PAUSELENGTH;
              currentChar=0;
              currentBlockTask=PILOT;
            break;
            
            case PILOT:
  
            break;
            
            case DATA:
  
            break;
          }
        #endif 
                
        case IDPAUSE:
         /*     currentPeriod = temppause;
              temppause = 0;
              currentTask = GETID; 
              bitSet(currentPeriod, 15);       */             
          if(temppause>0) {
            if(temppause > MAXPAUSE_PERIOD) {
              //Serial.println(temppause, DEC);
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
                  pauseOn=1;
                  currentTask = GETID;
                  printtext2F(PSTR("PAUSED* "),0);                  
                  forcePause0=0;
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
            currentPeriod = 32769;
            count += -1;
          } else {
            stopFile();
            return;
          }       

        break; 
        
        default:
          //stopFile();
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
            ltoa(bytesRead,PlayBytes,16);
            lcd.print(PlayBytes) ;  lcd.print(" - L: "); lcd.print(loopCount);
          #endif

          #ifdef OLED1306
             
   /*           u8g.firstPage();
              do {  
                  u8g.drawStr ( 0, 15, "ID? ");
                  u8g.setPrintPos(40, 15); 
                  u8g.print(String(currentID, HEX));
                  u8g.setPrintPos(0, 30); 
                  u8g.print(String(bytesRead,HEX) + " - L: " + String(loopCount, DEC));
              } while( u8g.nextPage() ); */
              //printtextF(PSTR("ID? "),0);
              //utoa(currentID,PlayBytes,16);setXY(4,0);sendStr((unsigned char *)PlayBytes);
              //ltoa(bytesToRead,PlayBytes,16);strcat_P(PlayBytes,PSTR(" - L: "));printtext(PlayBytes,lineaxy);

              ltoa(bytesRead,PlayBytes,16);printtext(PlayBytes,lineaxy);
              //ltoa(bytesRead,PlayBytes,16);strcat_P(PlayBytes,PSTR(" - L: "));printtext(PlayBytes,lineaxy);
              //utoa(loopCount,PlayBytes,10);setXY(10,lineaxy);sendStr((unsigned char *)PlayBytes);

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
            ltoa(bytesRead,PlayBytes,16);
            lcd.print(PlayBytes) ;  lcd.print(" - L: "); lcd.print(loopCount);
          #endif
  
          //delay(8000);
           noInterrupts();  
           while(digitalRead(btnStop)==HIGH) {
             //waits until the button Stop is pressed.
             //delay(50);
           }
          interrupts();
          stopFile();
        break;
      }
    }
}

void StandardBlock() {
  //Standard Block Playback
  switch (currentBlockTask) {
    case PILOT:
        //Start with Pilot Pulses
        currentPeriod = pilotLength;
        pilotPulses += -1;
        if(pilotPulses == 0) {
          currentBlockTask = SYNC1;
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
      if ((AYPASS==0)|(AYPASS==4)|(AYPASS==5)) writeData();   // Check if we are playing from file or Vector String and we need to send first 0xFF byte or checksum byte at EOF
      else {
        writeHeader2();            // write TAP Header data from String Vector (AYPASS=1)
      }
    break;
    
    case PAUSE:
      //Close block with a pause
        // DEBUG
        //lcd.setCursor(0,1);
        //lcd.print(blkchksum,HEX); lcd.print(F("ck ptr:")); lcd.print(hdrptr);
            
 
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
  currentPeriod = pilotLength;
  pilotPulses += -1;
  if(pilotPulses==0) {
    currentTask = GETCHUNKID;
  }
}

#endif

void PureToneBlock() {
  //Pure Tone Block - Long string of pulses with the same length
  currentPeriod = pilotLength;
  pilotPulses += -1;
  if(pilotPulses==0) {
    currentTask = GETID;
  }
}

void PulseSequenceBlock() {
  //Pulse Sequence Block - String of pulses each with a different length
  //Mainly used in speedload blocks
  byte r=0;
  if(r=ReadWord(bytesRead)==2) {
    currentPeriod = TickToUs(outWord);    
  }
  seqPulses += -1;
  if(seqPulses==0) {
    currentTask = GETID;
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
  byte r;
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
    } else
    //Stop bits
    if (currentBit==1 && stopBitPulses) {
      currentPeriod = stopBitValue ? onePulse : zeroPulse;
      pass+=1;
      if ( pass==stopBitPulses )  {
        currentBit += -1;
        pass = 0;
      }
    } else
    //Data bits
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
  else
  if (currentBit==0 && bytesToRead!=0) {
    //Read new byte
    if (r=ReadByte(bytesRead)==1) {
      bytesToRead += -1;
      currentByte = outByte;
      currentBit = 10;
      pass = 0;
    } else if (r==0) {
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
  //output ZX81 filename data  byte r;
  if(currentBit==0) {                         //Check for byte end/first byte
      //currentByte=ZX81Filename[currentChar];
      currentByte = pgm_read_byte(ZX81Filename+currentChar);
      currentChar+=1;
      if(currentChar==10) {
        currentBlockTask = DATA;
        return;
       }
    currentBit=9;
    pass=0;
  }
  /*currentPeriod = ZX80PULSE;
  if(pass==1) {
    currentPeriod=ZX80BITGAP;
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
  pass+=-1;*/
  ZX80ByteWrite();
}

void ZX8081DataBlock() {
  byte r;
  if(currentBit==0) {                         //Check for byte end/first byte
    if(r=ReadByte(bytesRead)==1) {            //Read in a byte
      currentByte = outByte;
    #ifdef ID19REW        
        bytesToRead += -1;
        if((bytesToRead == -1) && (currentID == ID19)) {    
          bytesRead += -1;                      //rewind a byte if we've reached the end
          temppause = PAUSELENGTH;
          currentID = IDPAUSE;
          //return;
        }                   
    #endif 
          
    } else if(r==0) {
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
  //if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) currentPeriod = ZX80TURBOPULSE;
  if (BAUDRATE != 1200) currentPeriod = ZX80TURBOPULSE;
#endif
  if(pass==1) {
    currentPeriod=ZX80BITGAP;
  #ifdef ZX81SPEEDUP
    //if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) currentPeriod = ZX80TURBOBITGAP;
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

//TODO
void writeUEFData() {
  #ifdef DEBUG
  //Serial.println(F("WriteUEFData"));
  #endif
  //Convert byte from file into string of pulses.  One pulse per pass
  byte r;
  if(currentBit==0) {                         //Check for byte end/first byte
    #ifdef DEBUG
    //Serial.println(F("currentBit==0"));
    #endif
    
    if(r=ReadByte(bytesRead)==1) {            //Read in a byte
      currentByte = outByte;
          //itoa(currentByte,PlayBytes,16); printtext(PlayBytes,lineaxy);
      bytesToRead += -1;
      bitChecksum = 0; 
      #ifdef DEBUG
      //Serial.print(F("  bytesToRead after decrement: "));
      //Serial.println(bytesToRead,DEC);
      #endif
      //blkchksum = blkchksum ^ currentByte;    // keep calculating checksum
      if(bytesToRead == 0) {                  //Check for end of data block
        lastByte = 1;
        //Serial.println(F("  Rewind bytesRead"));
        //bytesRead += -1;                      //rewind a byte if we've reached the end
        if(pauseLength==0) {                  //Search for next ID if there is no pause
          currentTask = PROCESSCHUNKID;
        } else {
          currentBlockTask = PAUSE;           //Otherwise start the pause
        }
        //return;                               // exit
      }
    } else if(r==0) {                         // If we reached the EOF
      currentTask = GETCHUNKID;
    }

    currentBit = 11;
    pass=0;
  }
  if ((currentBit == 2) && (parity == 0))currentBit = 1; // parity N
  if (currentBit == 11) {
    currentPeriod = zeroPulse;
  } else if (currentBit == 2) {
    //itoa(bitChecksum,PlayBytes,16);printtext(PlayBytes,lineaxy);
    currentPeriod = (bitChecksum ^ (parity & 0x01)) ? onePulse : zeroPulse; 
    //currentPeriod =  bitChecksum ? onePulse : zeroPulse;  
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

#endif

void writeData() {
  //Convert byte from file into string of pulses.  One pulse per pass
  byte r;
  if(currentBit==0) {                         //Check for byte end/first byte
    if(r=ReadByte(bytesRead)==1) {            //Read in a byte
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
    } else if(r==0) {                         // If we reached the EOF
        if (AYPASS!=4) {                   // Check if need to send checksum
          EndOfFile=true;
          if(pauseLength==0) {
            currentTask = GETID;
          } else {
            currentBlockTask = PAUSE;
          }
          return;                           // return here if normal TAP or TZX
        } 
        else {
          #ifdef AYPLAY  
            currentByte = blkchksum;            // else send calculated chksum
            bytesToRead += 1;                   // add one byte to read
            AYPASS = 0;                         // Reset flag to end block
          #endif
        }
      //return;
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
    //bitWrite(currentPeriod,13,currentByte&0x80);
    if(currentByte&0x80) bitSet(currentPeriod, 13);
    pass+=2;
  }
  else {
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

    byte r;
    if(currentBit==0) {                         //Check for byte end/first byte
        if(r=ReadByte(bytesRead)==1) {            //Read in a byte
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

void wave2() {
  //ISR Output routine
  //unsigned long fudgeTime = micros();         //fudgeTime is used to reduce length of the next period by the time taken to process the ISR
  //word workingPeriod = wbuffer[pos][workingBuffer];
  //word workingPeriod = 256 * wbuffer[pos][workingBuffer] | wbuffer[pos+1][workingBuffer];
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
        //pinState = !pinState;
        pauseFlipBit = true;
        wasPauseBlock = true;
      } else {
        /*
        if(workingPeriod >= 1 && wasPauseBlock==false) {
          //pinState = !pinState;
        } else if (wasPauseBlock==true && isPauseBlock==false) {
          wasPauseBlock=false;
        }
        */
            if (wasPauseBlock==true && isPauseBlock==false) wasPauseBlock=false;        
      }
      #ifdef DIRECT_RECORDING
      if (bitRead(workingPeriod, 14)== 0) {
      #endif
        //digitalWrite(outputPin, pinState);
        pinState = !pinState;
        if (pinState == LOW)     WRITE_LOW;    
        else  WRITE_HIGH;
      #ifdef DIRECT_RECORDING
      } else {
        if (bitRead(workingPeriod, 13) == 0)     WRITE_LOW;    
        else  {WRITE_HIGH; bitClear(workingPeriod,13);}     
        bitClear(workingPeriod,14);         //Clear ID15 flag
        workingPeriod = SampleLength;
        //workingPeriod += 5;              
      }
      #endif
      if(pauseFlipBit==true) {
        newTime = 1500;                     //Set 1.5ms initial pause block
        
      /*  #ifdef rpolarity
          pinState = LOW;                     //Set next pinstate LOW
        #endif
        #ifndef rpolarity
          pinState = HIGH;                     //Set next pinstate HIGH
        #endif */

 //       if (TSXCONTROLzxpolarityUEFSWITCHPARITY) pinState = LOW;         //Set next pinstate LOW
 //       else pinState = HIGH;                     //Set next pinstate HIGH

        pinState = !TSXCONTROLzxpolarityUEFSWITCHPARITY;
       
        //wbuffer[pos][workingBuffer] = highByte(workingPeriod - 1);
        //wbuffer[pos+1][workingBuffer] = lowByte(workingPeriod - 1);
        //wbuffer[pos][workingBuffer] = workingPeriod - 1;  //reduce pause by 1ms as we've already pause for 1.5ms
        wbuffer[pos][workingBuffer] = (workingPeriod - 1) /256;  //reduce pause by 1ms as we've already pause for 1.5ms
        wbuffer[pos+1][workingBuffer] = (workingPeriod - 1) %256;  //reduce pause by 1ms as we've already pause for 1.5ms                 
        pauseFlipBit=false;
      } else {
        if(isPauseBlock==true) {
          newTime = long(workingPeriod)*1000; //Set pause length in microseconds
          //newTime = long(workingPeriod)*1;
          isPauseBlock = false;
        } else {
          newTime = workingPeriod;          //After all that, if it's not a pause block set the pulse period 
        }
        //pos += 1;
        pos += 2;
        if(pos > buffsize)                  //Swap buffer pages if we've reached the end
        {
          pos = 0;
          workingBuffer^=1;
          morebuff = HIGH;                  //Request more data to fill inactive page
        } 
     }
  //} else if(workingPeriod <= 1 && isStopped==0) {
  } else if (isStopped==0) {  
    newTime = 1000;                         //Just in case we have a 0 in the buffer
    //pos += 1;
    pos += 2;
    if(pos > buffsize) {
      pos = 0;
      workingBuffer ^= 1;
      morebuff = HIGH;
    }
  } else {
    newTime = 1000000;                         //Just in case we have a 0 in the buffer    
    //newTime = 100000;                         //Just in case we have a 0 in the buffer
  }
  //newTime += 12;
  //fudgeTime = micros() - fudgeTime;         //Compensate for stupidly long ISR
  //Timer1.setPeriod(newTime - fudgeTime);    //Finally set the next pulse length
  Timer1.setPeriod(newTime +4);    //Finally set the next pulse length  
}



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
      //currentByte = TAPHdr[hdrptr];
       currentByte = pgm_read_byte(TAPHdr+hdrptr);     
      if(hdrptr==13){                           // insert calculated block length minus LEN bytes
            currentByte = lowByte(ayblklen-3);
      } else if(hdrptr==14){
            currentByte = highByte(ayblklen);
      }
      blkchksum = blkchksum ^ currentByte;    // Keep track of Chksum
    //}    
    //if(hdrptr<20) {               //If we're not reading the last byte play all 8 bits
    //if(bytesToRead!=1) {                      //If we're not reading the last byte play all 8 bits
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

void clearBuffer2()
{
  
  for(int i=0;i<buffsize+1;i++)
  {
    wbuffer[i][0]=0;
    wbuffer[i][1]=0;
  } 
}

/*
void UniSetup()
{
  clearBuffer();
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);
  isStopped=true;
  pinState=LOW;
}
*/
void UniSetup() {
    //pinMode(outputPin, OUTPUT);               //Set output pin
    INIT_OUTPORT;
 
    //digitalWrite(outputPin, LOW);             //Start output LOW
    WRITE_LOW;    
    isStopped=true;
    pinState=LOW;
 //   Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
 //   Timer1.attachInterrupt(wave2);
 //   Timer1.stop();                            //Stop the timer until we're ready
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
//  scale=BAUDRATE/1200;
//  period=208/scale;
  //Timer1.setPeriod(period);
  Timer1.stop();
}


void uniLoop() {

/*
 if (casduino) {                 //Check for CAS File.  As these have no header we can skip straight to playing data
    casduinoLoop();
 }  else {
    TZXLoop();
 }  
*/
 if (!casduino) {                 
    TZXLoop();
 }  

 #ifdef Use_CAS
 else {                      //Check for CAS File.  As these have no header we can skip straight to playing data
    casduinoLoop();
 }  
 #endif
}


int ReadByte(unsigned long pos) {
  //Read a byte from the file, and move file position on one if successful
  byte out[1];
  int i=0;

  if(entry.seekSet(pos)) {
    i = entry.read(out,1);
    if(i==1) bytesRead += 1;
  }
  outByte = out[0];
  //blkchksum = blkchksum ^ out[0];
  return i;
}

int ReadWord(unsigned long pos) {
  //Read 2 bytes from the file, and move file position on two if successful
  byte out[2];
  int i=0;

  if(entry.seekSet(pos)) {
    i = entry.read(out,2);
    if(i==2) bytesRead += 2;
  }
  outWord = word(out[1],out[0]);
  //blkchksum = blkchksum ^ out[0] ^ out[1];
  return i;
}

int ReadLong(unsigned long pos) {
  //Read 3 bytes from the file, and move file position on three if successful
  byte out[3];
  int i=0;
  if(entry.seekSet(pos)) {
    i = entry.read(out,3);
    if(i==3) bytesRead += 3;
  }
  outLong = ((unsigned long) word(out[2],out[1]) << 8) | out[0];
  //blkchksum = blkchksum ^ out[0] ^ out[1] ^ out[2];
  return i;
}

int ReadDword(unsigned long pos) {
  //Read 4 bytes from the file, and move file position on four if successful  
  byte out[4];
  int i=0;
  
  if(entry.seekSet(pos)) {
    i = entry.read(out,4);
    if(i==4) bytesRead += 4;
  }
  outLong = ((unsigned long)word(out[3],out[2]) << 16) | word(out[1],out[0]);
  //blkchksum = blkchksum ^ out[0] ^ out[1] ^ out[2] ^ out[3];
  return i;
}

void ReadTZXHeader() {
  //Read and check first 10 bytes for a TZX header
  char tzxHeader[11];
  int i=0;
  
  if(entry.seekSet(0)) {
    i = entry.read(tzxHeader,10);
    if(memcmp_P(tzxHeader,TZXTape,7)!=0) {
      printtextF(PSTR("Not TZXTape"),0);
      delay(300);     
      TZXStop();
    }
  } else {
    printtextF(PSTR("Error Reading File"),0);  
    delay(300);      
  }
  bytesRead = 10;
}

void ReadAYHeader() {
  //Read and check first 8 bytes for a TZX header
  char ayHeader[9];
  int i=0;
  
  if(entry.seekSet(0)) {
    i = entry.read(ayHeader,8);
    if(memcmp_P(ayHeader,AYFile,8)!=0) {
      printtextF(PSTR("Not AY File"),0);
      delay(300);    
      TZXStop();
    }
  } else {
    printtextF(PSTR("Error Reading File"),0);
    delay(300);    
  }
  bytesRead = 0;
}

#ifdef Use_UEF

void ReadUEFHeader() {
  //Read and check first 12 bytes for a UEF header
  char uefHeader[9];
  int i=0;
  
  if(entry.seekSet(0)) {
    i = entry.read(uefHeader,9);
    if(memcmp_P(uefHeader,UEFFile,9)!=0) {
      printtextF(PSTR("Not UEF File"),1);
      TZXStop();
    }
  } else {
    printtextF(PSTR("Error Reading File"),0);
  }
  bytesRead =12;
}

#endif


