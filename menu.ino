/*
void menuMode()
 {
      //Return to root of the SD card.
       sd.chdir(true);
       getMaxFile();
       currentFile=1;
       seekFile(currentFile);  
       while(digitalRead(btnMselect)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50);
       }
}
*/
/**************************************************************
 *                  Casduino Menu Code:
 *  Menu Button (was motor controll button) opens menu
 *  up/down move through menu, play select, stop back
 *  Menu Options:
 *  Baud:
 *    1200
 *    2400
 *    3600
 *    3850
 *  
 *  MotorControl:
 *    On
 *    Off
 *  
 *  Save settings to eeprom on exit. 
 */
// byte lastbtn=true;


#if defined(__arm__) && defined(__STM32F1__)

  uint8_t EEPROM_get(uint16_t address, byte *data) {
    if (EEPROM.init()==EEPROM_OK) {
      *data = (byte)(EEPROM.read(address) & 0xff);  
      return true;  
    } else 
      return false;
  }
  
  
  uint8_t EEPROM_put(uint16_t address, byte data) {
    if (EEPROM.init()==EEPROM_OK) {
      EEPROM.write(address, (uint16_t) data); 
      return true;    
    } else
      return false;
  }
  #endif

 void menuMode()
 { 
  byte menuItem=0;
  byte subItem=0;
  byte updateScreen=true;
  
  while(digitalRead(btnStop)==HIGH || lastbtn)
  {
    if(updateScreen) {
      printtextF(PSTR("Menu"),0);
      switch(menuItem) {
        case 0:
        printtextF(PSTR("Baud Rate ?"),lineaxy);
        break;
        case 1:
        printtextF(PSTR("Motor Ctrl ?"),lineaxy);
        break;
        case 2:
        printtextF(PSTR("TSXCzxpUEFSW ?"),lineaxy);
        break;
      #ifndef Use_UEF
        case 3:
        printtextF(PSTR("Skip BLK:2A ?"),lineaxy);
        break;       
      #endif
             
      }
      updateScreen=false;
    }
    if(digitalRead(btnDown)==LOW && !lastbtn){
      #ifndef Use_UEF
      if(menuItem<3) menuItem+=1;
      #endif
      #ifdef Use_UEF
      if(menuItem<2) menuItem+=1;      
      #endif
      
      lastbtn=true;
      updateScreen=true;
    }
    if(digitalRead(btnUp)==LOW && !lastbtn) {
      if(menuItem>0) menuItem+=-1;
      lastbtn=true;
      updateScreen=true;
    }
    if(digitalRead(btnPlay)==LOW && !lastbtn) {
      switch(menuItem){
        case 0:
          subItem=0;
          updateScreen=true;
          lastbtn=true;
          while(digitalRead(btnStop)==HIGH || lastbtn) {
            if(updateScreen) {
              printtextF(PSTR("Baud Rate"),0);
              switch(subItem) {
                case 0:                                  
                  printtextF(PSTR("1200"),lineaxy);
                  if(BAUDRATE==1200) {
                    #ifndef OLED1306
                      printtextF(PSTR("1200 *"),lineaxy);
                    #else
                      setXY(5,lineaxy);sendChar('*');
                    #endif
                  }
                break;
                case 1:        
                  printtextF(PSTR("2400"),lineaxy);
                  if(BAUDRATE==2400) {
                    #ifndef OLED1306
                      printtextF(PSTR("2400 *"),lineaxy);
                    #else
                      setXY(5,lineaxy);sendChar('*');
                    #endif
                  }
                break;
                case 2:                  
                  printtextF(PSTR("3600"),lineaxy);
                  if(BAUDRATE==3600) {
                    #ifndef OLED1306
                      printtextF(PSTR("3600 *"),lineaxy);
                    #else
                      setXY(5,lineaxy);sendChar('*');
                    #endif
                  }
                break;                  
                case 3:                  
                  printtextF(PSTR("3850"),lineaxy);
                  if(BAUDRATE==3850) {
                    #ifndef OLED1306
                      printtextF(PSTR("3850 *"),lineaxy);
                    #else
                      setXY(5,lineaxy);sendChar('*');
                    #endif
                  }
                break;                
              }
              updateScreen=false;
            }
                    
            if(digitalRead(btnDown)==LOW && !lastbtn){
              if(subItem<3) subItem+=1;
              lastbtn=true;
              updateScreen=true;
            }
            if(digitalRead(btnUp)==LOW && !lastbtn) {
              if(subItem>0) subItem+=-1;
              lastbtn=true;
              updateScreen=true;
            }
            if(digitalRead(btnPlay)==LOW && !lastbtn) {
              switch(subItem) {
                case 0:
                  BAUDRATE=1200;
                break;
                case 1:
                  BAUDRATE=2400;
                break;
                case 2:
                  BAUDRATE=3600;
                break;                
                case 3:
                  BAUDRATE=3850;
                break;
              }
              updateScreen=true;
              #ifdef OLED1306 
                OledStatusLine();
              #endif
              lastbtn=true;
            }
            checkLastButton();
          }
          lastbtn=true;
          updateScreen=true;
        break;

        case 1:
          subItem=0;
          updateScreen=true;
          lastbtn=true;
          while(digitalRead(btnStop)==HIGH || lastbtn) {
            if(updateScreen) {
              printtextF(PSTR("Motor Ctrl"),0);
              if(mselectMask==0) printtextF(PSTR("off *"),lineaxy);
              else  printtextF(PSTR("ON *"),lineaxy);
        /*      switch(subItem) {
                case 0:
                  printtextF(PSTR("off"),lineaxy);
                  if(mselectMask==0) printtextF(PSTR("off *"),lineaxy);
                break;
                case 1:
                  printtextF(PSTR("ON"),lineaxy);
                  if(mselectMask==1) printtextF(PSTR("ON *"),lineaxy);
                break;                
              } */
              updateScreen=false;
            }
      /*              
            if(digitalRead(btnDown)==LOW && !lastbtn){
              if(subItem<1) subItem+=1;
              lastbtn=true;
              updateScreen=true;
            }
            if(digitalRead(btnUp)==LOW && !lastbtn) {
              if(subItem>0) subItem+=-1;
              lastbtn=true;
              updateScreen=true;
            }  */
            
            if(digitalRead(btnPlay)==LOW && !lastbtn) {
              mselectMask= !mselectMask;
          /*    switch(subItem) {
                case 0:
                  mselectMask=0;
                break;
                case 1:
                  mselectMask=1;
                break;
              } */
              lastbtn=true;
              updateScreen=true;
              #ifdef OLED1306 
                OledStatusLine();
              #endif              
            }
            checkLastButton();
          }
          lastbtn=true;
          updateScreen=true;
        break;

        case 2:
          subItem=0;
          updateScreen=true;
          lastbtn=true;
          while(digitalRead(btnStop)==HIGH || lastbtn) {
            if(updateScreen) {
              printtextF(PSTR("TSXCzxpolUEFSW"),0);
              if(TSXCONTROLzxpolarityUEFSWITCHPARITY==0) printtextF(PSTR("off *"),lineaxy);
              else  printtextF(PSTR("ON *"),lineaxy);
          /*    switch(subItem) {
                case 0:
                  printtextF(PSTR("off"),1);
                  if(TSXCONTROLzxpolarityUEFSWITCHPARITY==0) printtextF(PSTR("off *"),1);
                break;
                case 1:
                  printtextF(PSTR("ON"),1);
                  if(TSXCONTROLzxpolarityUEFSWITCHPARITY==1) printtextF(PSTR("ON *"),1);
                break;                
              } */
              updateScreen=false;
            }
          /*          
            if(digitalRead(btnDown)==LOW && !lastbtn){
              if(subItem<1) subItem+=1;
              lastbtn=true;
              updateScreen=true;
            }
            if(digitalRead(btnUp)==LOW && !lastbtn) {
              if(subItem>0) subItem+=-1;
              lastbtn=true;
              updateScreen=true;
            } */
            if(digitalRead(btnPlay)==LOW && !lastbtn) {
              TSXCONTROLzxpolarityUEFSWITCHPARITY = !TSXCONTROLzxpolarityUEFSWITCHPARITY;
          /*    switch(subItem) {
                case 0:
                  TSXCONTROLzxpolarityUEFSWITCHPARITY=0;
                break;
                case 1:
                  TSXCONTROLzxpolarityUEFSWITCHPARITY=1;
                break;
              } */
              lastbtn=true;
              updateScreen=true;
              #ifdef OLED1306 
                OledStatusLine();
              #endif
            }
            checkLastButton();
          }
          lastbtn=true;
          updateScreen=true;
        break;
   #ifndef Use_UEF
        case 3:
          subItem=0;
          updateScreen=true;
          lastbtn=true;
          while(digitalRead(btnStop)==HIGH || lastbtn) {
            if(updateScreen) {
              printtextF(PSTR("Skip BLK:2A"),0);
              if(skip2A==0) printtextF(PSTR("off *"),lineaxy);
              else  printtextF(PSTR("ON *"),lineaxy);
          /*    switch(subItem) {
                case 0:
                  printtextF(PSTR("off"),lineaxy);
                  if(skip2A==0) printtextF(PSTR("off *"),lineaxy);
                break;
                case 1:
                  printtextF(PSTR("ON"),lineaxy);
                  if(skip2A==1) printtextF(PSTR("ON *"),lineaxy);
                break;                
              }    */
              updateScreen=false;
            }
          /*          
            if(digitalRead(btnDown)==LOW && !lastbtn){
              if(subItem<1) subItem+=1;
              lastbtn=true;
              updateScreen=true;
            }
            if(digitalRead(btnUp)==LOW && !lastbtn) {
              if(subItem>0) subItem+=-1;
              lastbtn=true;
              updateScreen=true;
            }  */
            if(digitalRead(btnPlay)==LOW && !lastbtn) {
              skip2A = !skip2A;
          /*    switch(subItem) {
                case 0:
                  skip2A=0;
                break;
                case 1:
                  skip2A=1;
                break;
              } */
              lastbtn=true;
              updateScreen=true;
              #ifdef OLED1306 
            //    OledStatusLine();
              #endif
            } 
            checkLastButton();
          }
          lastbtn=true;
          updateScreen=true;
        break;
   #endif     
      }
    }
    checkLastButton();
  }
  updateEEPROM();

  debounce(btnStop);   
/*  while(digitalRead(btnStop)==LOW) {
    //prevent button repeats by waiting until the button is released.
    delay(50);
  }
*/
 }

 void updateEEPROM()
 {
  /* Setting Byte: 
   *  bit 0: 1200
   *  bit 1: 2400
   *  bit 2: 3600
   *  bit 3: 3850
   *  bit 4: n/a
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
    case 3600:
    settings |=4;  
    break;      
    case 3850:
    settings |=8;
    break;
  }

  if(mselectMask) settings |=128;
  if(TSXCONTROLzxpolarityUEFSWITCHPARITY) settings |=64;
  #ifndef Use_UEF
  if(skip2A) settings |=32;
  #endif

  #if defined(__AVR__)
    EEPROM.put(EEPROM_CONFIG_BYTEPOS,settings);
  #elif defined(__arm__) && defined(__STM32F1__)
    EEPROM_put(EEPROM_CONFIG_BYTEPOS,settings);
  #endif      
  setBaud();
 }

 void loadEEPROM()
 {
  byte settings=0;
  #if defined(__AVR__)
    EEPROM.get(EEPROM_CONFIG_BYTEPOS,settings);
  #elif defined(__arm__) && defined(__STM32F1__)
    EEPROM_get(EEPROM_CONFIG_BYTEPOS,&settings);
  #endif
      
  if(!settings) return;

  if(bitRead(settings,7)) {
    mselectMask=1;
  } else {
    mselectMask=0;
  }
  if(bitRead(settings,6)) {
    TSXCONTROLzxpolarityUEFSWITCHPARITY=1;
  } else {
    TSXCONTROLzxpolarityUEFSWITCHPARITY=0;
  }
  #ifndef Use_UEF
  if(bitRead(settings,5)) {
    skip2A=1;
  } else {
    skip2A=0;
  }   
  #endif
  if(bitRead(settings,0)) {
    BAUDRATE=1200;
  }
  if(bitRead(settings,1)) {
    BAUDRATE=2400;
  }
  if(bitRead(settings,2)) {
    BAUDRATE=3600;  
  }
  if(bitRead(settings,3)) {
    BAUDRATE=3850;  
  }
  setBaud();
  //UniSetup();
 
 }

void checkLastButton()
{
  if(digitalRead(btnDown) && digitalRead(btnUp) && digitalRead(btnPlay) && digitalRead(btnStop)) lastbtn=false; 
        //    setXY(0,0);
        //  sendChar(lastbtn+'0');
  delay(50);
}

