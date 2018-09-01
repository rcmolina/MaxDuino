#define VERSION "v1.42 MaxDuino"
// ---------------------------------------------------------------------------------
// DO NOT USE CLASS-10 CARDS on this project - they're too fast to operate using SPI
// ---------------------------------------------------------------------------------
/*
 *                                    TZXduino
 *                             Written and tested by
 *                          Andrew Beer, Duncan Edwards
 *                          www.facebook.com/Arduitape/
 *                          
 *              Designed for TZX files for Spectrum (and more later)
 *              Load TZX files onto an SD card, and play them directly 
 *              without converting to WAV first!
 *              
 *              Directory system allows multiple layers,  to return to root 
 *              directory ensure a file titles ROOT (no extension) or by 
 *              pressing the Menu Select Button.
 *              
 *              Written using info from worldofspectrum.org
 *              and TZX2WAV code by Francisco Javier Crespo
 *              
 *              ***************************************************************
 *              Menu System:
 *                TODO: add ORIC and ATARI tap support, clean up code, sleep                 
 *                
 *              V1.0
 *                Motor Control Added.
 *                High compatibility with Spectrum TZX, and Tap files
 *                and CPC CDT and TZX files.
 *                
 *                V1.32 Added direct loading support of AY files using the SpecAY loader 
 *                to play Z80 coded files for the AY chip on any 128K or 48K with AY 
 *                expansion without the need to convert AY to TAP using FILE2TAP.EXE. 
 *                Download the AY loader from http://www.specay.co.uk/download 
 *                and load the LOADER.TAP AY file loader on your spectrum first then
 *                simply select any AY file and just hit play to load it. A complete
 *                set of extracted and DEMO AY files can be downloaded from
 *                http://www.worldofspectrum.org/projectay/index.htm
 *                Happy listening!
 *                
 *                V1.8.1 TSX support for MSX added by Natalia Pujol
 *                
 *                V1.8.2 Percentage counter and timer added by Rafael Molina Chesserot along with a reworking of the OLED1306 library. 
 *                Many memory usage improvements as well as a menu for TSX Baud Rates and a refined directory controls.
 *                
 *                V1.8.3 PCD8544 library changed to use less memory. Bitmaps added and Menu system reduced to a more basic level. 
 *                Bug fixes of the Percentage counter and timer when using motor control/
 *
 *                V1.8u1 (kernel@kernelcrash.com)  
 *                Mods to V1.8 TZXDuino to enable UEF playback. Only supports gunzip'd
 *                UEF files though (that still end in .uef). Plays back UEF's as square
 *                wave not sine waves.
 */ 
 //               MAXDUINO is a reworking from casduino and tzxduino to support both at the same time on Arduino Nano. 
 //               Original idea from Rafael Molina Chasserot, testing and help from Davide Barlotti, @ManuFerHi, @BCH, Alfredo Retrocant,
 //               and @Spirax who also provided modifications to support Oled double line with small font.
 //
 //               V1.28 Better speeed loading tzx files, more than 5000 baudios using PORT instructions
 //               V1.29 3-digits counter can be configured to display m:ss with #define CNTRBASE
 //               V1.30 max TSX speed 3850 vs  cas speed 3675. Also changed in Menu.
 //               V1.31 Modified ID20,2A to support automatic pausing(@spirax). Also swapped REW-FW for block navigation(@acf76es).
 //               V1.32 Deprecated old pskipPause and new block 2A pause control option in Menu.
 //               V1.33 Blocks for manual REW/FF expanded from 10 upto 20, used as circular buffer. On Oled screen prints upto 99 blocks,
 //                     overflowing to 00.  Selecting a block in pause mode traverse last 20 blocks.
 //               V1.34 alias "EEPROM version". Now blocks can be directly read/written from EEPROM.
 //                     Also logo can be copied to eeprom using #define option. After that, you can select option
 //                     for loading logo directly from EEPROM.. this saves 512 bytes in the sketch.
 //               V1.35 Uncompressed UEF support for Acorn computers, code imported from v1.8u1
 //               V1.35t Acorn speed for loading now 1500 baud, 25% faster (standard is 1200).
 //               V1.36 Polarity handling
 //               V1.37 Minor optimizations for counter format, acorn standard/turbo speed, simplication in polarity handling and 
 //                     now controlled from menu using the same option for tsx acceleration: tsxSPEEDzxPOL.
 //               V1.38 Code reformatted, cas is optional with new #def Use_CAS.Solved a bug when back with stop button from 4th subdir.
 //               V1.39 Optional hq.uef support for Acorn using define (implemented chunks 111 and 116).
 //                     Code better organized, and new userconfig.h with help for easier configuration -Alfredo Crespo
 //               V1.40 LCD sketch savings, tsx control/polarity/UEF turbo mode can be changed while paused with menu button. 
 //                     Reworked acorn chunk 116 floating point conversion to integer.
 //               V1.41 New logos from project Sugarless (@spark2k06/@yoxxxoy). New SPLASH_SCREEN configuration (@spark2k06).
 //                     Better .tsx start/stopbits support: Now working with SVI-3x8 machines!
 //               V1.42 Bugs solved: Rewinding block not showing first digit correctly (@acf76es). 
 //                     LCD printing when block ID not recognized again in hex. First support of 1,3" oled SH1106 but using 128x32 
 //                     and not 128x64
 //
#include <SdFat.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include "userconfig.h"
#include "MaxDuino.h"

#ifdef LCDSCREEN16x2
  #include <Wire.h> 
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(LCD_I2C_ADDR,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
  char indicators[] = {'|', '/', '-',0};
  uint8_t SpecialChar [8]= { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 };
  #define SCREENSIZE 16
  #define maxblock 20
#endif

#ifdef OLED1306 
  #include <Wire.h> 
  //#include "U8glib.h"
  //U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 

  //char line0[17];
  //char line1[17];
  char indicators[] = {'|', '/', '-',92};
  #define SCREENSIZE 16
  #define maxblock 20
#endif

#ifdef P8544
  #include <pcd8544.h>
  #define ADMAX 1023
  #define ADPIN 0
  #include <avr/pgmspace.h>
  byte dc_pin = 5;
  byte reset_pin = 3;
  byte cs_pin = 4;
  pcd8544 lcd(dc_pin, reset_pin, cs_pin);
  char indicators[] = {'|', '/', '-',0};
  uint8_t SpecialChar [8]= { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 };
  #define backlight_pin 2
  
  const byte Play [] PROGMEM = {
    0x00, 0x7f, 0x3e, 0x1c, 0x08, 0x00, 0x00
  };

  const byte Paused [] PROGMEM = {
    0x00, 0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x00
  };

  const byte Stop [] PROGMEM = {
    0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e
  };
  #define SCREENSIZE 14
  #define maxblock 20
#endif

char fline[17];

SdFat sd;                           //Initialise Sd card 

SdFile entry;                       //SD card file

#define subdirLength     22         // hasta 62 no incrementa el consumo de RAM
#define filenameLength   4*subdirLength  //Maximum length for scrolling filename, hasta 255 no incrementa consumo RAM

char fileName[filenameLength + 1];    //Current filename
char sfileName[13];                   //Short filename variable
char prevSubDir[3][subdirLength];    // Subir a la EPROM ¿?
int DirFilePos[3];                   //File Positios in Directory to be restored
byte subdir = 0;
unsigned long filesize;             // filesize used for dimensioning AY files
const byte chipSelect = 10;          //Sd card chip select pin

#define btnPlay       17            //Play Button
#define btnStop       16            //Stop Button
#define btnUp         15            //Up button
#define btnDown       14            //Down button
#define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       7             //Return to SD card root
#define scrollSpeed   250           //text scroll delay
#define scrollWait    3000          //Delay before scrolling starts

byte scrollPos = 0;                 //Stores scrolling text position
unsigned long scrollTime = millis() + scrollWait;


byte motorState = 1;                //Current motor control state
byte oldMotorState = 1;             //Last motor control state
byte start = 0;                     //Currently playing flag

byte pauseOn = 0;                   //Pause state
int currentFile = 1;                //Current position in directory
int maxFile = 0;                    //Total number of files in directory
byte isDir = 0;                     //Is the current file a directory
unsigned long timeDiff = 0;         //button debounce

#if (SPLASH_SCREEN && TIMEOUT_RESET)
    unsigned long timeDiff_reset = 0;
    byte timeout_reset = TIMEOUT_RESET;
#endif

byte UP = 0;                      //Next File, down button pressed
char PlayBytes[subdirLength];

unsigned long blockOffset[maxblock];
byte blockID[maxblock];

#if (SPLASH_SCREEN && TIMEOUT_RESET)
    void(* resetFunc) (void) = 0;//declare reset function at adress 0
    /*void resetFunc() // Restarts program from beginning but does not reset the peripherals and registers
    {
    asm volatile ("  jmp 0");
    }*/
#endif

void setup() {
  
  #ifdef LCDSCREEN16x2
    lcd.init();                     //Initialise LCD (16x2 type)
    //lcd.begin();                     //Initialise LCD (16x2 type)    
    lcd.backlight();
    lcd.clear();
    #if (SPLASH_SCREEN)
        lcd.setCursor(0,0);
        lcd.print(F("Welcome to")); // Set the text at the initilization for LCD Screen (Line 1)
        lcd.setCursor(0,1); 
        lcd.print(F("Maxduino")); // Set the text at the initilization for LCD Screen (Line 2)
    #endif   
//    lcd.createChar(0, SpecialChar);
  #endif
  
  #ifdef SERIALSCREEN
    Serial.begin(115200);
  #endif
  
  #ifdef OLED1306 
    //u8g.setRot180();  // Maybe you dont need this one, depends on how the display is mounted
    //sendcommand(0xc0); //set COM output scan direction (vertical flip) - reset value is 0xc0 (or 0xc8)
    /* sendcommand(0xa1);  //set segment re-map (horizontal flip) - reset value is 0xa0 (or 0xa1) */
                      
    //u8g.setFont(u8g_font_7x14); 
    //u8g.setFont(u8g_font_unifont); 
//    u8g.setFont(u8g_font_unifontr); // last from file u8g_font_data.c

    //const int PWR_ON = 0;  //Using Pin 0 to power the oleds VCC
    //const int RESET = 4;
    //pinMode(PWR_ON, OUTPUT);
    //digitalWrite(PWR_ON, HIGH);
    //pinMode(RESET, OUTPUT);
    //digitalWrite(RESET, HIGH); 
//    delay(1000);  //Needed!
    // Initialize I2C and OLED Display
      // I2C Init

  
    Wire.begin();
    init_OLED();
    //delay(1500);              // Show logo
    //reset_display();           // Clear logo and load saved mode
    #if (!SPLASH_SCREEN)
        delay(1500);              // Show logo
        reset_display();           // Clear logo and load saved mode
    #endif
  #endif

  #ifdef P8544 
    lcd.begin ();
    analogWrite (backlight_pin, 20);
    //lcd.clear();
    P8544_splash(); 
  #endif
 
  //General Pin settings
  //Setup buttons with internal pullup
   
  //pinMode(btnPlay,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnPlay,HIGH);
  PORTC |= _BV(3);
  
  //pinMode(btnStop,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnStop,HIGH);
  PORTC |= _BV(2);

  //pinMode(btnUp,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnUp,HIGH);
  PORTC |= _BV(1);

  //pinMode(btnDown,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnDown,HIGH);
  PORTC |= _BV(0);

  //pinMode(btnMotor, INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnMotor,HIGH);
  PORTD |= _BV(6);
  
  //pinMode(btnRoot, INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnRoot, HIGH); 
  PORTD |= _BV(7);

  #ifdef SPLASH_SCREEN
      while (digitalRead(btnPlay) == HIGH & 
             digitalRead(btnStop) == HIGH &
             digitalRead(btnUp)   == HIGH &
             digitalRead(btnDown) == HIGH &
             digitalRead(btnRoot) == HIGH)
      {
        delay(100);              // Show logo (OLED) or text (LCD) and remains until a button is pressed.
      }   
      #ifdef OLED1306    
          reset_display();           // Clear logo and load saved mode
      #endif
  #endif
  
  pinMode(chipSelect, OUTPUT);      //Setup SD card chipselect pin
//  if (!sd.begin(chipSelect,SPI_FULL_SPEED)) {
    while (!sd.begin(chipSelect,SPI_FULL_SPEED)) {      
    //Start SD card and check it's working
    printtextF(PSTR("No SD Card"),0);
    //lcd_clearline(0);
    //lcd.print(F("No SD Card"));
//    return;
//    delay(250);
  } 
  
  sd.chdir();                       //set SD to root directory
  UniSetup();                       //Setup TZX specific options
    
 //printtext(VERSION,0);
  //lcd_clearline(0);
  //lcd.print(F(VERSION));
  printtextF(PSTR("Reset.."),0);
  delay(500);
  
  #ifdef LCDSCREEN16x2
    lcd.clear();
  #endif

  #ifdef P8544
    lcd.clear();
  #endif
       
  getMaxFile();                     //get the total number of files in the directory
  seekFile(currentFile);            //move to the first file in the directory
  #ifdef Use_MENU
    loadEEPROM();
  #endif  
  //delay(2000);  
  //printtextF(PSTR("Ready.."),0);
  //lcd_clearline(0);
  //lcd.print(F("Ready.."));  

  #ifdef OLED1306
    OledStatusLine();
  #endif
  
}

void loop(void) {
  
  if(start==1)
  {
    //TZXLoop only runs if a file is playing, and keeps the buffer full.
    uniLoop();
  } else {
  //  digitalWrite(outputPin, LOW);    //Keep output LOW while no file is playing.
    WRITE_LOW;    
  }
  
  if((millis()>=scrollTime) && start==0 && (strlen(fileName)> SCREENSIZE)) {
    //Filename scrolling only runs if no file is playing to prevent I2C writes 
    //conflicting with the playback Interrupt
    scrollTime = millis()+scrollSpeed;
    scrollText(fileName);
    scrollPos +=1;
    if(scrollPos>strlen(fileName)) {
      scrollPos=0;
      scrollTime=millis()+scrollWait;
      scrollText(fileName);
    }
  }
  motorState=digitalRead(btnMotor);
  #if (SPLASH_SCREEN && TIMEOUT_RESET)
      if (millis() - timeDiff_reset > 1000) //check timeout reset every second
      {
        timeDiff_reset = millis(); // get current millisecond count
        if (start==0)
        {
          timeout_reset--;
          if (timeout_reset==0)
          {
            timeout_reset = TIMEOUT_RESET;
            resetFunc();
          }
        }
        else
        {
          timeout_reset = TIMEOUT_RESET;
        }    
      }
  #endif
    
  if (millis() - timeDiff > 50) {   // check switch every 50ms 
     timeDiff = millis();           // get current millisecond count
      
     if(digitalRead(btnPlay) == LOW) {
        //Handle Play/Pause button
        if(start==0) {
          //If no file is play, start playback
          playFile();
          if (mselectMask == 1){  
            oldMotorState = !motorState;  //Start in pause if Motor Control is selected
          }
          delay(200);
          
        } else {
          //If a file is playing, pause or unpause the file                  
          if (pauseOn == 0) printtextF(PSTR("Paused"),0); 
          else printtextF(PSTR("Playing"),0);      
                         
          #ifdef LCDSCREEN16x2            
   /*         //lcd_clearline(0);
            lcd.setCursor(0,0);
            lcd.print(F("Paused "));
            //sprintf(PlayBytes,"Paused % 3d%%  %03d",newpct,lcdsegs%1000);lcd.setCursor(0,0);lcd.print(PlayBytes);               
            //lcd.print(entry.curPosition()); */

            //sprintf(PlayBytes,"Paused % 3d%%  %03d",newpct,lcdsegs%1000); sendStrXY(PlayBytes,0,0);
            if (currpct <100) {              
              itoa(newpct,PlayBytes,10);strcat_P(PlayBytes,PSTR("%"));lcd.setCursor(8,0);lcd.print(PlayBytes);
            }
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);lcd.setCursor(13,0);lcd.print(PlayBytes);
            strcpy(PlayBytes,"000");
    /*        if ((lcdsegs %(CNTRBASE*10)) <10) itoa(lcdsegs%10,PlayBytes+2,10);
            else 
               if ((lcdsegs %(CNTRBASE*10)) < CNTRBASE) itoa(lcdsegs%(CNTRBASE*10),PlayBytes+1,10);
               else
                  itoa(lcdsegs%(CNTRBASE*10) /CNTRBASE *100 + lcdsegs%CNTRBASE ,PlayBytes,10); */

            #ifndef cntrMSS
              if ((lcdsegs %1000) <10) itoa(lcdsegs%10,PlayBytes+2,10);
              else 
                 if ((lcdsegs %1000) <100)itoa(lcdsegs%1000,PlayBytes+1,10);
                 else 
                    itoa(lcdsegs%1000,PlayBytes,10);
            #endif
            #ifdef cntrMSS
              if ((lcdsegs %600) <10) itoa(lcdsegs%10,PlayBytes+2,10);
              else 
                 if ((lcdsegs %600) <60)itoa(lcdsegs%600,PlayBytes+1,10);
                 else 
                    itoa(lcdsegs%600 /60 *100 + lcdsegs%60,PlayBytes,10);
            #endif  
                              
            lcd.setCursor(13,0);lcd.print(PlayBytes); 
            
          #endif
          #ifdef OLED1306
            //sprintf(PlayBytes,"Paused % 3d%%  %03d",newpct,lcdsegs%1000); sendStrXY(PlayBytes,0,0);
            if (currpct <100) {                         
              itoa(newpct,PlayBytes,10);strcat_P(PlayBytes,PSTR("%"));setXY (8,0);sendStr(PlayBytes);
            }              
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);sendStrXY(PlayBytes,13,0);
            strcpy(PlayBytes,"000");
    /*        if ((lcdsegs %(CNTRBASE*10)) <10) itoa(lcdsegs%10,PlayBytes+2,10);
            else 
               if ((lcdsegs %(CNTRBASE*10)) < CNTRBASE) itoa(lcdsegs%(CNTRBASE*10),PlayBytes+1,10);
               else
                  itoa(lcdsegs%(CNTRBASE*10) /CNTRBASE *100 + lcdsegs%CNTRBASE ,PlayBytes,10); */

            #ifndef cntrMSS
              if ((lcdsegs %1000) <10) itoa(lcdsegs%10,PlayBytes+2,10);
              else 
                 if ((lcdsegs %1000) <100)itoa(lcdsegs%1000,PlayBytes+1,10);
                 else 
                    itoa(lcdsegs%1000,PlayBytes,10);
            #endif
            #ifdef cntrMSS
              if ((lcdsegs %600) <10) itoa(lcdsegs%10,PlayBytes+2,10);
              else 
                 if ((lcdsegs %600) <60)itoa(lcdsegs%600,PlayBytes+1,10);
                 else 
                    itoa(lcdsegs%600 /60 *100 + lcdsegs%60,PlayBytes,10);
            #endif                 
                              
            setXY(13,0);
            sendStr(PlayBytes);
            
            setXY(14,2);
            sendChar(48+block/10);
            //setXY(15,2);
            sendChar(48+block%10);   
          #endif

         #ifdef P8544                                      
/*            itoa(newpct,PlayBytes,10);
            strcat_P(PlayBytes,PSTR("%"));
            lcd.setCursor(0,3);
            lcd.print(PlayBytes); */
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);lcd.setCursor(13,0);lcd.print(PlayBytes);strcpy(PlayBytes,"000");
            if (currpct <100) {              
              itoa(newpct,PlayBytes,10);strcat_P(PlayBytes,PSTR("%"));lcd.setCursor(0,3);lcd.print(PlayBytes);
            }
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);lcd.setCursor(13,0);lcd.print(PlayBytes);
            strcpy(PlayBytes,"000");
    /*        if ((lcdsegs %(CNTRBASE*10)) <10) itoa(lcdsegs%10,PlayBytes+2,10);
            else 
               if ((lcdsegs %(CNTRBASE*10)) < CNTRBASE) itoa(lcdsegs%(CNTRBASE*10),PlayBytes+1,10);
               else
                  itoa(lcdsegs%(CNTRBASE*10) /CNTRBASE *100 + lcdsegs%CNTRBASE ,PlayBytes,10); */

            #ifndef cntrMSS
              if ((lcdsegs %1000) <10) itoa(lcdsegs%10,PlayBytes+2,10);
              else 
                 if ((lcdsegs %1000) <100)itoa(lcdsegs%1000,PlayBytes+1,10);
                 else 
                    itoa(lcdsegs%1000,PlayBytes,10);
            #endif
            #ifdef cntrMSS
              if ((lcdsegs %600) <10) itoa(lcdsegs%10,PlayBytes+2,10);
              else 
                 if ((lcdsegs %600) <60)itoa(lcdsegs%600,PlayBytes+1,10);
                 else 
                    itoa(lcdsegs%600 /60 *100 + lcdsegs%60,PlayBytes,10);
            #endif  
                              
            //lcd.setCursor(13,3);lcd.print(PlayBytes); 
            lcd.gotoRc(3,38);
            lcd.bitmap(Paused, 1, 6);
          #endif
            
          //scrollPos=0;
          //scrollText(fileName);                                       
          pauseOn = !pauseOn;
       }
       
       debounce(btnPlay);         
/*       while(digitalRead(btnPlay)==LOW) {
        //prevent button repeats by waiting until the button is released.
        delay(50);
       }
*/
     }


     if(digitalRead(btnRoot)==LOW && start==1 && pauseOn==1){
       // change tsx speed control/zx polarity/uefTurboMode
       TSXCONTROLzxpolarityUEFTURBO = !TSXCONTROLzxpolarityUEFTURBO;
       #ifdef OLED1306 
          OledStatusLine();
       #endif 
       debounce(btnRoot);  
//       while(digitalRead(btnRoot)==LOW) {
//         //prevent button repeats by waiting until the button is released.
//         delay(50);
//       }

     }

     if(digitalRead(btnRoot)==LOW && start==0){
       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Return to root of the SD card.
       //printtextF(PSTR(VERSION),0);
       //lcd_clearline(0);
       //lcd.print(F(VERSION));

        #ifdef Use_MENU
           menuMode();
           //setBaud();
           //printtextF(PSTR(VERSION),0);
           printtext(PlayBytes,0);           
           //lcd_clearline(1);
          #ifdef LCDSCREEN16x2
            printtextF(PSTR(""),1);
          #endif      
          #ifdef OLED1306
            printtextF(PSTR(""),lineaxy);
          #endif
          #ifdef P8544
            printtextF(PSTR(""),1);
          #endif      
         
          scrollPos=0;
          scrollText(fileName);
     /*     #ifdef OLED1306 
            OledStatusLine();
          #endif */
       #else             
         subdir=0;
         prevSubDir[0][0]='\0';
         prevSubDir[1][0]='\0';
         prevSubDir[2][0]='\0';
         sd.chdir(true);
         getMaxFile();
         currentFile=1;
         seekFile(currentFile);         
       #endif

       debounce(btnRoot);  
/*       while(digitalRead(btnRoot)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50);
       }
*/       
     }
     
     if(digitalRead(btnStop)==LOW && start==1) {
       stopFile();

       debounce(btnStop);
/*  
       while(digitalRead(btnStop)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50);
       }
*/       
     }
     if(digitalRead(btnStop)==LOW && start==0 && subdir >0) { 
       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif     
       fileName[0]='\0';
       subdir--;
       prevSubDir[subdir][0]='\0';     
       switch(subdir){
        case 0:
           //sprintf(fileName,"%s",prevSubDir[0]);
           sd.chdir("/",true);
           break;
        case 1:
           //sprintf(fileName,"%s%s",prevSubDir[0],prevSubDir[1]);
           sd.chdir(strcat(strcat(fileName,"/"),prevSubDir[0]),true);
           break;
        case 2:
        default:
           //sprintf(fileName,"%s%s/%s",prevSubDir[0],prevSubDir[1],prevSubDir[2]);
           subdir = 2;
           sd.chdir(strcat(strcat(strcat(strcat(fileName,"/"),prevSubDir[0]),"/"),prevSubDir[1]),true);
           break;
    /*   case 3:
       //default:
           //sprintf(fileName,"%s%s/%s/%s",prevSubDir[0],prevSubDir[1],prevSubDir[2],prevSubDir[3]);
           //subdir = 3;
           sd.chdir(strcat(strcat(strcat(strcat(strcat(strcat(fileName,"/"),prevSubDir[0]),"/"),prevSubDir[1]),"/"),prevSubDir[2]),true); 
           break; */         
       }
       //Return to prev Dir of the SD card.
       //sd.chdir(fileName,true);
       //sd.chdir("/CDT");       
       //printtext(prevDir,0); //debug back dir
       
       getMaxFile();
       //currentFile=1;
       currentFile=DirFilePos[subdir];
       
/*          PlayBytes[0]='\0'; itoa(currentFile,PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000);
          PlayBytes[0]='\0'; itoa(DirFilePos[0],PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000);
          PlayBytes[0]='\0'; itoa(DirFilePos[1],PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000);
          PlayBytes[0]='\0'; itoa(DirFilePos[2],PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000); */

       UP=1;
       seekFile(currentFile);
       DirFilePos[subdir]=0;

    /*      PlayBytes[0]='\0'; itoa(currentFile,PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000);
          PlayBytes[0]='\0'; itoa(DirFilePos[0],PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000);
          PlayBytes[0]='\0'; itoa(DirFilePos[1],PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000);
          PlayBytes[0]='\0'; itoa(DirFilePos[2],PlayBytes,10); 
          printtext(PlayBytes,0);
          delay(1000); */

       debounce(btnStop);   
/*       while(digitalRead(btnStop)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50);
       }
*/
     }     

     if(digitalRead(btnUp)==LOW && start==1 && pauseOn==1) {

/*
       bytesRead=11;                     // for tzx skip header(10) + GETID(11)
       currentTask=PROCESSID;
*/
/*
       bytesRead=0;                       // for both tap and tzx, no header for tap
       currentTask=GETFILEHEADER;         //First task (default): search for tzx header
*/
       if (block > 0) block--;
       else block = 99; 
/*
       EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
       EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
       currentTask=PROCESSID; 
       
       SetPlayBlock();
*/
       GetAndPlayBlock();        
       debounce(btnUp);         
/*       while(digitalRead(btnUp)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50); 
       }
 */      
     }
         
     if(digitalRead(btnUp)==LOW && start==0) {
       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Move up a file in the directory
       scrollTime=millis()+scrollWait;
       scrollPos=0;
       upFile();
       
       debounce(btnUp);       
/*       while(digitalRead(btnUp)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50); 
       }
*/
     }

     if(digitalRead(btnDown)==LOW && start==1 && pauseOn==1) {

/*
       bytesRead=11;                     // for tzx skip header(10) + GETID(11)
       currentTask=PROCESSID;
*/
/*
       bytesRead=0;                       // for both tap and tzx, no header for tap
       currentTask=GETFILEHEADER;         //First task (default): search for tzx header
*/
       if (block < 99) block++;
       else block = 0;
/*
       EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
       EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
       currentTask=PROCESSID;
       
       SetPlayBlock();
*/
       GetAndPlayBlock();  
       debounce(btnDown);                  
/*       while(digitalRead(btnDown)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50);
       }
*/
     }
     
     if(digitalRead(btnDown)==LOW && start==0 ) {
       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Move down a file in the directory
       scrollTime=millis()+scrollWait;
       scrollPos=0;
       downFile();
       
       debounce(btnDown);      
/*       while(digitalRead(btnDown)==LOW) {
         //prevent button repeats by waiting until the button is released.
         delay(50); 
       }
*/
     }
     if(start==1 && (!oldMotorState==motorState) && mselectMask==1 ) {  
       //if file is playing and motor control is on then handle current motor state
       //Motor control works by pulling the btnMotor pin to ground to play, and NC to stop
       if(motorState==1 && pauseOn==0) {
         //printtextF(PSTR("PAUSED"),0);
         #ifdef LCDSCREEN16x2
              lcd.setCursor(0,0);
              lcd.print("PAUSED ");    
         #endif 
         #ifdef OLED1306
              sendStrXY("PAUSED ",0,0);
         #endif 
         #ifdef P8544
              lcd.setCursor(0,0);
              lcd.print("PAUSED ");                       
         #endif                 
         scrollPos=0;
         scrollText(fileName);
         //lcd_clearline(0);
         //lcd.print(F("Paused "));         
         pauseOn = 1;
       } 
       if(motorState==0 && pauseOn==1) {
         //printtextF(PSTR("PLAYing"),0);
         #ifdef LCDSCREEN16x2
              lcd.setCursor(0,0);
              lcd.print("PLAYing");    
         #endif 
         #ifdef OLED1306
              sendStrXY("PLAYing",0,0);
         #endif 
         #ifdef P8544
              lcd.setCursor(0,0);
              lcd.print("PLAYing");                       
         #endif            
         scrollPos=0;
         scrollText(fileName);
         //lcd_clearline(0);
         //lcd.print(F("Playing"));   
         //delay(2250);
         pauseOn = 0;
       }
       oldMotorState=motorState;
     }
  }
}

void debounce(int boton){
  while(digitalRead(boton)==LOW){
    //prevent button repeats by waiting until the button is released.
    delay(50);
  }
}

void upFile() {    
  //move up a file in the directory
  currentFile--;
  if(currentFile<1) {
    getMaxFile();
    currentFile = maxFile;
  }
  UP=1;   
  seekFile(currentFile);
}

void downFile() {    
  //move down a file in the directory
  currentFile++;
  if(currentFile>maxFile) { currentFile=1; }
  UP=0;  
  seekFile(currentFile);
}

void seekFile(int pos) {    
  //move to a set position in the directory, store the filename, and display the name on screen.
  if (UP==1) {  
    entry.cwd()->rewind();
    for(int i=1;i<=currentFile-1;i++) {
      entry.openNext(entry.cwd(),O_READ);
      entry.close();
    }
  }

  if (currentFile==1) {entry.cwd()->rewind();}
  entry.openNext(entry.cwd(),O_READ);
  entry.getName(fileName,filenameLength);
  entry.getSFN(sfileName);
  filesize = entry.fileSize();
  ayblklen = filesize + 3;  // add 3 file header, data byte and chksum byte to file length
  if(entry.isDir() || !strcmp(sfileName, "ROOT")) { isDir=1; } else { isDir=0; }
  entry.close();

  PlayBytes[0]='\0'; 
  if (isDir==1) {
 //   strcat_P(PlayBytes,PSTR(VERSION));
    //if (subdir >0)strncpy(PlayBytes,prevSubDir[subdir-1],16);
    if (subdir >0)strcpy(PlayBytes,prevSubDir[subdir-1]);
    else strcat_P(PlayBytes,PSTR(VERSION));
    #ifdef P8544
      printtext("                 ",3);
    #endif
    
  } else {
    ltoa(filesize,PlayBytes,10);strcat_P(PlayBytes,PSTR(" bytes"));
    #ifdef P8544
      printtext("                 ",3);
    #endif
  }

  //PlayBytes[0]='\0'; itoa(DirFilePos[0],PlayBytes,10); 
  printtext(PlayBytes,0);
  //printtext(prevSubDir[0],0);

  
  scrollPos=0;
  scrollText(fileName);
}

void stopFile() {
  //TZXStop();
  TZXStop();
  if(start==1){
    printtextF(PSTR("Stopped"),0);
    //lcd_clearline(0);
    //lcd.print(F("Stopped"));
    #ifdef P8544
      lcd.gotoRc(3,38);
      lcd.bitmap(Stop, 1, 6);
    #endif
    start=0;
  }
}

void playFile() {
  //PlayBytes[0]='\0';
  //strcat_P(PlayBytes,PSTR("Playing "));ltoa(filesize,PlayBytes+8,10);strcat_P(PlayBytes,PSTR("B"));  
  if(isDir==1) {
    //If selected file is a directory move into directory
    changeDir();
  } else {
    if(entry.cwd()->exists(sfileName)) {
      printtextF(PSTR("Playing"),0);
      //printtext(PlayBytes,0);
      //lcd_clearline(0);
      //lcd.print(PlayBytes);      
      scrollPos=0;
      pauseOn = 0;
      scrollText(fileName);
      currpct=100;
      lcdsegs=0;
      UniPlay(sfileName);           //Load using the short filename
        #ifdef P8544
          lcd.gotoRc(3,38);
          lcd.bitmap(Play, 1, 6);
        #endif      
      start=1;       
    } else {
      #ifdef LCDSCREEN16x2
        printtextF(PSTR("No File Selected"),1);
      #endif      
      #ifdef OLED1306
        printtextF(PSTR("No File Selected"),lineaxy);
      #endif
      #ifdef P8544
        printtextF(PSTR("No File Selected"),1);
      #endif
      
      //lcd_clearline(1);
      //lcd.print(F("No File Selected"));
    }
  }
}

void getMaxFile() {    
  //gets the total files in the current directory and stores the number in maxFile
  
  entry.cwd()->rewind();
  maxFile=0;
  while(entry.openNext(entry.cwd(),O_READ)) {
    //entry.getName(fileName,filenameLength);
    entry.close();
    maxFile++;
  }
  entry.cwd()->rewind();
}



void changeDir() {    
  //change directory, if fileName="ROOT" then return to the root directory
  //SDFat has no easy way to move up a directory, so returning to root is the easiest way. 
  //each directory (except the root) must have a file called ROOT (no extension)
                      
  if(!strcmp(fileName, "ROOT")) {
    subdir=0;    
    sd.chdir(true);
  } else {
     //if (subdir >0) entry.cwd()->getName(prevSubDir[subdir-1],filenameLength); // Antes de cambiar
     DirFilePos[subdir] = currentFile;
     sd.chdir(fileName, true);
     if (strlen(fileName) > subdirLength) {
      //entry.getSFN(sfileName);
      strcpy(prevSubDir[subdir], sfileName);
     }
     else {
      strcpy(prevSubDir[subdir], fileName);
     }
     
     //entry.cwd()->getName(prevSubDir[subdir],filenameLength);
     //entry.getSFN(sfileName);
     //strcpy(prevSubDir[subdir], sfileName);
     //strcpy(prevSubDir[subdir], fileName);
     
     subdir++;      
  }
  getMaxFile();
  currentFile=1;
  seekFile(currentFile);
}

void scrollText(char* text)
{
  #ifdef LCDSCREEN16x2
  //Text scrolling routine.  Setup for 16x2 screen so will only display 16 chars
  if(scrollPos<0) scrollPos=0;
  char outtext[17];
  if(isDir) { outtext[0]= 0x3E; 
    for(int i=1;i<16;i++)
    {
      int p=i+scrollPos-1;
      if(p<strlen(text)) 
      {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  } else { 
    for(int i=0;i<16;i++)
    {
      int p=i+scrollPos;
      if(p<strlen(text)) 
      {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  }
  outtext[16]='\0';
  printtext(outtext,1);
  //lcd_clearline(1);
  //lcd.print(outtext);
  #endif

  #ifdef OLED1306
  //Text scrolling routine.  Setup for 16x2 screen so will only display 16 chars
  if(scrollPos<0) scrollPos=0;
  char outtext[17];
  if(isDir) { outtext[0]= 0x3E; 
    for(int i=1;i<16;i++)
    {
      int p=i+scrollPos-1;
      if(p<strlen(text)) 
      {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  } else { 
    for(int i=0;i<16;i++)
    {
      int p=i+scrollPos;
      if(p<strlen(text)) 
      {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  }
  outtext[16]='\0';
  printtext(outtext,lineaxy);
  //lcd_clearline(1);
  //lcd.print(outtext);
  #endif

  #ifdef P8544
  //Text scrolling routine.  Setup for 16x2 screen so will only display 16 chars
  if(scrollPos<0) scrollPos=0;
  char outtext[15];
  if(isDir) { outtext[0]= 0x3E; 
    for(int i=1;i<14;i++)
    {
      int p=i+scrollPos-1;
      if(p<strlen(text)) 
      {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  } else { 
    for(int i=0;i<14;i++)
    {
      int p=i+scrollPos;
      if(p<strlen(text)) 
      {
        outtext[i]=text[p];
      } else {
        outtext[i]='\0';
      }
    }
  }
  outtext[14]='\0';
  printtext(outtext,1);
  //lcd_clearline(1);
  //lcd.print(outtext);
  #endif
}

/*
void lcd_clearline(int l) {    
  //clear a single line on the LCD
  
  lcd.setCursor(0,l);
  lcd.print(F("                    "));
  lcd.setCursor(0,l);
}
*/

void printtextF(const char* text, int l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
  Serial.println(reinterpret_cast <const __FlashStringHelper *> (text));
  #endif
  
  #ifdef LCDSCREEN16x2
  /*  strncpy_P(fline, text, 16);
    for(int i=strlen(fline);i<16;i++) fline[i]=0x20;
    //lcd.setCursor(0,l);
    //lcd.print(F("                    "));
    lcd.setCursor(0,l);
    lcd.print(fline); */
    //lcd.print(reinterpret_cast <const __FlashStringHelper *> (text));
    lcd.setCursor(0,l);

    char x = 0;
    while (char ch=pgm_read_byte(text+x)) {
      lcd.print(ch);
      x++;
    }
    for(x; x<16; x++) lcd.print(' ');  
    
  #endif

 #ifdef OLED1306
   /*   strncpy_P(fline, text, 16);
      for(int i=strlen(fline);i<16;i++) fline[i]=0x20;
      sendStrXY(fline,0,l); */
      setXY(0,l);
      
      char x = 0;
      while (char ch=pgm_read_byte(text+x)) {
        sendChar(ch);
        x++;
      }
      for(x; x<16; x++) sendChar(' ');

/*
      for(int i=0;i<16;i++)
      {
        int j;
        if(i<strlen(text))  j=pgm_read_byte(text);
        else  j=0x20;
        sendChar(j);
      }  
*/       
   
  /*    u8g.firstPage();
      do {  
         u8g.drawStr( 0, 15, line0);   
         u8g.drawStr( 0, 30, line1);    
      } while( u8g.nextPage() ); */
      //sendStrXY(line0,0,0);
      //sendStrXY(line1,0,1);
  #endif

  #ifdef P8544
    strncpy_P(fline, text, 14);
    for(int i=strlen(fline);i<14;i++) fline[i]=0x20;
    //lcd.setCursor(0,l);
    //lcd.print(F("              "));
    lcd.setCursor(0,l);
    lcd.print(fline);
    //lcd.print(reinterpret_cast <const __FlashStringHelper *> (text));
  #endif 
   
}

void printtext(char* text, int l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
  Serial.println(text);
  #endif
  
  #ifdef LCDSCREEN16x2
/*    for(int i=0;i<16;i++)
    {
      if(i<strlen(text))  fline[i]=text[i];
      else  fline[i]=0x20;
    }    

    //lcd.setCursor(0,l);
    //lcd.print(F("                    "));
    lcd.setCursor(0,l);
    lcd.print(fline); */
    lcd.setCursor(0,l);
        
    char ch;
    const char len = strlen(text);
    for(char x=0;x<16;x++) {
        if(x<len)  ch=text[x];
        else  ch=0x20;
        lcd.print(ch); 
    }
    
  #endif

  #ifdef OLED1306
/*
      for(int i=0;i<16;i++)
      {
        if(i<strlen(text))  fline[i]=text[i];
        else  fline[i]=0x20;
      }    
      sendStrXY(fline,0,l);
*/

      setXY(0,l); 

      char ch;
      const char len = strlen(text);
       for(char x=0;x<16;x++)
      {
        if(x<len)  ch=text[x];
        else  ch=0x20;
        sendChar(ch);
      }       

/*
      char x = 0;
       while ((*text) && (x<16)) 
      {
        sendChar(*text);
        *text++;
        x++;
      }
      for(x; x<16; x++) sendChar(' ');      
*/              
      //setXY(0,l);
      //sendStr("                    ");
      //setXY(0,l);
      //sendStr(text);
  #endif

  #ifdef P8544
    for(int i=0;i<14;i++)
      {
        if(i<strlen(text))  fline[i]=text[i];
        else  fline[i]=0x20;
      }  
    //lcd.setCursor(0,l);
    //lcd.print(F("              "));
    lcd.setCursor(0,l);
    lcd.print(fline);
  #endif 
   
}

#ifdef OLED1306
  void OledStatusLine() {
    #ifdef XY
    setXY(4,2);
    sendStr("ID:   BLK:");
 //   setXY(11,2);
 //   sendStr("BLK:");
    setXY(0,3);
    //sendChar(48+BAUDRATE/1000); sendChar(48+(BAUDRATE/100)%10);sendChar(48+(BAUDRATE/10)%10);sendChar(48+BAUDRATE%10);
    itoa(BAUDRATE,input,10);sendStr(input);
    setXY(5,3);
    if(mselectMask==1) sendStr(" M:ON");
    else sendStr("m:off");    
    setXY(11,3); 
    if (TSXCONTROLzxpolarityUEFTURBO == 1) sendStr(" %^ON");
    else sendStr("%^off");
    #endif
  }
#endif

void SetPlayBlock()
{

       printtextF(PSTR(" "),0);
       #ifdef LCDSCREEN16x2
    //     lcd.setCursor(11,0);lcd.print(" <<>>");
    //     lcd.setCursor(11,0);lcd.print(" Paus");
         lcd.setCursor(0,0);
         lcd.print("BLK:");
         lcd.print(block);lcd.print(' ');
         
         if (bytesRead > 0){
           lcd.print("ID:");lcd.print(currentID,HEX); // Block ID en hex
    //       lcd.print(' ');lcd.print(bytesRead,HEX);
         }
       #endif
       #ifdef OLED1306
    //      setXY(11,0);sendStr(" <<>>");
    //      setXY(11,0);sendStr(" Paus"); 
          setXY(0,0);
          sendStr("BLK:");
          utoa(block, input, 10);sendStr(input);sendChar(' ');
          if (bytesRead > 0){
        //    setXY(11,0);
            sendStr("ID:");utoa(currentID,input,16);sendStr(strupr(input)); // Block ID en hex
     //       sendChar(' ');utoa(bytesRead,input,16);sendStr(strupr(input));            
          }
       #endif      
       #ifdef P8544
          lcd.setCursor(12,3);lcd.print('B'+block);
       #endif

//       printtextF(PSTR("REWind ALL..    "),0);

   //    delay(1000);       
   //    printtextF(PSTR("Paused"),0);
       //block--;
       //currentID=BlockCurrentID[block];
       //pos=Fpos[block];
       //entry.seekSet(Fpos[block]);
       //entry.seekSet(0);
       //pos=0;       
       //lcd.println(pos); lcd.print(" ");
       //stopFile();
       //playFile(); 
       //TZXPlay(sfileName);           //Load using the short filename

       //reset data block values
       //clearBuffer();
       currpct=100; 
//       newpct=(100 * bytesRead)/filesize;

       lcdsegs=0;       
       currentBit=0;                               // fallo reproducción de .tap tras .tzx
       pass=0;
              

/*
       entry.getSFN(sfileName);

       currentBlockTask = READPARAM;               //First block task is to read in parameters
       //currentTask=PROCESSID;
       //currentID=TAP;
       currentTask=GETFILEHEADER;                  //First task: search for header
       checkForEXT(sfileName);
*/


//  entry.getSFN(sfileName);
  
  
//  Timer1.stop();                              //Stop timer interrupt

//  bytesRead=0;                                //start of file

  checkForEXT (sfileName);
  if (!casduino) {
    currentBlockTask = READPARAM;               //First block task is to read in parameters
//    clearBuffer2();                               // chick sound with CASDUINO clearBuffer()
//    isStopped=false;
//    pinState=LOW;                               //Always Start on a LOW output for simplicity
//    count = 255;                                //End of file buffer flush
//    EndOfFile=false;
//    digitalWrite(outputPin, pinState);
    Timer1.setPeriod(1000);                     //set 1ms wait at start of a file.
  }


   
/*
       isStopped=false;
       pinState=LOW;                               //Always Start on a LOW output for simplicity
       count = 255;                                //End of file buffer flush
       EndOfFile=false;
       digitalWrite(outputPin, pinState);
*/   
}

void GetAndPlayBlock()
{
   EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
   EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
   currentTask=PROCESSID; 
   
   SetPlayBlock(); 
}

