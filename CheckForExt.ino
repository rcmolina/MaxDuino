
void checkForEXT (char *filename) {
    //Check for .xxx file extension as these have no header
  char x =0,ch1;
  while (*(filename+x) && (*(filename+x) != '.')) x++;
  //if(strstr_P(strlwr(filename + x), PSTR(".tap"))) {
  if (!strcasecmp_P(filename + x, PSTR(".tap"))) {
  //if (strcasestr_P(filename, PSTR(".tap"))) {    
    casduino =0; currentTask=PROCESSID; currentID=TAP;
    #ifdef tapORIC
      entry.read(ch1,bytesRead);
      if (ch1 == 0x16) {currentID=ORIC;}
    #endif
  }
  //else if(strstr_P(strlwr(filename + x), PSTR(".p"))) {casduino =0; currentTask=PROCESSID; currentID=ZXP;}
  else if (!strcasecmp_P(filename + x, PSTR(".p"))) {casduino =0; currentTask=PROCESSID; currentID=ZXP;}
  //else if (strcasestr_P(filename, PSTR(".p"))) {casduino =0; currentTask=PROCESSID; currentID=ZXP;}  
  //else if(strstr_P(strlwr(filename + x), PSTR(".o"))) {casduino =0; currentTask=PROCESSID; currentID=ZXO;}  
  else if (!strcasecmp_P(filename + x, PSTR(".o"))) {casduino =0; currentTask=PROCESSID; currentID=ZXO;}
  //else if (strcasestr_P(filename, PSTR(".o"))) {casduino =0; currentTask=PROCESSID; currentID=ZXO;}  
 #ifdef AYPLAY  
  //else if(strstr_P(strlwr(filename + x), PSTR(".ay"))) {casduino =0; currentTask=GETAYHEADER; currentID=AYO; AYPASS = 0; hdrptr = HDRSTART;}  
  else if (!strcasecmp_P(filename + x, PSTR(".ay"))) {casduino =0; currentTask=GETAYHEADER; currentID=AYO; AYPASS = 0; hdrptr = HDRSTART;}
  //else if (strcasestr_P(filename, PSTR(".ay"))) {casduino =0; currentTask=GETAYHEADER; currentID=AYO; AYPASS = 0; hdrptr = HDRSTART;}
 #endif
 #ifdef Use_UEF
  //else if(strstr_P(strlwr(filename + x), PSTR(".uef"))) {casduino =0; currentTask=GETUEFHEADER; currentID=UEF;}
  else if (!strcasecmp_P(filename + x, PSTR(".uef"))) {casduino =0; currentTask=GETUEFHEADER; currentID=UEF;}
  //else if (strcasestr_P(filename, PSTR(".uef"))) {casduino =0; currentTask=GETUEFHEADER; currentID=UEF;}  
 #endif
 #ifdef Use_CAS
  //else if(strstr_P(strlwr(filename + x), PSTR(".cas"))) {
  else if (!strcasecmp_P(filename + x, PSTR(".cas"))) {
  //else if (strcasestr_P(filename, PSTR(".cas"))) {    
    casduino =1; 
    out=LOW;
    dragonMode=0;
      entry.read(ch1,bytesRead);
      #if defined(Use_CAS) && defined(Use_DRAGON)
        //if(!memcmp_P(input,DRAGON,1)) {
        if (ch1 == 0x55) {
          out=HIGH;
          dragonMode=1;
          period=249;
          count=255;
        }
      #endif

    bytesRead=0;
    currentType=typeNothing;
    currentTask=lookHeader;
    fileStage=0;
    //noInterrupts();
//    clearBuffer();
    isStopped=false;
    //interrupts();            
  }
 #endif  
}

