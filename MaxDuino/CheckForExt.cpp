#include "configs.h"
#include "constants.h"
#include "compat.h"
#include "processing_state.h"
#include "file_utils.h"
#include "ayplay.h"
#include "casProcessing.h"
#include "MaxProcessing.h"

void checkForEXT(const char * const filenameExt) {
  //Check for .xxx file extension as these have no header

#ifdef Use_CAS
  casduino = CASDUINO_FILETYPE::NONE;
#endif

  if (!strcasecmp_P(filenameExt, PSTR("tap"))) {
    currentTask=TASK::PROCESSID;
    currentID=BLOCKID::TAP;
    readfile(1,bytesRead);
    if (filebuffer[0] == 0x1A) {
      currentID=BLOCKID::JTAP;    
    }   
    #ifdef tapORIC
      //readfile(1,bytesRead);
      if (filebuffer[0] == 0x16) {
        currentID=BLOCKID::ORIC;
      }
    #endif
  }
  else if (!strcasecmp_P(filenameExt, PSTR("p"))) {
    currentTask=TASK::PROCESSID;
    currentID=BLOCKID::ZXP;
  }
  else if (!strcasecmp_P(filenameExt, PSTR("o"))) {
    currentTask=TASK::PROCESSID;
    currentID=BLOCKID::ZXO;
  }
#ifdef AYPLAY  
  else if (!strcasecmp_P(filenameExt, PSTR("ay"))) {
    currentTask=TASK::GETAYHEADER;
    currentID=BLOCKID::AYO;
    AYPASS_hdrptr = AYPASS_STEP::HDRSTART;
  }
#endif
#ifdef Use_UEF
  else if (!strcasecmp_P(filenameExt, PSTR("uef"))) {
    currentTask=TASK::GETUEFHEADER;
    currentID=BLOCKID::UEF;
  }
#endif
#ifdef Use_CAS
  else if (!strcasecmp_P(filenameExt, PSTR("cas"))) {
    casduino = CASDUINO_FILETYPE::CASDUINO;
    invert=false;
    #if defined(Use_DRAGON)
      readfile(1,bytesRead);
      if (filebuffer[0] == 0x55) {
        invert=true;
        casduino = CASDUINO_FILETYPE::DRAGONMODE;
        period=249;
        count_r=255;
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
