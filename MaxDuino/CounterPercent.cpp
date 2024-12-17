#include "configs.h"
#include "Arduino.h"
#include "compat.h"
#include "Display.h"
#include "utils.h"
#include "file_utils.h"

byte currpct = 100;
unsigned int lcdsegs = 0;

static unsigned long timeDiff2 = 0;
static byte newpct = 0;

void lcdTime() {
  if (millis() - timeDiff2 > 1000) {   // check switch every second 
    timeDiff2 = millis();           // get current millisecond count

    #ifdef LCDSCREEN16x2

      if (lcdsegs % 10 != 0) {
        // ultima cifra 1,2,3,4,5,6,7,8,9
        utoa(lcdsegs%10,PlayBytes,10);
        lcd.setCursor(15,0);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % CNTRBASE != 0) {
        // es 10,20,30,40,50,60,70,80,90,110,120,..
        utoa(lcdsegs%CNTRBASE,PlayBytes,10);
        lcd.setCursor(14,0);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % (CNTRBASE*10) != 0) {
        // es 100,200,300,400,500,600,700,800,900,1100,..
        utoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
        lcd.setCursor(13,0);
        lcd.print(PlayBytes);
      } 
      else {
        // es 000,1000,2000,...
        lcd.setCursor(13,0);
        lcd.print('0');
        lcd.print('0');
        lcd.print('0');
      }

      lcdsegs++;
    #endif

    #ifdef OLED1306
      #ifdef XY2force
        if (lcdsegs % 10 != 0) {
          // ultima cifra 1,2,3,4,5,6,7,8,9
          utoa(lcdsegs%10,(char *)input,10);
          sendStrXY((char *)input,15,0);
        }
        else if (lcdsegs % CNTRBASE != 0) {
          // es 10,20,30,40,50,60,70,80,90,110,120,..
          utoa((lcdsegs % CNTRBASE)/10,(char *)input,10);
          input[1]='0';
          input[2]=0;
          sendStrXY((char *)input,14,0);
        }
        else if (lcdsegs % (CNTRBASE*10) != 0) {
          // es 100,200,300,400,500,600,700,800,900,1100,..
          utoa((lcdsegs % (CNTRBASE*10))/CNTRBASE,(char *)input,10);
          input[1]='0';
          input[2]='0';
          input[3]=0;
          sendStrXY((char *)input,13,0);
        }
        else {
          // es 000,1000,2000,...
          sendStrXY("000",13,0);
        }

        lcdsegs++;

      #else // not XY2force

        if (lcdsegs % 10 != 0) {
          // ultima cifra 1,2,3,4,5,6,7,8,9
          setXY(15,0);
          sendChar(48+lcdsegs%10);
        }
        else if (lcdsegs % CNTRBASE != 0) {
          // es 10,20,30,40,50,60,70,80,90,110,120,..
          setXY(14,0);
          sendChar(48+(lcdsegs%CNTRBASE)/10);
          sendChar('0');
        } else if (lcdsegs % (CNTRBASE*10) != 0) {
          // es 100,200,300,400,500,600,700,800,900,1100,..
          setXY(13,0);
          sendChar(48+(lcdsegs % (CNTRBASE*10))/CNTRBASE);
          sendChar('0');
          sendChar('0');
        } else {
          // es 000,1000,2000,...
          setXY(13,0);
          sendChar('0');
          sendChar('0');
          sendChar('0');
        }

        lcdsegs++; 

      #endif
    #endif

    #ifdef P8544
      if (lcdsegs % 10 != 0) {
        // ultima cifra 1,2,3,4,5,6,7,8,9
        utoa(lcdsegs%10,PlayBytes,10);
        lcd.setCursor(13,3);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % CNTRBASE != 0) {
        // es 10,20,30,40,50,60,70,80,90,110,120,..
        utoa(lcdsegs%CNTRBASE,PlayBytes,10);
        lcd.setCursor(12,3);
        lcd.print(PlayBytes);
      }
      else if (lcdsegs % (CNTRBASE*10) != 0) {
        // es 100,200,300,400,500,600,700,800,900,1100,..
        utoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
        lcd.setCursor(11,3);
        lcd.print(PlayBytes);
      }
      else {
        // es 000,1000,2000,...
        lcd.setCursor(11,3);
        lcd.print('0');
        lcd.print('0');
        lcd.print('0');
      }
    #endif
  }
}

void lcdPercent() {
  newpct=(100 * bytesRead)/filesize;                   
  if (currpct==100) {
      currpct= 0;
  }

  if (((newpct>currpct) || currpct==0) && (newpct % 1 == 0)) {
    #ifdef LCDSCREEN16x2            
      lcd.setCursor(8,0);
      lcd.print(newpct);lcd.print('%');
    #endif             

    #ifdef OLED1306
      #ifdef XY2force
        if (newpct <10) {
          input[0]=' ';
          input[1]=48+(newpct%10);
          input[2]='%';
          input[3]=0;
          sendStrXY((char *)input,8,0);
        }
        else if (newpct <100) {
          input[0]=48+(newpct/10);
          input[1]=48+(newpct%10);
          input[2]='%';
          input[3]=0;
          sendStrXY((char *)input,8,0);
        }
        else {
          input[0]='1';
          input[1]='0';
          input[2]='0';
          input[3]='%';
          input[4]=0;
          sendStrXY((char *)input,8,0);
        }
                                          
      #else // not XY2force
        if (newpct <10) {
          setXY(8,0);
          sendChar(' ');
          sendChar(48+newpct%10);
        }
        else if (newpct <100) {
          setXY(8,0);
          sendChar(48+newpct/10);
          sendChar(48+newpct%10);
        }
        else {
          setXY(8,0);
          sendChar('1');
          sendChar('0');
          sendChar('0');
        }

        sendChar('%');             
      #endif                   
    #endif

    #ifdef P8544
      lcd.setCursor(0,3);
      lcd.print(newpct);lcd.print('%');
    #endif
    
    currpct = newpct;
  }
}
