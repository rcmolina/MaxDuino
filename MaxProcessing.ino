
void casPause()
{
  noInterrupts();
  isStopped=pauseOn;
  interrupts();
}

void casStop()
{
  Timer1.stop();
  //noInterrupts();
  isStopped=true;
  start=0;
  //interrupts();
  entry.close();
  UP=1;   
  seekFile(currentFile);
  bytesRead=0;
  dragonMode=0;
  casduino=0;
/*  Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
  Timer1.attachInterrupt(wave2);
  Timer1.stop();     */                       //Stop the timer until we're ready   
}

word TickToUs(word ticks) {
  return (word) ((((float) ticks)/3.5)+0.5);
}

void checkForEXT (char *filename) {

/*
    Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();                            //Stop the timer until we're ready    
*/    
  if(checkForTap(filename)) {                 //Check for Tap File.  As these have no header we can skip straight to playing data
    casduino =0;
 /*   Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();         */                   //Stop the timer until we're ready    
    currentTask=PROCESSID;
    currentID=TAP;
    //printtextF(PSTR("TAP Playing"),0);
  }
  if(checkForP(filename)) {                 //Check for P File.  As these have no header we can skip straight to playing data
    casduino=0;
/*    Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();          */                  //Stop the timer until we're ready    
    currentTask=PROCESSID;
    currentID=ZXP;
    //printtextF(PSTR("ZX81 P Playing"),0);
  }
  if(checkForO(filename)) {                 //Check for O File.  As these have no header we can skip straight to playing data
    casduino =0;
 /*   Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();      */                      //Stop the timer until we're ready    
    currentTask=PROCESSID;
    currentID=ZXO;
    //printtextF(PSTR("ZX80 O Playing"),0);
  }
  if(checkForAY(filename)) {                 //Check for AY File.  As these have no TAP header we must create it and send AY DATA Block after
    casduino =0;    
/*    Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();          */                  //Stop the timer until we're ready    
    currentTask=GETAYHEADER;
    currentID=AYO;
    AYPASS = 0;                             // Reset AY PASS flags
    hdrptr = HDRSTART;                      // Start reading from position 1 -> 0x13 [0x00]
    //printtextF(PSTR("AY Playing"),0);
  }
  if(checkForCAS(filename)) {                 //Check for CAS File.  As these have no header we can skip straight to playing data
    //printtextF(PSTR("CAS Playing"),0);
    casduino = 1;
/*
    Timer1.initialize(period);
    Timer1.attachInterrupt(wave);
    Timer1.stop();
*/
    byte r=0;
    out=LOW;
    if((r=readfile(8,bytesRead))==8) {
      if(!memcmp_P(input,DRAGON,8)) {
        out=HIGH;
        dragonMode=1;
        period=249;
/*
        Timer1.setPeriod(period);
        Timer1.stop();
*/
      }
    }
    bytesRead=0;
    currentType=typeNothing;
    currentTask=lookHeader;
    fileStage=0;
    //noInterrupts();
    clearBuffer();
    isStopped=false;
    //interrupts();
/*
    Timer1.restart();
*/
  }
    
}

/*
void checkForEXT (char *filename) {
  if(checkForTap(filename)) {                 //Check for Tap File.  As these have no header we can skip straight to playing data
    currentTask=PROCESSID;
    currentID=TAP;
    //printtextF(PSTR("TAP Playing"),0);
  }
  if(checkForP(filename)) {                 //Check for P File.  As these have no header we can skip straight to playing data
    currentTask=PROCESSID;
    currentID=ZXP;
    //printtextF(PSTR("ZX81 P Playing"),0);
  }
  if(checkForO(filename)) {                 //Check for O File.  As these have no header we can skip straight to playing data
    currentTask=PROCESSID;
    currentID=ZXO;
    //printtextF(PSTR("ZX80 O Playing"),0);
  }
  if(checkForAY(filename)) {                 //Check for AY File.  As these have no TAP header we must create it and send AY DATA Block after
    currentTask=GETAYHEADER;
    currentID=AYO;
    AYPASS = 0;                             // Reset AY PASS flags
    hdrptr = HDRSTART;                      // Start reading from position 1 -> 0x13 [0x00]
    //printtextF(PSTR("AY Playing"),0);
  }  
}
*/


void UniPlay(char *filename){
  Timer1.stop();                              //Stop timer interrupt
  if(!entry.open(filename,O_READ)) {
    printtextF(PSTR("Error Opening File"),0);
  }
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
   // pinState=LOW;                               //Always Start on a LOW output for simplicity
   // digitalWrite(outputPin, pinState);
//    digitalWrite(outputPin, LOW);
    WRITE_LOW;    
    Timer1.initialize(1000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);

//    Timer1.setPeriod(1000);                     //set 1ms wait at start of a file.
  }
  else {
    Timer1.initialize(period);
    Timer1.attachInterrupt(wave);    
  }
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

bool checkForCAS(char *filename) {
  //Check for CAS file extensions as these have no header
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-4)), PSTR(".cas"))) {
    return true;
  }
  return false;
}


bool checkForTap(char *filename) {
  //Check for TAP file extensions as these have no header
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-4)), PSTR(".tap"))) {
    return true;
  }
  return false;
}

bool checkForP(char *filename) {
  //Check for TAP file extensions as these have no header
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-2)), PSTR(".p"))) {
    return true;
  }
  return false;
}

bool checkForO(char *filename) {
  //Check for TAP file extensions as these have no header
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-2)), PSTR(".o"))) {
    return true;
  }
  return false;
}

bool checkForAY(char *filename) {
  //Check for AY file extensions as these have no header
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-3)), PSTR(".ay"))) {
    return true;
  }
  return false;
}


void TZXStop() {
  Timer1.stop();                              //Stop timer
  isStopped=true;
  start=0;
  entry.close();                              //Close file                                                                                // DEBUGGING Stuff
  //lcd.setCursor(0,1);
  //lcd.print(blkchksum,HEX); lcd.print(F("ck ")); lcd.print(bytesRead); lcd.print(F(" ")); lcd.print(ayblklen);
  UP=1;   
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

void lcdSpinner() {
    if (millis() - timeDiff2 > 1000) {   // check switch every 50ms 
        timeDiff2 = millis();           // get current millisecond count
        #ifdef LCDSCREEN16x2          
          lcd.setCursor(15,0);
          lcd.print(indicators[spinpos++]);
        #endif
        #ifdef OLED1306
            //sendCharXY(indicators[spinpos++],15,0);
           setXY(15,0);sendChar(indicators[spinpos++]);                 
        #endif
        #ifdef P8544
          lcd.setCursor(8,3);
          lcd.print(indicators[spinpos++]);
        #endif                
        if (spinpos > 3) {   
          spinpos = 0;
        }    
    } 
}

void lcdTime() {
    if (millis() - timeDiff2 > 1000) {   // check switch every second 
        timeDiff2 = millis();           // get current millisecond count

  /*       switch(lcdsegs){
            case 0:
              offset=2;
              break;
            case 10:
              offset=1;
              break;
            case 100:
              offset=0;
              break;
            case 1000:
              lcdsegs=0;
              offset=2;
              break;         
          } */
          
        #ifdef LCDSCREEN16x2

            if (lcdsegs % 10 != 0) {itoa(lcdsegs%10,PlayBytes,10);lcd.setCursor(15,0);lcd.print(PlayBytes);} // ultima cifra 1,2,3,4,5,6,7,8,9
            else 
               if (lcdsegs % CNTRBASE != 0){itoa(lcdsegs%CNTRBASE,PlayBytes,10);lcd.setCursor(14,0);lcd.print(PlayBytes);} // es 10,20,30,40,50,60,70,80,90,110,120,..
               else 
                  if (lcdsegs % (CNTRBASE*10) != 0) {itoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);lcd.setCursor(13,0);lcd.print(PlayBytes);} // es 100,200,300,400,500,600,700,800,900,1100,..
                  else {lcd.setCursor(13,0);lcd.print("000");} // es 000,1000,2000,...
/*
          if (lcdsegs % 10 != 0) {lcd.setCursor(15,0);lcd.print(lcdsegs%10);} // ultima cifra 1,2,3,4,5,6,7,8,9
          else 
             if (lcdsegs % 100 != 0){lcd.setCursor(14,0);lcd.print(lcdsegs%100);}  // es 10,20,30,40,50,60,70,80,90,110,120,..
             else 
                if (lcdsegs %1000 != 0) {lcd.setCursor(13,0);lcd.print(lcdsegs%1000);}  // es 100,200,300,400,500,600,700,800,900,1100,..
                else {lcd.setCursor(13,0);lcd.print("000");}   // es 000,1000,2000,...
*/

           lcdsegs++;
          //if (lcdsegs == 1000) lcdsegs = 0; 
          //itoa(lcdsegs++,PlayBytes,10);lcd.setCursor(13,0);lcd.print(PlayBytes);          
    /*      switch(offset){
            case 0:
              break;
            case 1:
              //lcd.print("0");
              break;
            case 2:
              //lcd.print("00");
              break;          
          }  */
          //lcd.print(lcdsegs++);                 
          //sprintf(PlayBytes, "%03d",lcdsegs++);lcd.print(PlayBytes);
           //sprintf(PlayBytes,"Playing% 3d%%  %03d",newpct,lcdsegs++);lcd.setCursor(0,0);lcd.print(PlayBytes);        
        #endif
        #ifdef OLED1306
    
            //sprintf(PlayBytes,"%03d",lcdsegs);
            //sendStrXY(PlayBytes,13,0);

            if (lcdsegs % 10 != 0) {setXY(15,0);sendChar(48+lcdsegs%10);} // ultima cifra 1,2,3,4,5,6,7,8,9
            else 
               if (lcdsegs % CNTRBASE != 0){setXY(14,0);sendChar(48+(lcdsegs%CNTRBASE)/10);sendChar('0');} // es 10,20,30,40,50,60,70,80,90,110,120,..
               else 
                  if (lcdsegs % (CNTRBASE*10) != 0) {setXY(13,0);sendChar(48+(lcdsegs% (CNTRBASE*10))/CNTRBASE);sendChar('0');sendChar('0');} // es 100,200,300,400,500,600,700,800,900,1100,..
                  else {setXY(13,0);sendChar('0');sendChar('0');sendChar('0');} // es 000,1000,2000,...

           lcdsegs++;        
          //if (lcdsegs == 1000) lcdsegs = 0;
                     
 /*         PlayBytes[0]='\0';
          u8g.firstPage();
          do { 
            switch(offset){
              case 0:         
                utoa(lcdsegs,PlayBytes,10);            
                break;
              case 1:
                strcat_P(PlayBytes,PSTR("0"));utoa(lcdsegs,PlayBytes+1,10);            
                break;
              case 2:
                strcat_P(PlayBytes,PSTR("00"));utoa(lcdsegs,PlayBytes+2,10);
                break;          
            }
            //u8g.drawStr( 8*13, 15, PlayBytes);
            u8g.setPrintPos(8*13, 15);
            u8g.print(PlayBytes);    
          } while( u8g.nextPage() );
          lcdsegs++ ;  */
      
          //sprintf(PlayBytes,"Playing %d\% %03d",newpct,lcdsegs++);
  //        sprintf(PlayBytes,"Playing% 3d%%  %03d",newpct,lcdsegs++);
        /*  u8g.firstPage();
          do { 
             u8g.setPrintPos(0,15);
             //u8g.print(PlayBytes);
             u8g.print("Playing "+ String(newpct) + "%" + "  " + String(lcdsegs++));
             u8g.setPrintPos(0,30);
             u8g.print(line1);    
          } while( u8g.nextPage() ); */

   //        printtext(PlayBytes,0);         
        #endif

        #ifdef P8544
        if (lcdsegs % 10 != 0) {itoa(lcdsegs%10,PlayBytes,10);
        lcd.setCursor(13,3);
        lcd.print(PlayBytes);} // ultima cifra 1,2,3,4,5,6,7,8,9
            else 
               if (lcdsegs % CNTRBASE != 0){itoa(lcdsegs%CNTRBASE,PlayBytes,10);
               lcd.setCursor(12,3);
               lcd.print(PlayBytes);} // es 10,20,30,40,50,60,70,80,90,110,120,..
               else 
                  if (lcdsegs % (CNTRBASE*10) != 0) {itoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
                  lcd.setCursor(11,3);
                  lcd.print(PlayBytes);} // es 100,200,300,400,500,600,700,800,900,1100,..
                  else {
                    lcd.setCursor(11,3);
                    lcd.print("000");} // es 000,1000,2000,...
/*
          if (lcdsegs % 10 != 0) {lcd.setCursor(15,0);lcd.print(lcdsegs%10);} // ultima cifra 1,2,3,4,5,6,7,8,9
          else 
             if (lcdsegs % 100 != 0){lcd.setCursor(14,0);lcd.print(lcdsegs%100);}  // es 10,20,30,40,50,60,70,80,90,110,120,..
             else 
                if (lcdsegs %1000 != 0) {lcd.setCursor(13,0);lcd.print(lcdsegs%1000);}  // es 100,200,300,400,500,600,700,800,900,1100,..
                else {lcd.setCursor(13,0);lcd.print("000");}   // es 000,1000,2000,...
*/

           lcdsegs++;
          //if (lcdsegs == 1000) lcdsegs = 0; 
          //itoa(lcdsegs++,PlayBytes,10);lcd.setCursor(13,0);lcd.print(PlayBytes);          
    /*      switch(offset){
            case 0:
              break;
            case 1:
              //lcd.print("0");
              break;
            case 2:
              //lcd.print("00");
              break;          
          }  */
          //lcd.print(lcdsegs++); 
          //sprintf(PlayBytes,"%03d",lcdsegs++); lcd.print(PlayBytes);                     
           //sprintf(PlayBytes,"Playing% 3d%%  %03d",newpct,lcdsegs++);lcd.setCursor(0,0);lcd.print(PlayBytes);        
        #endif
          
    }
}

void lcdPercent() {  
    newpct=(100 * bytesRead)/filesize;                   
    if (currpct ==100){
        currpct= 0;
        #ifdef LCDSCREEN16x2  
          lcd.setCursor(8,0);
          lcd.print(newpct);lcd.print("%");  
          //lcdSpinner(); 
        #endif
        #ifdef OLED1306
          //strcat_P(PlayBytes,PSTR("%"));
          
          if (newpct <10) {setXY(8,0);sendChar(48+newpct%10);}
          else
            if (newpct <100){setXY(8,0);sendChar(48+newpct/10); sendChar(48+newpct%10);}
            else {setXY(8,0);sendChar('1');sendChar('0');sendChar('0');}
            
          sendChar('%');
     
  /*            u8g.firstPage();
          do {  
              //u8g.drawStr( 8*8, 15, PlayBytes);
              u8g.setPrintPos(8*8, 15);
              u8g.print(String(newpct,DEC)+F(" %"));    
          } while( u8g.nextPage() ); */
        #endif
  
        #ifdef P8544
          lcd.setCursor(0,3);
          lcd.print(newpct);
          lcd.print("%");               
        #endif
                             
    }
    if ((newpct >currpct)&& (newpct % 1 == 0)) {
        //PlayBytes[0]='\0'; 
        //strcat_P(PlayBytes,PSTR("Playing "));itoa(newpct,PlayBytes+8,10);strcat_P(PlayBytes,PSTR("%"));
        //printtext(PlayBytes,0);
         #ifdef LCDSCREEN16x2            
            lcd.setCursor(8,0);
            lcd.print(newpct);lcd.print("%"); 
         #endif             
        #ifdef OLED1306
        
          if (newpct <10) {setXY(8,0);sendChar(48+newpct%10);}
          else
            if (newpct <100){setXY(8,0);sendChar(48+newpct/10); sendChar(48+newpct%10);}
            else {setXY(8,0);sendChar('1');sendChar('0');sendChar('0');}
            
          sendChar('%');
  /*           u8g.firstPage();
        do {  
          //u8g.drawStr( 8*8, 15, PlayBytes); 
          u8g.setPrintPos(8*8, 15);
          u8g.print(String(newpct,DEC)+F(" %"));                 
        } while( u8g.nextPage() ); */
        #endif
  
        #ifdef P8544
          lcd.setCursor(0,3);
          lcd.print(newpct);
          lcd.print("%");  
        #endif
        
        currpct = newpct;
        //lcdSpinner();   
    } 
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
    if(currentTask == GETAYHEADER) {
      //grab 8 byte string
      ReadAYHeader();
      //set current task to PROCESSID
      currentTask = PROCESSID;
    }
    if(currentTask == GETID) {
      //grab 1 byte ID
      if(ReadByte(bytesRead)==1) {
        currentID = outByte;
      } else {
        currentID = EOF;
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
              blockOffset[block] = bytesRead;
              blockID[block] = currentID;
              #ifdef OLED1306
                    #ifdef XY
                      setXY(8,2);
                      sendChar('1');sendChar('0');
                      setXY(15,2);
                      sendChar(48+block);   
                    #endif
                    #ifdef XY2
                      setXY(10,1);
                      sendChar(48+block);
                    #endif
              #endif      
              if (block < maxblock-1) block++;               
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
              blockOffset[block] = bytesRead;
              blockID[block] = currentID;
              #ifdef OLED1306
                    #ifdef XY
                      setXY(8,2);
                      sendChar('1');sendChar('1');
                      setXY(15,2);
                      sendChar(48+block);   
                    #endif
                    #ifdef XY2
                      setXY(10,1);
                      sendChar(48+block);
                    #endif
              #endif   
              if (block < maxblock-1) block++;              
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
        /*
        case ID15:
          //process ID15 - Direct Recording
          if(currentBlockTask==READPARAM) {
            if(r=ReadWord(bytesRead)==2) {     
              //Number of T-states per sample (bit of data) 79 or 158 - 22.6757uS for 44.1KHz
              TstatesperSample = TickToUs(outWord); 
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
            }
            currentBlockTask=DATA;
          } else {
            DirectRecording();
          }
        break;
        */

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
              initialpause0=0;
              temppause = outWord;
       //       currentID = IDPAUSE;
            } else {
              initialpause0=1;
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
      //    currentTask = GETID;
            initialpause0 = 1;
            currentID = IDPAUSE;        
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
              blockOffset[block] = bytesRead;
              blockID[block] = currentID;
              #ifdef OLED1306
                    #ifdef XY
                      setXY(8,2);
                      sendChar('4');sendChar('B');
                      setXY(15,2);
                      sendChar(48+block);   
                    #endif
                    #ifdef XY2
                      setXY(10,1);
                      sendChar(48+block);
                    #endif
              #endif     
              if (block < maxblock-1) block++;  
              if(r=ReadDword(bytesRead)==4) {  // Data size to read
                bytesToRead = outLong - 12;
              }
              if(r=ReadWord(bytesRead)==2) {  // Pause after block in ms
                pauseLength = outWord;
              }
              if (TSXspeedup == 0){
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
                  //Fixed speedup baudrate, reduced pilot duration
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
               
              } //TSX_SPEEDUP

              if(r=ReadByte(bytesRead)==1) {  // BitCfg
                oneBitPulses =  outByte & 0x0f;       //(default:4)
                zeroBitPulses = outByte >> 4;         //(default:2)
                if (!oneBitPulses) oneBitPulses = 16;
                if (!zeroBitPulses) zeroBitPulses = 16;
              }
              if(r=ReadByte(bytesRead)==1) {  // ByteCfg
                //Start Bits Cfg
                startBitValue = (outByte >> 5) & 1;   //(default:0)
                startBitPulses = (outByte >> 6) & 3;  //(default:1)
                startBitPulses *= startBitValue ? oneBitPulses : zeroBitPulses;
                //Stop Bits Cfg
                stopBitValue = (outByte >> 2) & 1;    //(default:1)
                stopBitPulses = (outByte >> 3) & 3;   //(default:2)
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
              blockOffset[block] = bytesRead;
              blockID[block] = currentID;
              #ifdef OLED1306
                    #ifdef XY
                      setXY(8,2);
                      sendChar('F');sendChar('E');
                      setXY(15,2);
                      sendChar(48+block);   
                    #endif
                    #ifdef XY2
                      setXY(10,1);
                      sendChar(48+block);
                    #endif
              #endif     
              if (block < maxblock-1) block++;  
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
              pauseLength = PAUSELENGTH*5;
              currentChar=0;
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
              currentBlockTask=DATA;
            break;
            
            case DATA:
              ZX8081DataBlock();
            break; 
            
          }
        break;

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
        
        case IDPAUSE:
         /*     currentPeriod = temppause;
              temppause = 0;
              currentTask = GETID; 
              bitSet(currentPeriod, 15);       */             
          if(temppause>0) {
            if(temppause > 8300) {
              //Serial.println(temppause, DEC);
              currentPeriod = 8300;
              temppause += -8300;         
            } else {
              currentPeriod = temppause;
              temppause = 0;
            }
            bitSet(currentPeriod, 15);
          } else {
              if (initialpause0) {
                if(!count==0) {
                  currentPeriod = 32769;
                  count += -1;
                } else {
                  pauseOn=1;
                  currentTask = GETID;
                  printtextF(PSTR("ID PAUSED"),0);
                  initialpause0=0;
                }
              } else { 
                currentTask = GETID;
                if(EndOfFile==true) currentID=EOF;
              } 
          } 
        break;
    
        case EOF:
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
            lcd.print(String(currentID, HEX));
            lcd.setCursor(0,1);
            lcd.print(String(bytesRead,HEX) + " - L: " + String(loopCount, DEC));
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
              printtextF(PSTR("ID? "),0);
              itoa(currentID,PlayBytes,16);sendStrXY(PlayBytes,4,0);
              itoa(bytesRead,PlayBytes,16);strcat_P(PlayBytes,PSTR(" - L: "));printtext(PlayBytes,lineaxy);
              itoa(loopCount,PlayBytes,10);sendStrXY(PlayBytes,10,lineaxy);

          #endif 
          
          #ifdef P8544             
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("ID? ");
            lcd.setCursor(4,0);
            lcd.print(String(currentID, HEX));
            lcd.setCursor(0,1);
            lcd.print(String(bytesRead,HEX) + " - L: " + String(loopCount, DEC));
          #endif
  
          delay(5000);
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
      if(EndOfFile==true) currentID=EOF;
    break;
  }
}

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
    if (currentBit==10) {
      currentPeriod = startBitValue ? onePulse : zeroPulse;
      pass+=1;
      if (pass==startBitPulses) {
        currentBit += -1;
        pass = 0;
      }
    } else
    //Stop bits
    if (currentBit==1) {
      currentPeriod = stopBitValue ? onePulse : zeroPulse;
      pass+=1;
      if (pass==stopBitPulses) {
        currentBit += -1;
        pass = 0;
      }
    } else
    //Data bits
    {
      dataBit = (currentByte >> (endianness ? (currentBit - 2) : (9 - currentBit))) & 1;
      currentPeriod = dataBit ? onePulse : zeroPulse;
      pass+=1;
      if (pass==(dataBit ? oneBitPulses : zeroBitPulses)) {
        currentBit += -1;
        pass = 0;
      }
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
      currentID=EOF;
      return;
    }
  }
  //End of block?
  if (bytesToRead==0 && currentBit==0) {
    temppause = pauseLength;
    currentBlockTask = PAUSE;
  }
}

void DirectRecording() {
  //Direct Recording - Output bits based on specified sample rate (Ticks per clock) either 44.1KHz or 22.05
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
      bytesToRead += -1;
      /*if(bytesToRead == 0) {                  //Check for end of data block
        bytesRead += -1;                      //rewind a byte if we've reached the end
        if(pauseLength==0) {                  //Search for next ID if there is no pause
          currentTask = GETID;
        } else {
          currentBlockTask = PAUSE;           //Otherwise start the pause
        }
        return;
      }*/
    } else if(r==0) {
      EndOfFile=true;
      temppause = pauseLength;
    currentID = IDPAUSE;
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

void ZX80ByteWrite(){
  currentPeriod = ZX80PULSE;
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
  pass+=-1;
    
}

void writeData() {
  //Convert byte from file into string of pulses.  One pulse per pass
  byte r;
  if(currentBit==0) {                         //Check for byte end/first byte
    if(r=ReadByte(bytesRead)==1) {            //Read in a byte
      currentByte = outByte;
      if (AYPASS==5) {
        currentByte = 0xFF;                 // Only insert first DATA byte if sending AY TAP DATA Block and don't decrement counter
        AYPASS = 4;                         // set Checksum flag to be sent when EOF reached
        bytesRead += -1;                    // rollback ptr and compensate for dummy read byte
        bytesToRead += 2;                   // add 2 bytes to read as we send 0xFF (data flag header byte) and chksum at the end of the block
      } else {
        bytesToRead += -1;  
      }
      blkchksum = blkchksum ^ currentByte;    // keep calculating checksum
      if(bytesToRead == 0) {                  //Check for end of data block
        bytesRead += -1;                      //rewind a byte if we've reached the end
        if(pauseLength==0) {                  //Search for next ID if there is no pause
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
        } else {
          currentByte = blkchksum;            // else send calculated chksum
          bytesToRead += 1;                   // add one byte to read
          AYPASS = 0;                         // Reset flag to end block
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



void wave()
{
  if(isStopped==0)
  {
    switch(wbuffer[pos][working]) {
    case 0:
      if(pass == 0 | pass == 1)
      {
       // digitalWrite(outputPin, out);
          if (out == LOW)     WRITE_LOW;    
          else  WRITE_HIGH;
      } else
      {
       // digitalWrite(outputPin, !out);
          if (out == LOW)     WRITE_HIGH;    
          else  WRITE_LOW; 
      }
      break;

    case 1:
      if(pass==0 | pass==2)
      {
       // digitalWrite(outputPin, out);
          if (out == LOW)     WRITE_LOW;    
          else  WRITE_HIGH;
      } else
      {
       // digitalWrite(outputPin, !out);
          if (out == LOW)     WRITE_HIGH;    
          else  WRITE_LOW;
      }
      if(dragonMode==1 && pass == 1) {
        pass=3;
      }
      break;

    case 2:
    //  digitalWrite(outputPin, LOW);
      WRITE_LOW; 
      break;
    }
  
    pass = pass + 1;
    if(pass == 4) 
    {
      pass=0;
      pos += 1;
      if(pos > buffsize - (dragonBuff * dragonMode)) 
      {
        pos = 0;
        working ^=1;
        morebuff = HIGH;
      }
    }
  } else 
      // digitalWrite(outputPin, LOW);
      WRITE_LOW;

}

void wave2() {
  //ISR Output routine
  //unsigned long fudgeTime = micros();         //fudgeTime is used to reduce length of the next period by the time taken to process the ISR
  //word workingPeriod = wbuffer[pos][workingBuffer];
  word workingPeriod = 256 * wbuffer[pos][workingBuffer] | wbuffer[pos+1][workingBuffer];
  //word workingPeriod = word(wbuffer[pos][workingBuffer], wbuffer[pos+1][workingBuffer]);  
  byte pauseFlipBit = false;
  unsigned long newTime=1;
  intError = false;
  if(isStopped==0 && workingPeriod >= 1)
  {
      if(bitRead(workingPeriod, 15) && !pskipMask)          
      {
        //If bit 15 of the current period is set we're about to run a pause
        //Pauses start with a 1.5ms where the output is untouched after which the output is set LOW
        //Pause block periods are stored in milliseconds not microseconds
        isPauseBlock = true;
        bitClear(workingPeriod,15);         //Clear pause block flag
        pinState = !pinState;
        pauseFlipBit = true;
        wasPauseBlock = true;
      } else {
        if(workingPeriod >= 1 && wasPauseBlock==false) {
          pinState = !pinState;
        } else if (wasPauseBlock==true && isPauseBlock==false) {
          wasPauseBlock=false;
        }
      }
      //digitalWrite(outputPin, pinState);
      if (pinState == LOW)     WRITE_LOW;    
      else  WRITE_HIGH;      
      if(pauseFlipBit==true) {
        newTime = 1500;                     //Set 1.5ms initial pause block
        pinState = LOW;                     //Set next pinstate LOW
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
  } else if(workingPeriod <= 1 && isStopped==0) {
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



void writeByte(byte b)
{
if(dragonMode==1) {
    for(int i=0;i<8;i++)
    {
      if(b&1)
      {
        bits[i]=1;
      } else bits[i]=0;
      b = b >> 1;
    }
    bits[8]=2;
    bits[9]=2;
    bits[10]=2;
  } else {
    bits[0]=0;
    for(int i=1;i<9;i++)
    {
      if(b&1)
      {
        bits[i]=1;
      } else bits[i]=0;
      b = b >> 1;
    }
    bits[9]=1;
    bits[10]=1;
  }
}

void writeSilence()
{
  for(int i=0;i<11;i++)
  {
    bits[i]=2;
  }
}

void writeHeader()
{
  for(int i=0;i<11;i++)
  {
    bits[i]=1;
  }
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

void process()
{
  byte r=0;
  if(currentType==typeEOF)
  {
    if(!count==0) {
      writeSilence();
      count+=-1;  
    } else stopFile();    
    return;
  }
   if(currentTask==lookHeader || currentTask==wData)
   {
     if((r=readfile(8,bytesRead))==8) 
     {
      if(!memcmp_P(input,HEADER,8)) {
        if(fileStage==0) 
        {
          currentTask = lookType;
        } else
        {
          currentTask = wSilence;
          count=SHORT_SILENCE*scale;
        }
        if(currentType==typeNothing) fileStage=1;
        bytesRead+=8;
      }
      
     } else if(r==0) 
     {
      currentType=typeEOF;
      currentTask=wClose;
      count=LONG_SILENCE*scale;
     }
     
   }
   if(currentTask==lookType)
   {
    if((r=readfile(10,bytesRead))==10)
    {
      if(!memcmp_P(input,ASCII,10))
      {
        currentType = typeAscii;
        currentTask = wSilence;
        count = LONG_SILENCE*scale;
        fileStage=1;
      }else if(!memcmp_P(input,BINF,10))
      {
        currentType = typeBinf;
        currentTask  = wSilence;
        count = LONG_SILENCE*scale;
        fileStage=1;        
      }else if(!memcmp_P(input,BASIC,10))
      {
        currentType = typeBasic;
        currentTask = wSilence;
        count = LONG_SILENCE*scale;
        fileStage=1;
      } else 
      {
        currentType = typeUnknown;
        currentTask = wSilence;
        count = LONG_SILENCE*scale;
        fileStage=1;       
      }
    } else {
        currentType = typeUnknown;
        currentTask = wSilence;
        count = LONG_SILENCE*scale;
        fileStage=1;       
    }
   }
   if(currentTask==wSilence)
   {
    if(!count==0)
    {
      writeSilence();
      count+=-1;
    } else 
    {
      currentTask=wHeader;
      if(fileStage==1) 
      {
        //count=LONG_HEADER*scale;
        count=LONG_HEADER; 
        fileStage+=1;
      } else 
      {
        count=SHORT_HEADER*scale;
        //count=SHORT_HEADER;
        if(currentType==typeAscii) {
          fileStage+=1;
        } else {
          fileStage=0;
        }
      }
    }
   }
   if(currentTask==wHeader)
   {
    if(!count==0)
    {
      writeHeader();
      count+=-1;
    } else
    {
      currentTask=wData;
      return;
    }
   }
   if(currentTask==wData)
   {
    writeByte(input[0]);
    if(input[0]==0x1a && currentType==typeAscii) 
    {
      fileStage=0;
    }
   }
   if(currentTask==lookHeader || currentTask==lookType || currentTask==wData) bytesRead+=1; 
   
}

void processDragon()
{
  lastByte=input[0];
  byte r=0;
  if((r=readfile(1,bytesRead))==1) {
    currentTask=wData;
    writeByte(input[0]);
    bytesRead+=1;
  } else {
    if(currentTask==wData) {
       if(lastByte != 0x55) {
      writeByte(0x55);
      //lcd.print(lastByte);
      //Serial.println(lastByte);
 //     printtext(lastByte,0);
 //     delay(200);
    }      
      count = 54;
      currentTask=wSilence;
    }
    if(currentTask==wSilence) {      
      if(!count==0) {
        writeSilence();
        count--;
      } else {
        stopFile();
      }
    }
  }
}

int readfile(byte bytes, unsigned long p)
{
  
  int i=0;
  int t=0;
  if(entry.seekSet(p)) {
    i=entry.read(input,bytes);
  } 
  return i;
}

void clearBuffer()
{
  for(int i=0;i<buffsize+1;i++)
  {
    wbuffer[i][0]=2;
    wbuffer[i][1]=2;
  }
}

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
    DDRB |= _BV(1);                            // El pin9 es el bit1 del PORTB
    //digitalWrite(outputPin, LOW);             //Start output LOW
    WRITE_LOW;    
    isStopped=true;
    pinState=LOW;
    Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();                            //Stop the timer until we're ready
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
  Timer1.setPeriod(period);
  Timer1.stop();
}


void uniLoop() {
 
 if (casduino) {                 //Check for CAS File.  As these have no header we can skip straight to playing data
    casduinoLoop();
 }  else {
    TZXLoop();
 }  
} 

void casduinoLoop()
{
  noInterrupts();
  copybuff = morebuff;
  morebuff = LOW;
  isStopped=pauseOn;
  interrupts();

  if(copybuff==HIGH) {
    btemppos=0;
    copybuff=LOW;
  }
  if(btemppos<=buffsize - (dragonBuff * dragonMode))
  { 
    if(dragonMode==1) {
      processDragon();
//      noInterrupts();
      for(int t=0;t<8;t++)
      {
        if(btemppos<=buffsize)
        {
          wbuffer[btemppos][working ^ 1] = bits[t];
          btemppos+=1;         
        }        
      }
//      interrupts();
    } else {
      process();      
//      noInterrupts();
      for(int t=0;t<11;t++)
      {
        if(btemppos<=buffsize)
        {
          wbuffer[btemppos][working ^ 1] = bits[t];
          btemppos+=1;         
        }        
      }
//      interrupts();
    }
    } else {
         //lcdSpinner();
         if (pauseOn == 0) {
          lcdTime();
          lcdPercent();
         }
    } 
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
  outLong = (word(out[2],out[1]) << 8) | out[0];
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
  outLong = (word(out[3],out[2]) << 16) | word(out[1],out[0]);
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

