#ifdef Use_CAS
void casPause()
{
  noInterrupts();
  isStopped=pauseOn;
  interrupts();
}

void casStop()
{
  #if defined(__AVR__)
    Timer1.stop();
  #elif defined(__arm__) && defined(__STM32F1__)
    timer.pause();
  #endif
  //noInterrupts();
  isStopped=true;
  start=0;
  //interrupts();
  entry.close();
  //REWIND=1;   
  seekFile(currentFile);
  bytesRead=0;
  dragonMode=0;
  casduino=0;
/*  Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
  Timer1.attachInterrupt(wave2);
  Timer1.stop();     */                       //Stop the timer until we're ready   
}
#endif


#ifdef Use_CAS
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
      //WRITE_LOW;
      if (out == LOW)     WRITE_LOW;    
      else  WRITE_HIGH;
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
#endif


#ifdef Use_CAS
void writeByte(byte b)
{
#if defined(Use_CAS) && defined(Use_DRAGON)
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
#endif
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
#if defined(Use_CAS) && defined(Use_DRAGON)
  }
#endif
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
#endif


#ifdef Use_CAS
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

#if defined(Use_CAS) && defined(Use_DRAGON)
void processDragon()
{
  lastByte=input[0];
  byte r=0;
  if((r=readfile(1,bytesRead))==1) {
    
#if defined(Use_CAS) && defined(Use_DRAGON) && defined(Use_Dragon_sLeader)
     
    if(currentTask==lookHeader) {      
      if(input[0] == 0x55) {
       writeByte(0x55); 
       bytesRead+=1;
       count--;
      } else {
       currentTask=wHeader; 
      }
        
    } else if(currentTask==wHeader) {      
        if(count>=0) {
          writeByte(0x55);
          count--;
        } else {
          //count= 119;
          count = 2;      
          currentTask=wSync;
        }

    } else if(currentTask==wSync) { 
      
          if(!count==0) {
            writeByte(input[0]);
            bytesRead+=1;
            count--;
          } else {
            writeByte(input[0]);            //Si no cierras el FileNmae block con el primer 0x55 se desincroniza
            bytesRead+=1;
            currentTask=wNameFileBlk;
            count=input[0]++;                   
            //currentTask==lookLeader;             
            //count=255;                 
          }
 
 
    } else if(currentTask==wNameFileBlk) { 
          if(!count==0) {
            writeByte(input[0]);
            bytesRead+=1;
            count--;
          } else {
            writeByte(input[0]);            //Si no cierras el FileNmae block con el primer 0x55 se desincroniza
            bytesRead+=1;            
            currentTask=lookLeader;
            count=255;                 
          }

          
    } else if(currentTask==lookLeader) { 
          if(input[0] == 0x55) {
            writeByte(0x55); 
            bytesRead+=1;
            count--;
          } else {
            currentTask=wNewLeader; 
          }

    } else if(currentTask==wNewLeader) {      
        if(count>=0) {
          writeByte(0x55);
          count--;
        } else {   
          currentTask=wData;
        }
                  
    } else {

#endif
          currentTask=wData;
          writeByte(input[0]);
          bytesRead+=1; 
#if defined(Use_CAS) && defined(Use_DRAGON) && defined(Use_Dragon_sLeader)                       
    }
#endif             
    } else {
      if(currentTask==wData) {
       if(lastByte != 0x55) {
          writeByte(0x55);
          //lcd.print(lastByte);
          //Serial.println(lastByte);
          //printtext(lastByte,0);
          //delay(200);
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
#endif

#endif 
int readfile(byte bytes, unsigned long p)
{
  
  int i=0;
  int t=0;
  if(entry.seekSet(p)) {
    i=entry.read(input,bytes);
  } 
  return i;
}

//#ifdef Use_CAS
void clearBuffer()
{
  for(int i=0;i<buffsize+1;i++)
  {
    wbuffer[i][0]=2;
    wbuffer[i][1]=2;
  }
}
//#endif


#ifdef Use_CAS
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
#if defined(Use_CAS) && defined(Use_DRAGON)
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
#endif
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
#if defined(Use_CAS) && defined(Use_DRAGON)
    }
#endif
    } else {
         //lcdSpinner();
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
#endif


