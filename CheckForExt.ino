
void checkForEXT(const char * const filenameExt) {
  //Check for .xxx file extension as these have no header

#ifdef Use_CAS
  casduino = CASDUINO_FILETYPE::NONE;
#endif

  if (!strcasecmp_P(filenameExt, PSTR("tap"))) {
    currentTask=PROCESSID;
    currentID=TAP;
    #ifdef tapORIC
      readfile(1,bytesRead);
      if (filebuffer[0] == 0x16) {
        currentID=ORIC;
      }
    #endif
  }
  else if (!strcasecmp_P(filenameExt, PSTR("p"))) {
    currentTask=PROCESSID;
    currentID=ZXP;
  }
  else if (!strcasecmp_P(filenameExt, PSTR("o"))) {
    currentTask=PROCESSID;
    currentID=ZXO;
  }
#ifdef AYPLAY  
  else if (!strcasecmp_P(filenameExt, PSTR("ay"))) {
    currentTask=GETAYHEADER;
    currentID=AYO;
    AYPASS = 0;
    hdrptr = HDRSTART;
  }
#endif
#ifdef Use_UEF
  else if (!strcasecmp_P(filenameExt, PSTR("uef"))) {
    currentTask=GETUEFHEADER;
    currentID=UEF;
  }
#endif
#ifdef Use_CAS
  else if (!strcasecmp_P(filenameExt, PSTR("cas"))) {
    casduino = CASDUINO_FILETYPE::CASDUINO;
    out=LOW;
    #if defined(Use_DRAGON)
      readfile(1,bytesRead);
      if (filebuffer[0] == 0x55) {
        out=HIGH;
        casduino = CASDUINO_FILETYPE::DRAGONMODE;
        period=249;
        count=255;
      }
    #endif         
  }
#endif
#ifdef ID11CDTspeedup  
  else if (!strcasecmp_P(filenameExt, PSTR("cdt"))) {
    AMScdt = true;
  }
#endif  
}
