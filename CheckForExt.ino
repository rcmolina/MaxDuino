
void checkForEXT (char *filenameExt) {
    //Check for .xxx file extension as these have no header

  //if(strstr_P(strlwr(filenameExt), PSTR(".tap"))) {
  if (!strcasecmp_P(filenameExt, PSTR(".tap"))) {
  //if (strcasestr_P(filename, PSTR(".tap"))) {    
    casduino =0; currentTask=PROCESSID; currentID=TAP;
    #ifdef tapORIC
      readfile(1,bytesRead);
      if (input[0] == 0x16) {currentID=ORIC;}
    #endif
  }
  //else if(strstr_P(strlwr(filenameExt), PSTR(".p"))) {casduino =0; currentTask=PROCESSID; currentID=ZXP;}
  else if (!strcasecmp_P(filenameExt, PSTR(".p"))) {casduino =0; currentTask=PROCESSID; currentID=ZXP;}
  //else if (strcasestr_P(filenameExt, PSTR(".p"))) {casduino =0; currentTask=PROCESSID; currentID=ZXP;}  
  //else if(strstr_P(strlwr(filenameExt), PSTR(".o"))) {casduino =0; currentTask=PROCESSID; currentID=ZXO;}  
  else if (!strcasecmp_P(filenameExt, PSTR(".o"))) {casduino =0; currentTask=PROCESSID; currentID=ZXO;}
  //else if (strcasestr_P(filenameExt, PSTR(".o"))) {casduino =0; currentTask=PROCESSID; currentID=ZXO;}  
 #ifdef AYPLAY  
  //else if(strstr_P(strlwr(filenameExt), PSTR(".ay"))) {casduino =0; currentTask=GETAYHEADER; currentID=AYO; AYPASS = 0; hdrptr = HDRSTART;}  
  else if (!strcasecmp_P(filenameExt, PSTR(".ay"))) {casduino =0; currentTask=GETAYHEADER; currentID=AYO; AYPASS = 0; hdrptr = HDRSTART;}
  //else if (strcasestr_P(filenameExt, PSTR(".ay"))) {casduino =0; currentTask=GETAYHEADER; currentID=AYO; AYPASS = 0; hdrptr = HDRSTART;}
 #endif
 #ifdef Use_UEF
  //else if(strstr_P(strlwr(filenameExt), PSTR(".uef"))) {casduino =0; currentTask=GETUEFHEADER; currentID=UEF;}
  else if (!strcasecmp_P(filenameExt, PSTR(".uef"))) {casduino =0; currentTask=GETUEFHEADER; currentID=UEF;}
  //else if (strcasestr_P(filenameExt, PSTR(".uef"))) {casduino =0; currentTask=GETUEFHEADER; currentID=UEF;}  
 #endif
 #ifdef Use_CAS
  //else if(strstr_P(strlwr(filenameExt), PSTR(".cas"))) {
  else if (!strcasecmp_P(filenameExt, PSTR(".cas"))) {
  //else if (strcasestr_P(filenameExt, PSTR(".cas"))) {    
    casduino =1; out=LOW;dragonMode=0;
    #if defined(Use_CAS) && defined(Use_DRAGON)
      //if(!memcmp_P(input,DRAGON,1)) {
      readfile(1,bytesRead);
      if (input[0] == 0x55) {out=HIGH;dragonMode=1;period=249;count=255;}
    #endif         
  }
 #endif  
}

