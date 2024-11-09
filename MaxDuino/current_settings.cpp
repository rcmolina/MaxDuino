#include "configs.h"
#include "Arduino.h"
#include "preferences.h"
#include "current_settings.h"
#include "casProcessing.h"

word BAUDRATE = DEFAULT_BAUDRATE;
// TODO really the following should only be defined ifndef NO_MOTOR
// but the order of #includes is wrong and we only define NO_MOTOR later :-/
bool mselectMask = DEFAULT_MSELECTMASK;
bool TSXCONTROLzxpolarityUEFSWITCHPARITY = DEFAULT_TSXzxpUEF;
bool skip2A = DEFAULT_SKIP2A;

#ifdef LOAD_EEPROM_SETTINGS
#include "EEPROM_wrappers.h"
void updateEEPROM()
{
    /* Setting Byte: 
    *  bit 0: 1200
    *  bit 1: 2400
    *  bit 2: 3150
    *  bit 3: 3600
    *  bit 4: 3850
    *  bit 5: BLK_2A control
    *  bit 6: TSXCONTROLzxpolarityUEFSWITCHPARITY
    *  bit 7: Motor control
    */
    byte settings=0;

    switch(BAUDRATE) {
      case 1200:
      settings |=1;
      break;
      case 2400:
      settings |=2;
      break;
      case 3150:
      settings |=4;
      break;    
      case 3600:
      settings |=8;  
      break;      
      case 3850:
      settings |=16;
      break;     
    }

    #ifndef NO_MOTOR
      if(mselectMask) settings |=128;
    #endif

    if(TSXCONTROLzxpolarityUEFSWITCHPARITY) settings |=64;
    
    #ifdef MenuBLK2A
      if(skip2A) settings |=32;
    #endif

    EEPROM_put(EEPROM_CONFIG_BYTEPOS, settings);
    setBaud();
}

void loadEEPROM()
{
    byte settings=0;
    EEPROM_get(EEPROM_CONFIG_BYTEPOS, settings);
        
    if(!settings) return;
    
    #ifndef NO_MOTOR
      mselectMask=bitRead(settings,7);
    #endif

    TSXCONTROLzxpolarityUEFSWITCHPARITY=bitRead(settings,6);
    
    #ifdef MenuBLK2A
      skip2A=bitRead(settings,5);
    #endif
    
    if(bitRead(settings,0)) {
      BAUDRATE=1200;
    }
    if(bitRead(settings,1)) {
      BAUDRATE=2400;
    }
    if(bitRead(settings,2)) {
      BAUDRATE=3150;  
    }
    if(bitRead(settings,3)) {
      BAUDRATE=3600;  
    }
    if(bitRead(settings,4)) {
      BAUDRATE=3850;  
    }
}
#endif
