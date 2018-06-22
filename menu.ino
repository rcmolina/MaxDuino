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

 void menuMode()
 { 
  byte lastbtn=true;
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
//        #ifndef Use_UEF
        case 1:
        printtextF(PSTR("Motor Ctrl ?"),lineaxy);
        break;
        case 2:
        printtextF(PSTR("TSXCzxpUEFT ?"),lineaxy);
        break;
        case 3:
        printtextF(PSTR("Skip BLK:2A ?"),lineaxy);
        break;       

//        #endif       
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
                  if(BAUDRATE==1200) printtextF(PSTR("1200 *"),lineaxy);
                break;
                case 1:        
                  printtextF(PSTR("2400"),lineaxy);
                  if(BAUDRATE==2400) printtextF(PSTR("2400 *"),lineaxy);
                break;
                case 2:                  
                  printtextF(PSTR("3600"),lineaxy);
                  if(BAUDRATE==3600) printtextF(PSTR("3600 *"),lineaxy);
                break;                  
                case 3:                  
                  printtextF(PSTR("3850"),lineaxy);
                  if(BAUDRATE==3850) printtextF(PSTR("3850 *"),lineaxy);
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
            if(digitalRead(btnDown) && digitalRead(btnUp) && digitalRead(btnPlay) && digitalRead(btnStop)) lastbtn=false;
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
            if(digitalRead(btnDown) && digitalRead(btnUp) && digitalRead(btnPlay) && digitalRead(btnStop)) lastbtn=false;
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
              printtextF(PSTR("TSXCzxpolUEFT"),0);
              if(TSXCONTROLzxpolarityUEFTURBO==0) printtextF(PSTR("off *"),lineaxy);
              else  printtextF(PSTR("ON *"),lineaxy);
          /*    switch(subItem) {
                case 0:
                  printtextF(PSTR("off"),1);
                  if(TSXCONTROLzxpolarityUEFTURBO==0) printtextF(PSTR("off *"),1);
                break;
                case 1:
                  printtextF(PSTR("ON"),1);
                  if(TSXCONTROLzxpolarityUEFTURBO==1) printtextF(PSTR("ON *"),1);
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
              TSXCONTROLzxpolarityUEFTURBO = !TSXCONTROLzxpolarityUEFTURBO;
          /*    switch(subItem) {
                case 0:
                  TSXCONTROLzxpolarityUEFTURBO=0;
                break;
                case 1:
                  TSXCONTROLzxpolarityUEFTURBO=1;
                break;
              } */
              lastbtn=true;
              updateScreen=true;
              #ifdef OLED1306 
                OledStatusLine();
              #endif
            }
            if(digitalRead(btnDown) && digitalRead(btnUp) && digitalRead(btnPlay) && digitalRead(btnStop)) lastbtn=false;
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
            if(digitalRead(btnDown) && digitalRead(btnUp) && digitalRead(btnPlay) && digitalRead(btnStop)) lastbtn=false;
          }
          lastbtn=true;
          updateScreen=true;
        break;
   #endif     
      }
    }
    if(digitalRead(btnDown) && digitalRead(btnUp) && digitalRead(btnPlay) && digitalRead(btnStop)) lastbtn=false;
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
   *  bit 6: TSXCONTROLzxpolarityUEFTURBO
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
  if(TSXCONTROLzxpolarityUEFTURBO) settings |=64;
  #ifndef Use_UEF
  if(skip2A) settings |=32;
  #endif
  EEPROM.put(1023,settings);
  setBaud();
 }

 void loadEEPROM()
 {
  byte settings=0;
  EEPROM.get(1023,settings);
  if(!settings) return;


  if(bitRead(settings,7)) {
    mselectMask=1;
  } else {
    mselectMask=0;
  }
  if(bitRead(settings,6)) {
    TSXCONTROLzxpolarityUEFTURBO=1;
  } else {
    TSXCONTROLzxpolarityUEFTURBO=0;
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

