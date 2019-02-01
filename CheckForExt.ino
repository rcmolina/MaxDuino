
void checkForEXT (char *filename) {
    //Check for .xxx file extension as these have no header
  char x =0;
  while (*(filename+x) && (*(filename+x) != '.')) x++;
  if(strstr_P(strlwr(filename + x), PSTR(".tap"))) {casduino =0; currentTask=PROCESSID; currentID=TAP;}
  else if(strstr_P(strlwr(filename + x), PSTR(".p"))) {casduino =0; currentTask=PROCESSID; currentID=ZXP;}
  else if(strstr_P(strlwr(filename + x), PSTR(".o"))) {casduino =0; currentTask=PROCESSID; currentID=ZXO;}
 
  else if(strstr_P(strlwr(filename + x), PSTR(".ay"))) {casduino =0; currentTask=GETAYHEADER; currentID=AYO; AYPASS = 0; hdrptr = HDRSTART;}

 #ifdef Use_UEF
  else if(strstr_P(strlwr(filename + x), PSTR(".uef"))) {casduino =0; currentTask=GETUEFHEADER; currentID=UEF;}
 #endif
 //#ifdef Use_CAS
  else if(strstr_P(strlwr(filename + x), PSTR(".cas"))) {
    casduino =1; 
    byte r=0;
    out=LOW;
    if((r=readfile(8,bytesRead))==8) {
      if(!memcmp_P(input,DRAGON,8)) {
        out=HIGH;
        dragonMode=1;
        period=249;
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
  }
// #endif  
}




