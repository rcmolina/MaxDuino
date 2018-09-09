
void checkForEXT (char *filename) {

/*
    Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();                            //Stop the timer until we're ready    
*/    
  //if(checkForTap(filename)) {                 //Check for Tap File.  As these have no header we can skip straight to playing data
  if(checkFor(PSTR(".tap"),filename)) {                 //Check for Tap File.  As these have no header we can skip straight to playing data
    casduino =0;
 /*   Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();         */                   //Stop the timer until we're ready    
    currentTask=PROCESSID;
    currentID=TAP;
    //printtextF(PSTR("TAP Playing"),0);
  }
  //if(checkForP(filename)) {                 //Check for P File.  As these have no header we can skip straight to playing data
  if(checkFor(PSTR(".p"),filename)) {                 //Check for P File.  As these have no header we can skip straight to playing data
    casduino=0;
/*    Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();          */                  //Stop the timer until we're ready    
    currentTask=PROCESSID;
    currentID=ZXP;
    //printtextF(PSTR("ZX81 P Playing"),0);
  }
  //if(checkForO(filename)) {                 //Check for O File.  As these have no header we can skip straight to playing data
  if(checkFor(PSTR(".o"),filename)) {                 //Check for O File.  As these have no header we can skip straight to playing data    
    casduino =0;
 /*   Timer1.initialize(100000);                //100ms pause prevents anything bad happening before we're ready
    Timer1.attachInterrupt(wave2);
    Timer1.stop();      */                      //Stop the timer until we're ready    
    currentTask=PROCESSID;
    currentID=ZXO;
    //printtextF(PSTR("ZX80 O Playing"),0);
  }
  
#ifdef AYPLAY
  //if(checkForAY(filename)) {                 //Check for AY File.  As these have no TAP header we must create it and send AY DATA Block after
  if(checkFor(PSTR(".ay"),filename)) {                 //Check for AY File.  As these have no TAP header we must create it and send AY DATA Block after    
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
#endif
  
#ifdef Use_UEF
  //if(checkForUEF(filename)) {                 //Check for UEF File. 
  if(checkFor(PSTR(".uef"),filename)) {                 //Check for UEF File.     
    currentTask=GETUEFHEADER;
    currentID=UEF;
    //Serial.println(F("UEF playing"));
    //printtextF(PSTR("UEF Playing"),0);
  }
#endif 

//#ifdef Use_CAS 
  //if(checkForCAS(filename)) {                 //Check for CAS File.  As these have no header we can skip straight to playing data
  if(checkFor(PSTR(".cas"),filename)) {                 //Check for CAS File.  As these have no header we can skip straight to playing data    
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
//#endif     
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

bool checkFor(const char* ext, char *filename) {
  //Check for .xxx file extension as these have no header
  char x =0;
  while (*(filename+x) && (*(filename+x) != '.')) {
    x++;
  }

  if(strstr_P(strlwr(filename + x), ext)) {
    return true;
  }
  return false;


/*
  byte extbegin =  strlen(filename)- strlen_P(ext);  
  if(strstr_P(strlwr(filename + extbegin), ext)) {
    return true;
  }
  return false;
*/
/*
  char x =0;
  while (*(filename+x) && (*(filename+x) != '.')) {
    x++;
  }
  boolean goflag = true;
  while (char ch=pgm_read_byte(ext) && goflag) {
    if (*(filename+x) != ch) goflag = false;
    ext++;
    x++;
  }
  return goflag;
*/
}

//#ifdef Use_CAS
bool checkForCAS(char *filename) {
  //Check for CAS file extensions as these have no header
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-4)), PSTR(".cas"))) {
    return true;
  }
  return false;
}
//#endif

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

//#ifdef AYPLAY
bool checkForAY(char *filename) {
  //Check for AY file extensions as these have no header
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-3)), PSTR(".ay"))) {
    return true;
  }
  return false;
}
//#endif

//#ifdef Use_UEF
bool checkForUEF(char *filename) {
  //Serial.println(F("checkForUEF"));
  byte len = strlen(filename);
  if(strstr_P(strlwr(filename + (len-4)), PSTR(".uef"))) {
    return true;
  }
  return false;
}
//#endif


