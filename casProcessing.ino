#ifdef Use_CAS

void casPause()
{
  noInterrupts();
  isStopped=pauseOn;
  interrupts();
}

void wave()
{
  if(isStopped==0)
  {
    switch(wbuffer[pos][working]) {
      case 0:
        if(pass == 0 || pass == 1) {
          if (out == LOW) WRITE_LOW;    
          else WRITE_HIGH;
        } else {
          if (out == LOW) WRITE_HIGH;    
          else WRITE_LOW; 
        }
        break;

      case 1:
        if(pass==0 || pass==2) {
          if (out == LOW) WRITE_LOW;    
          else WRITE_HIGH;
        } else {
          if (out == LOW) WRITE_HIGH;    
          else WRITE_LOW;
        }
        #if defined(Use_DRAGON)
        if(casduino == CASDUINO_FILETYPE::DRAGONMODE && pass == 1) {
          pass=3;
        }
        #endif
        break;

      case 2:
        if (out == LOW) WRITE_LOW;
        else WRITE_HIGH;
        break;
    }
  
    pass = pass + 1;
    if(pass == 4) 
    {
      pass=0;
      pos += 1;
      if(pos >= buffsize) 
      {
        pos = 0;
        working ^=1;
        morebuff = true;
      }
    }
  } else {
    WRITE_LOW;
  }
}

void writeByte(byte b)
{
  byte * _pbits = bits;
#if defined(Use_DRAGON)
  if(casduino == CASDUINO_FILETYPE::CASDUINO)
#endif
  {
    *_pbits++ = 0; // 1 start bit
  }

  for(int i=0;i<8;i++)
  {
    *_pbits++ = (b&1);
    b >>= 1;
  }

#if defined(Use_DRAGON)
  if(casduino == CASDUINO_FILETYPE::CASDUINO)
#endif
  {
    // 2 stop bits
    *_pbits++ = 1;
    *_pbits++ = 1;
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

void process()
{
//  byte r=0;
  if(cas_currentType==CAS_TYPE::typeEOF)
  {
//    if(!count==0) {
    if(count) {
      writeSilence();
      count+=-1;  
    } else stopFile();    
    return;
  }
  if(currentTask==TASK::GETFILEHEADER || currentTask==TASK::CAS_wData)
  {
//    if((r=readfile(8,bytesRead))==8)
    if(readfile(8,bytesRead))
    {
      if(!memcmp_P(filebuffer, HEADER,8)) {
        if(fileStage==0) 
        {
          currentTask = TASK::CAS_lookType;
        } else
        {
          currentTask = TASK::CAS_wSilence;
          count=SHORT_SILENCE*scale;
        }
        if(cas_currentType==CAS_TYPE::Nothing) fileStage=1;
        bytesRead+=8;
      }
      
//    } else if(r==0)
    } else
    {
      cas_currentType=CAS_TYPE::typeEOF;
      currentTask=TASK::CAS_wClose;
      count=LONG_SILENCE*scale;
    }
     
  }

  if(currentTask==TASK::CAS_lookType)
  {
    currentTask = TASK::CAS_wSilence;
    count = LONG_SILENCE*scale;
    fileStage=1;       
    cas_currentType = CAS_TYPE::Unknown;
//    if((r=readfile(10,bytesRead))==10)
    if((readfile(10,bytesRead))==10)
    {
      if(!memcmp_P(filebuffer, ASCII, 10))
      {
        cas_currentType = CAS_TYPE::Ascii;
      }else if(!memcmp_P(filebuffer, BINF, 10))
      {
        cas_currentType = CAS_TYPE::Binf;
      }else if(!memcmp_P(filebuffer, BASIC, 10))
      {
        cas_currentType = CAS_TYPE::Basic;
      }
    }
  }

  if(currentTask==TASK::CAS_wSilence)
  {
//    if(!count==0)
    if(count)
    {
      writeSilence();
      count+=-1;
    } else 
    {
      currentTask=TASK::CAS_wHeader;
      if(fileStage==1) 
      {
        //count=LONG_HEADER*scale;
        count=LONG_HEADER; 
        fileStage+=1;
      } else 
      {
        count=SHORT_HEADER*scale;
        //count=SHORT_HEADER;
        if(cas_currentType==CAS_TYPE::Ascii) {
          fileStage+=1;
        } else {
          fileStage=0;
        }
      }
    }
  }
  if(currentTask==TASK::CAS_wHeader)
  {
//    if(!count==0)
    if(count)
    {
      writeHeader();
      count+=-1;
    } else
    {
      currentTask=TASK::CAS_wData;
      return;
    }
  }
  if(currentTask==TASK::CAS_wData)
  {
    writeByte(filebuffer[0]);
    if(filebuffer[0]==0x1a && cas_currentType==CAS_TYPE::Ascii) 
    {
      fileStage=0;
    }
  }
  if(currentTask==TASK::GETFILEHEADER || currentTask==TASK::CAS_lookType || currentTask==TASK::CAS_wData) bytesRead+=1; 
}


#if defined(Use_DRAGON)
void processDragon()
{
  lastByte=filebuffer[0];
//  byte r=0;
//  if((r=readfile(1,bytesRead))==1) {
  if((readfile(1,bytesRead))==1) {

  #if defined(Use_Dragon_sLeader) && not defined(Expand_All)
    if(currentTask==TASK::GETFILEHEADER) {
      if(filebuffer[0] == 0x55) {
       writeByte(0x55); 
       bytesRead+=1;
       count--;
      } else {
       currentTask=TASK::CAS_wHeader;
      }

    } else if(currentTask==TASK::CAS_wHeader) {
        if(count>=0) {
          writeByte(0x55);
          count--;
        } else {    
          if (fileStage > 0) currentTask=TASK::CAS_wData;
          else {
            count =19;
            currentTask=TASK::CAS_wNameFileBlk;
          }
        }
    } else if(currentTask==TASK::CAS_wNameFileBlk) {
//        if(!count==0) {
        if(count) {
            writeByte(filebuffer[0]);
            bytesRead+=1;
            count--;            
        } else {            
            fileStage=1;
            currentTask=TASK::GETFILEHEADER;
            count=255;
        }
    } else {        
  #endif
    
  #if defined(Use_Dragon_sLeader) && defined(Expand_All)

    if(currentTask==TASK::GETFILEHEADER) {
      if(filebuffer[0] == 0x55) {
       writeByte(0x55); 
       bytesRead+=1;
       count--;
      } else {
       currentTask=TASK::CAS_wHeader;
      }

    } else if(currentTask==TASK::CAS_wHeader) {
      if(count>=0) {
        writeByte(0x55);
        count--;
      } else {
        //count= 119;
        count = 2;      
        currentTask=TASK::CAS_wSync;
      }

    } else if(currentTask==TASK::CAS_wSync) { 
//      if(!count==0) {
      if(count) {
        writeByte(filebuffer[0]);
        bytesRead+=1;
        count--;
      } else {
        writeByte(filebuffer[0]);            //Si no cierras el FileNmae block con el primer 0x55 se desincroniza
        bytesRead+=1;
        currentTask=TASK::CAS_wNameFileBlk;
        count=filebuffer[0]++;                   
      }
 
    } else if(currentTask==TASK::CAS_wNameFileBlk) { 
//      if(!count==0) {
      if(count) {
        writeByte(filebuffer[0]);
        bytesRead+=1;
        count--;
      } else {
        writeByte(filebuffer[0]);            //Si no cierras el FileNmae block con el primer 0x55 se desincroniza
        bytesRead+=1;            
        currentTask=TASK::CAS_lookLeader;
        count=255;                 
      }
          
    } else if(currentTask==TASK::CAS_lookLeader) { 
      if(filebuffer[0] == 0x55) {
        writeByte(0x55); 
        bytesRead+=1;
        count--;
      } else {
        currentTask=TASK::CAS_wNewLeader;
      }

    } else if(currentTask==TASK::CAS_wNewLeader) {
      if(count>=0) {
        writeByte(0x55);
        count--;
      } else {   
        currentTask=TASK::CAS_wData;
      }
                  
    } else {

  #endif
      //currentTask=TASK::CAS_wData;
      writeByte(filebuffer[0]);
      bytesRead+=1; 
  //#if defined(Use_DRAGON) && defined(Use_Dragon_sLeader)                       
    }
  //#endif
  
    } else {          // readfile !=1 , ending
      if(currentTask==TASK::CAS_wData) {
       if(lastByte != 0x55) {
          writeByte(0x55);
      }      
      count = 54;
      currentTask=TASK::CAS_wSilence;
    }    
    if(currentTask==TASK::CAS_wSilence) {
//      if(!count==0) {
      if(count) {
        writeSilence();
        count--;
      } else {
        stopFile();
      }
    }
  }
}
#endif

void casduinoLoop()
{
  noInterrupts();
  copybuff = morebuff;
  morebuff = false;
  isStopped=pauseOn;
  interrupts();

  if(copybuff) {
    btemppos=0;
    copybuff=false;
  }

  if(btemppos<buffsize)
  { 
#if defined(Use_DRAGON)
    if(casduino == CASDUINO_FILETYPE::DRAGONMODE) {
      processDragon();
    }
    else
#endif
    {
      process();      
    }

    if(btemppos<buffsize)
    {
      // casduino isn't just true/false - it's the number of bits (8 or 11)
      for(int t=0; t<(byte)casduino; t++)
      {
        wbuffer[btemppos][working ^ 1] = bits[t];
        btemppos+=1;         
      }        
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

#endif
