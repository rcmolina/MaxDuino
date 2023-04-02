/**************************************************************
 *                  Casduino Menu Code:
 *  Menu Button (was motor control button) opens menu
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
#include "buttons.h"

#if defined(LOAD_EEPROM_SETTINGS)
#include "EEPROM.h"
#endif

enum MenuItems{
  BAUD_RATE,
#ifndef NO_MOTOR
  MOTOR_CTL,
#endif
  TSX_POL_UEFSW,
#ifdef MenuBLK2A
  BLK2A,
#endif
  _Num_Menu_Items
};

void menuMode()
{ 
  byte menuItem=0;
  byte subItem=0;
  byte updateScreen=true;
  
  while(!button_stop() || lastbtn)
  {
    if(updateScreen) {
      printtextF(PSTR("Menu"),0);
      switch(menuItem) {
        case MenuItems::BAUD_RATE:
        printtextF(PSTR("Baud Rate ?"),lineaxy);
        break;
      #ifndef NO_MOTOR
        case MenuItems::MOTOR_CTL:
        printtextF(PSTR("Motor Ctrl ?"),lineaxy);
        break;
      #endif
        case MenuItems::TSX_POL_UEFSW:
        printtextF(PSTR("TSXCzxpUEFSW ?"),lineaxy);
        break;
      #ifdef MenuBLK2A
        case MenuItems::BLK2A:
        printtextF(PSTR("Skip BLK:2A ?"),lineaxy);
        break;       
      #endif
             
      }
      updateScreen=false;
    }
    if(button_down() && !lastbtn){
      if(menuItem<MenuItems::_Num_Menu_Items-1) menuItem+=1;
      lastbtn=true;
      updateScreen=true;
    }
    if(button_up() && !lastbtn) {
      if(menuItem>0) menuItem+=-1;
      lastbtn=true;
      updateScreen=true;
    }
    if(button_play() && !lastbtn) {
      switch(menuItem){
        case MenuItems::BAUD_RATE:
          subItem=0;
          updateScreen=true;
          lastbtn=true;
          while(!button_stop() || lastbtn) {
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
                    
            if(button_down() && !lastbtn){
              if(subItem<3) subItem+=1;
              lastbtn=true;
              updateScreen=true;
            }
            if(button_up() && !lastbtn) {
              if(subItem>0) subItem+=-1;
              lastbtn=true;
              updateScreen=true;
            }
            if(button_play() && !lastbtn) {
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
              #if defined(OLED1306) && defined(OSTATUSLINE) 
                OledStatusLine();
              #endif
              lastbtn=true;
            }
            checkLastButton();
          }
          lastbtn=true;
          updateScreen=true;
        break;

        #ifndef NO_MOTOR
          case MenuItems::MOTOR_CTL:
            doOnOffSubmenu(PSTR("Motor Ctrl"), mselectMask);
            lastbtn=true;
            updateScreen=true;
            break;
        #endif

        case MenuItems::TSX_POL_UEFSW:
          doOnOffSubmenu(PSTR("TSXCzxpolUEFSW"), TSXCONTROLzxpolarityUEFSWITCHPARITY);
          lastbtn=true;
          updateScreen=true;
          break;
          
        #ifdef MenuBLK2A
          case MenuItems::BLK2A:
            doOnOffSubmenu(PSTR("Skip BLK:2A"), skip2A);
            lastbtn=true;
            updateScreen=true;
            break;
        #endif     
      }
    }
    checkLastButton();
  }
  #ifdef LOAD_EEPROM_SETTINGS
    updateEEPROM();
  #endif

  debounce(button_stop);
}

void doOnOffSubmenu(const char * title, byte& refVar)
{
  bool updateScreen=true;
  lastbtn=true;
  while(!button_stop() || lastbtn) {
    if(updateScreen) {
      printtextF(title,0);
      if(refVar==0) printtextF(PSTR("off *"),lineaxy);
      else  printtextF(PSTR("ON *"),lineaxy);
      updateScreen=false;
    }
    
    if(button_play() && !lastbtn) {
      refVar = !refVar;
      lastbtn=true;
      updateScreen=true;
      #if defined(OLED1306) && defined(OSTATUSLINE) 
        OledStatusLine();
      #endif              
    }
    checkLastButton();
  }
}

#ifdef LOAD_EEPROM_SETTINGS
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
      if(bitRead(settings,7)) {
        mselectMask=1;
      } else {
        mselectMask=0;
      }
    #endif

    if(bitRead(settings,6)) {
      TSXCONTROLzxpolarityUEFSWITCHPARITY=1;
    } else {
      TSXCONTROLzxpolarityUEFSWITCHPARITY=0;
    }
    
    #ifdef MenuBLK2A
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
  }
#endif

void checkLastButton()
{
  if(!button_down() && !button_up() && !button_play() && !button_stop()) lastbtn=false; 
  delay(50);
}
