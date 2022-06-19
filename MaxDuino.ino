#define VERSION "new MaxDuino"
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
 //               MAXDUINO http://github.com/rcmolina/maxduino is a reworking from casduino and tzxduino 
 //               to support both at the same time on Arduino Nano. Original idea from Rafael Molina Chasserot, 
 //               testing and help from Davide Barlotti, @ManuFerHi, @BCH, Alfredo Retrocant, @jgilcas and @Spirax
 //               who also provided modifications to support Oled double line with small font.
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
 //               V1.43 aka "Mar Menor". Implemented half-interval (logarithmic) search for dirs and block selection. 
 //                     Now root button used as pivot. Menu funcions now activated with ROOT + STOP, new half-interval search with 
 //                     ROOT + UP/DOWN. (Frank Schröder)
 //               V1.44 New define btnRoot_AS_PIVOT in userconfig.h so it can be deactivated by user. New #define in Maxduino.h to support 
 //                     Antonio Villena's MINIDUINO new design with amplifier and new define OLED_SETCONTRAS in userconfig.h for contrast 
 //                     value override. New Miniduino logo. It's posible to select RECORD_EEPROM and LOAD_EEPROM both for better testing 
 //                     when selecting new logos in userconfig.h, pressing MENU simulates a reset to show logo again.
 //               V1.45 New ID15 testing code, can be deactivated from userconfig.h to save space. Amstrad bugs solved: 
 //                     Deflektor and other cdts now loading.
 //               V1.46 OLED 128x64 mode with 8 rows, logo can also be loaded with full resolution (@geloalex, @merlinkv, @acf76es)
 //               V1.47 Optional BLOCKID_INTO_MEM if loading many turbo short blocks. BLOCK_EEPROM_PUT must be disabled when loading 
 //                     Amstrad cpc Breaking Baud demo. Bug fixed: block counter should not be incremented when pausing.
 //               V1.48 New chunks for Acorn computers: Implemented parity handling when loading protected games (@acf76es).
 //                     Remove warnings when compilation fron IDE (@llopis).
 //               V1.49 ID15 adjustment for slow Amstrad musical loaders.
 //               V1.50 aka "Maria". File auto-scrolling when holding down UP or DOWN buttons (Frank Schröder).
 //               V1.51 Dragon TOSEC files with short leader loading now.
 //               V1.52 New arduino nano EVERY micro supported. Need work for turbo loading.
 //                     Different config files for new high capacity chips (Riccardo Repetto).
 //                     Compressed logo where only even columns are used for load/save with EEPROM.
 //               V1.53 Some rework for Arduino nano every and timers TCA / TCB0 and using last SDFat for better speed.
 //               V1.54 Maintenance release. Final byte patched again for Dragon and Coco, got switched when testing in v1.51. 
 //               V1.55 support for BBC chunk 0x117 (@vanekp)
 //               V1.56 aka "baby Fran". ID19 short implementation to load zx81 tzx files. 
 //                     Now you can use TSXControl to speedup loadings
 //               V1.57 Changed zx81 TSXControl to Speed to avoid polarity problems. 
 //                     Better zx81 file loading (@El Viejoven FX). Removed Amstrad tweaking to slower ID15, 
 //                     Oricium game now can load in 4 minutes in Oric Atmos, file attached for testing.
 //               V1.58 STM32 testing support thanks to @Sildenafile.
 //               V1.59 Oric native tap support.
 //               V1.60 Oric tap speedup if speed greater than 2400.
 //               V1.61 Graphic 128x64 and text 128x32 combined mode for Oled screens. Optional 8x8 bold font.
 //                     Better XY2 thanks to @alferboy.
 //               V1.62 aka "Marble". New StandardFont (8x8) pack. DoubleFont (8x16) support. 
 //                     Improved speed loading in oled marble mode (XY2force) @alferboy. 
 //                     MenuBLK2A option in userconfig, no dependencies from UEF (@merlinkv).
 //               V1.63 New Logos. Fixed side-effect on MSX cas when implementing Oric native tap files loading.
 //               V1.64 V1.64 Thinary PCB support (cheap arduino nano every). 
 //                     Many thanks to Ricky El-Qasem for his help and his perseverance for this to work !!
 //               V1.65 Code rearrangement for esasier pinout modification. 
 //                     Support for the new Maple miniduino (STM32) from Antonio Villena.
 //               V1.66 aka "Beast Nano". New userconfig option for optional oled on screen print, so more space for nano 
 //                     to include oric and uef in the same upgrade. Fixed bug when dragon to msx switching without reset.
 //               V1.67 End of file adjustments, better loading in Camputerx Lynx tzx id15. 
 //                     Removed Oric final pause, for testing, forgot to remove it. Some changes in STM32 for handling the clock.
 //               V1.68 aka "ALLinOne". Maintenance release, new optimizations and changes for Acorn computers and Oric.
 //                     Solved a bug for zx polarity. Dragon 32 optimizations.
 //               V1.69 aka "Jumbo". New TZX Block Search with online Fast Forwarding, enjoy (still need testing).
 //               V1.70 aka "optiboot". To flash this complete relase in your Arduino 328, ask your seller to burn optiboot first !! 
 //               V1.71 aka "Clive". Better Blockmode: when menu pressed in pause mode, jump 20 blocks with REW OR FF
 //               V1.72 New suboption XY2shadedplay for XY2. Softwire working with both OLED and LCD to save extra 550 bytes, 
 //                     I2C can be configured in fastmode for better performance. 
 //               V1.73 aka "Christmas21 edition". Better config frontend, some adjustments in hwconfig.h
 //               V1.74 aka "Christmas21 Eve". Used SoftI2CMaster instead of Softwire to save more space, new adjusted configs.
 //               V1.75 New option to handle more than 255 block in Blockmode if needed. Decrease block count in Blockmode
 //                     to skip some blocks thus matching live oled block count.
 //               V1.76 New option to trace ID15 blocks #BLOCKID15_IN. Support variable baudrate on the fly 
 //                     for Amstrad CPC ID11 blocks (like TSXControl).
 //
#if defined(__AVR_ATmega2560__)
  #include "userMAXconfig.h"
#elif defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__)
  #include "userEVERYconfig.h"
#elif defined(__arm__) && defined(__STM32F1__)
  #include "userSTM32config.h"  
#elif defined(SEEED_XIAO_M0)
  #include "userSEEEDUINO_XIAO_M0config.h"
#else //__AVR_ATmega328P__
  #include "userconfig.h"
#endif

#include "MaxDuino.h"
#include "hwconfig.h"
#include "buttons.h"

#if defined(BLOCK_EEPROM_PUT) || defined(LOAD_EEPROM_LOGO) || defined(RECORD_EEPROM_LOGO) || defined(LOAD_EEPROM_SETTINGS)
#include <EEPROM.h>
#endif

char fline[17];

SdFat sd;                           //Initialise Sd card 
SdBaseFile entry, currentDir, tmpdir;                       //SD card file and directory

#define filenameLength 256

char fileName[filenameLength + 1];    //Current filename
#define nMaxPrevSubDirs 10
char prevSubDir[SCREENSIZE+1];
uint16_t DirFilePos[nMaxPrevSubDirs];  //File Positions in Directory to be restored (also, history of subdirectories)
byte subdir = 0;
unsigned long filesize;             // filesize used for dimensioning AY files

byte scrollPos = 0;                 //Stores scrolling text position
unsigned long scrollTime = millis() + scrollWait;

#ifndef NO_MOTOR
byte motorState = 1;                //Current motor control state
byte oldMotorState = 1;             //Last motor control state
#endif

byte start = 0;                     //Currently playing flag

byte pauseOn = 0;                   //Pause state
uint16_t currentFile = 0;           //File index (per filesystem) of current file, relative to current directory (pointed to by currentDir)
uint16_t maxFile = 0;                    //Total number of files in directory
uint16_t oldMinFile = 0;
uint16_t oldMaxFile = 0;
#define fnameLength  5
char oldMinFileName[fnameLength];
char oldMaxFileName[fnameLength];
byte isDir = 0;                     //Is the current file a directory
unsigned long timeDiff = 0;         //button debounce

#if (SPLASH_SCREEN && TIMEOUT_RESET)
    unsigned long timeDiff_reset = 0;
    byte timeout_reset = TIMEOUT_RESET;
#endif

char PlayBytes[17];

unsigned long blockOffset[maxblock];
byte blockID[maxblock];

byte lastbtn=true;

#if (SPLASH_SCREEN && TIMEOUT_RESET)
    void(* resetFunc) (void) = 0;//declare reset function at adress 0
    /*void resetFunc() // Restarts program from beginning but does not reset the peripherals and registers
    {
    asm volatile ("  jmp 0");
    }*/
#endif

void setup() {

  #include "pinSetup.h"
  pinMode(chipSelect, OUTPUT);      //Setup SD card chipselect pin

  #ifdef USB_STORAGE_ENABLED
  if (sd.begin(chipSelect,SPI_FULL_SPEED)) {
    // need to do this as early as possible, to ensure mass storage gets enumerated
    // see https://github.com/adafruit/Adafruit_TinyUSB_ArduinoCore/issues/4
    setup_usb_storage();
  }
  #endif

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
    #if defined(Use_SoftI2CMaster) 
         i2c_init();
    #else
        Wire.begin();
    #endif    
    init_OLED();
    //delay(1500);              // Show logo
    //reset_display();           // Clear logo and load saved mode
    #if (!SPLASH_SCREEN)
      #if defined(LOAD_MEM_LOGO) || defined(LOAD_EEPROM_LOGO)
        delay(1500);              // Show logo
      #endif
        reset_display();           // Clear logo and load saved mode
    #endif
  #endif

  #ifdef P8544 
    lcd.begin ();
    analogWrite (backlight_pin, 20);
    //lcd.clear();
    P8544_splash(); 
  #endif
 
  #ifdef SPLASH_SCREEN
      while (!button_any()){
        delay(100);              // Show logo (OLED) or text (LCD) and remains until a button is pressed.
      }   
      #ifdef OLED1306    
          reset_display();           // Clear logo and load saved mode
      #endif
  #endif
  
    while (!sd.begin(chipSelect,SPI_FULL_SPEED)) {
     //Start SD card and check it's working
      printtextF(PSTR("No SD Card"),0);
    //lcd_clearline(0);
    //lcd.print(F("No SD Card"));
//    return;
//    delay(250);
    }    

  changeDirRoot();
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

/*
  #ifdef SHOW_DIRNAMES
    oldMaxFile=maxFile;
    seekFile(oldMaxFile);
    str4cpy(oldMaxFileName,fileName);
  #endif
*/  
  seekFile();            //move to the first file in the directory
/*
  #ifdef SHOW_DIRNAMES
    str4cpy(oldMinFileName,fileName);
  #endif
*/
  #ifdef LOAD_EEPROM_SETTINGS
    loadEEPROM();
  #endif  
  //delay(2000);  
  //printtextF(PSTR("Ready.."),0);
  //lcd_clearline(0);
  //lcd.print(F("Ready.."));  

  #if defined(OLED1306) && defined(OSTATUSLINE)
    OledStatusLine();
  #endif
  
}

/*void setup() {
  

    INIT_OUTPORT;
    //WRITE_LOW;    
      //timer.setCount(0);
      timer.pause();
      timer.setPeriod(500000);
      timer.attachInterrupt(2,wave2); // channel 2
      timer.resume();      
} */

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
  #ifndef NO_MOTOR
  motorState=digitalRead(btnMotor);
  #endif
  
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
      
     if(button_play()) {
        //Handle Play/Pause button
        if(start==0) {
          //If no file is play, start playback
          playFile();
          #ifndef NO_MOTOR
          if (mselectMask == 1){  
            //oldMotorState = !motorState;  //Start in pause if Motor Control is selected
            oldMotorState = 0;
          }
          #endif
          delay(50);
          
        } else {
          //If a file is playing, pause or unpause the file                  
          if (pauseOn == 0) {
            printtext2F(PSTR("Paused  "),0);
    /*        #ifdef LCDSCREEN16x2
              lcd.setCursor(0,0); 
              //lcd.print(F("Paused "));
              char x = 0;
              while (char ch=pgm_read_byte(PSTR("Paused  ")+x)) {
                lcd.print(ch);
                x++;
              }
            #endif
            #ifdef OLED1306
              setXY(0,0); 
              //sendStr("Paused ");
              char x = 0;
              while (char ch=pgm_read_byte(PSTR("Paused  ")+x)) {
                sendChar(ch);
                x++;
              }              
            #endif
            #ifdef P8544
            #endif
           */
            jblks =1; 
            firstBlockPause = true;
          } else  {
            printtext2F(PSTR("Playing      "),0);
            currpct=100;
       /*     #ifdef LCDSCREEN16x2
              lcd.setCursor(0,0); 
              //lcd.print(F("Playing"));
              char x = 0;
              while (char ch=pgm_read_byte(PSTR("Playing ")+x)) {
                lcd.print(ch);
                x++;
              }                         
            #endif
            #ifdef OLED1306
              setXY(0,0); 
              //sendStr("Playing");
              char x = 0;
              while (char ch=pgm_read_byte(PSTR("Playing ")+x)) {
                sendChar(ch);
                x++;
              }                           
            #endif
            #ifdef P8544
            #endif 
           */            
            firstBlockPause = false;      
          }
/*                         
          #ifdef LCDSCREEN16x2            
            //lcd_clearline(0);
            //lcd.setCursor(0,0);
            //lcd.print(F("Paused "));
            //sprintf(PlayBytes,"Paused % 3d%%  %03d",newpct,lcdsegs%1000);lcd.setCursor(0,0);lcd.print(PlayBytes);               
            //lcd.print(entry.curPosition()); 

            //sprintf(PlayBytes,"Paused % 3d%%  %03d",newpct,lcdsegs%1000); sendStrXY(PlayBytes,0,0);
            if (currpct <100) {              
              itoa(newpct,PlayBytes,10);strcat_P(PlayBytes,PSTR("%"));lcd.setCursor(8,0);lcd.print(PlayBytes);
            }
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);lcd.setCursor(13,0);lcd.print(PlayBytes);
            //strcpy(PlayBytes,PSTR("000"));
            
            //PlayBytes[0]= 48+ (lcdsegs/CNTRBASE)%10;
            //PlayBytes[1]= 48+ (lcdsegs%CNTRBASE)/10;
            //PlayBytes[2]= 48+ (lcdsegs%CNTRBASE)%10;
            //PlayBytes[3]= '\0'
            
            PlayBytes[0]=PlayBytes[1]=PlayBytes[2]='0';
    //        if ((lcdsegs %(CNTRBASE*10)) <10) itoa(lcdsegs%10,PlayBytes+2,10);
    //        else 
    //           if ((lcdsegs %(CNTRBASE*10)) < CNTRBASE) itoa(lcdsegs%(CNTRBASE*10),PlayBytes+1,10);
    //           else
    //              itoa(lcdsegs%(CNTRBASE*10) /CNTRBASE *100 + lcdsegs%CNTRBASE ,PlayBytes,10); 

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
            } else {                          // Block number must me printed after REW
                setXY(14,2);
                sendChar(48+(block)/10);
              //setXY(15,2);
                sendChar(48+(block)%10);                            
            }
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);sendStrXY(PlayBytes,13,0);
            //strcpy(PlayBytes,PSTR("000"));
            
            //PlayBytes[0]= 48+ (lcdsegs/CNTRBASE)%10;
            //PlayBytes[1]= 48+ (lcdsegs%CNTRBASE)/10;
            //PlayBytes[2]= 48+ (lcdsegs%CNTRBASE)%10;
            //PlayBytes[3]= '\0'
                        
            PlayBytes[0]=PlayBytes[1]=PlayBytes[2]='0';
    //        if ((lcdsegs %(CNTRBASE*10)) <10) itoa(lcdsegs%10,PlayBytes+2,10);
    //        else 
    //           if ((lcdsegs %(CNTRBASE*10)) < CNTRBASE) itoa(lcdsegs%(CNTRBASE*10),PlayBytes+1,10);
    //           else
    //              itoa(lcdsegs%(CNTRBASE*10) /CNTRBASE *100 + lcdsegs%CNTRBASE ,PlayBytes,10); 

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

        //    if (currpct==100){
        //      setXY(14,2);
        //      sendChar(48+(block)/10);
            //setXY(15,2);
        //      sendChar(48+(block)%10);
        //    }   
          #endif

         #ifdef P8544                                      
       //     itoa(newpct,PlayBytes,10);
       //     strcat_P(PlayBytes,PSTR("%"));
       //     lcd.setCursor(0,3);
       //     lcd.print(PlayBytes); 
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);lcd.setCursor(13,0);lcd.print(PlayBytes);strcpy(PlayBytes,"000");
            if (currpct <100) {              
              itoa(newpct,PlayBytes,10);strcat_P(PlayBytes,PSTR("%"));lcd.setCursor(0,3);lcd.print(PlayBytes);
            }
            //sprintf(PlayBytes,"%03d",lcdsegs%1000);lcd.setCursor(13,0);lcd.print(PlayBytes);
            //strcpy(PlayBytes,PSTR("000"));
            PlayBytes[0]=PlayBytes[1]=PlayBytes[2]='0';
    //        if ((lcdsegs %(CNTRBASE*10)) <10) itoa(lcdsegs%10,PlayBytes+2,10);
    //        else 
    //           if ((lcdsegs %(CNTRBASE*10)) < CNTRBASE) itoa(lcdsegs%(CNTRBASE*10),PlayBytes+1,10);
    //           else
    //              itoa(lcdsegs%(CNTRBASE*10) /CNTRBASE *100 + lcdsegs%CNTRBASE ,PlayBytes,10);

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
*/                                              
          pauseOn = !pauseOn;
       }
       
       debounce(button_play);
     }

#ifdef ONPAUSE_POLCHG

     if(button_root() && start==1 && pauseOn==1 
                                                    #ifdef btnRoot_AS_PIVOT   
                                                            && button_stop()
                                                    #endif
                                                            ){             // change polarity

       // change tsx speed control/zx polarity/uefTurboMode
       TSXCONTROLzxpolarityUEFSWITCHPARITY = !TSXCONTROLzxpolarityUEFSWITCHPARITY;
       #if defined(OLED1306) && defined(OSTATUSLINE) 
          OledStatusLine();
       #endif 
       debounce(button_root);
     }
#endif

#ifdef btnRoot_AS_PIVOT
     lastbtn=false;     
     if(button_root() && start==0 && !lastbtn) {                                          // show min-max dir
       
       #ifdef SHOW_DIRPOS
        #if defined(LCDSCREEN16x2) && !defined(SHOW_STATUS_LCD) && !defined(SHOW_DIRNAMES)
           char len=0;
           lcd.setCursor(0,0); 
    
           lcd.print(itoa(oldMinFile,input,10)); lcd.print('<'); len += strlen(input) + 1;
           lcd.print(itoa(currentFile,input,10)); lcd.print('<'); len += strlen(input) + 1;
           lcd.print(itoa(oldMaxFile,input,10)); len += strlen(input); 
           //lcd.print(oldMinFile);lcd.print('<');lcd.print(currentFile);lcd.print('<');lcd.print(oldMaxFile);
           //const char len=strlen(itoa(oldMinFile,input,10)) + 1 + strlen(itoa(currentFile,input,10)) + 1 + strlen(itoa(oldMaxFile,input,10));
           for(char x=len;x<16;x++) lcd.print(' '); 
        #endif
        #if defined(LCDSCREEN16x2) && defined(SHOW_STATUS_LCD)        
           lcd.setCursor(0,0);
           lcd.print(BAUDRATE);
           lcd.print(' ');
           if(mselectMask==1) lcd.print(F(" M:ON"));
           else lcd.print(F("m:off"));
           lcd.print(' ');
           if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) lcd.print(F(" %^ON"));
           else lcd.print(F("%^off"));         
        #endif 
        #if defined(LCDSCREEN16x2) && defined(SHOW_DIRNAMES)

          str4cpy(input,fileName);
          GetFileName(oldMinFile); str4cpy(oldMinFileName,fileName);
          //GetFileName(currentFile); str4cpy(input,fileName); 
          GetFileName(oldMaxFile); str4cpy(oldMaxFileName,fileName);
          GetFileName(currentFile); 
        
          lcd.setCursor(0,0);
          lcd.print(oldMinFileName);lcd.print(' ');lcd.print('<');
          lcd.print((char *)input);lcd.print('<');lcd.print(' ');
          lcd.print(oldMaxFileName);                  
        #endif                
        #if defined(OLED1306) && !defined(SHOW_DIRNAMES)
          char len=0;
          setXY(0,0);
          //sendStr(itoa(oldMinFile,input,10));sendChar('<');
          //itoa(oldMinFile,input,10); sendStr((char *)input);sendChar('<'); len += strlen(input) + 1;
          sendStr(itoa(oldMinFile,input,10));sendChar('<');len += strlen(input) + 1;
          
          //sendStr(itoa(currentFile,input,10));sendChar('<');
          //itoa(currentFile,input,10); sendStr((char *)input);sendChar('<'); len += strlen(input) + 1;
          sendStr(itoa(currentFile,input,10));sendChar('<'); len += strlen(input) + 1;
                
          //sendStr(itoa(oldMaxFile,input,10));
          //itoa(oldMaxFile,input,10); sendStr((char *)input);  len += strlen(input);
          sendStr(itoa(oldMaxFile,input,10)); len += strlen(input);                   
          //const char len=strlen(itoa(oldMinFile,input,10)) + 1 + strlen(itoa(currentFile,input,10)) + 1 + strlen(itoa(oldMaxFile,input,10));
          for(char x=len;x<16;x++) sendChar(' ');                       
        #endif
        #if defined(OLED1306) && defined(SHOW_DIRNAMES)

          str4cpy(input,fileName);
          GetFileName(oldMinFile); str4cpy(oldMinFileName,fileName);
          //GetFileName(currentFile); str4cpy(input,fileName); 
          GetFileName(oldMaxFile); str4cpy(oldMaxFileName,fileName);
          GetFileName(currentFile); 
          
          setXY(0,0);
          sendStr(oldMinFileName);sendChar(' ');sendChar('<');
          sendStr((char *)input);sendChar('<');sendChar(' ');
          sendStr(oldMaxFileName);
           
        #endif
      #endif        
        while(button_root() && !lastbtn) {
           //prevent button repeats by waiting until the button is released.
           //delay(50);
           lastbtn = 1;
           checkLastButton();           
        }        
        printtext(PlayBytes,0);
     }
     
     #if defined(LCDSCREEN16x2) && defined(SHOW_BLOCKPOS_LCD)
       if(button_root() && start==1 && pauseOn==1 && !lastbtn) {                                          // show min-max block
        lcd.setCursor(11,0);
         if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) lcd.print(F(" %^ON"));
        else lcd.print(F("%^off"));  
               
        while(button_root() && start==1 && !lastbtn) {
         //prevent button repeats by waiting until the button is released.
         //delay(50);
         lastbtn = 1;
         checkLastButton();           
        }
        //printtextF(PSTR("Help"),0);       
        //printtext(PlayBytes,0);
        lcd.setCursor(11,0);
        lcd.print(' ');
        lcd.print(' ');
        lcd.print(PlayBytes);        
       }
      #endif
#endif

     if(button_root() && start==0
                                        #ifdef btnRoot_AS_PIVOT
                                              && button_stop()
                                        #endif        
                                              ){                   // go menu

       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Return to root of the SD card.
       //printtextF(PSTR(VERSION),0);
       //lcd_clearline(0);
       //lcd.print(F(VERSION));

        #if defined(Use_MENU) && !defined(RECORD_EEPROM_LOGO)
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
          #ifdef RECORD_EEPROM_LOGO
              init_OLED();
              delay(1500);              // Show logo
              reset_display();           // Clear logo and load saved mode
              printtextF(PSTR("Reset.."),0);
              delay(500);
              PlayBytes[0]='\0'; 
              strcat_P(PlayBytes,PSTR(VERSION));
              printtext(PlayBytes,0);
              #if defined(OSTATUSLINE)
                OledStatusLine();
              #endif

          #else             
             subdir=0;
             changeDirRoot();
             getMaxFile();
             currentFile=0;
             seekFile();
          #endif         
       #endif

       debounce(button_root);
     }

     if(button_stop() && start==1
                                        #ifdef btnRoot_AS_PIVOT
                                                && !button_root()
                                        #endif
                                              ){      

       stopFile();

       debounce(button_stop);
     }

     if(button_stop() && start==0 && subdir >0) {               // back subdir
       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif     
       changeDirParent();

       debounce(button_stop);   
     }
     
#ifdef BLOCKMODE
     if(button_up() && start==1 && pauseOn==1
                                                  #ifdef btnRoot_AS_PIVOT
                                                            && !button_root()
                                                  #endif
                                                          ){             //  up block sequential search                                                                 
/*
       bytesRead=11;                     // for tzx skip header(10) + GETID(11)
       currentTask=PROCESSID;
*/
/*
       bytesRead=0;                       // for both tap and tzx, no header for tap
       currentTask=GETFILEHEADER;         //First task (default): search for tzx header
*/
       firstBlockPause = false;
       #ifdef BLOCKID_INTO_MEM
         oldMinBlock = 0;
         oldMaxBlock = maxblock;
         if (block > 0) block--;
         else block = maxblock;      
       #endif
       #if defined(BLOCK_EEPROM_PUT)
         oldMinBlock = 0;
         oldMaxBlock = 99;
         if (block > 0) block--;
         else block = 99;
       #endif
       #if defined(BLOCKID_NOMEM_SEARCH)
         //oldMinBlock = 0;
         //oldMaxBlock = 99;
         if (block > jblks) block=block-jblks;
         else block = 0;
       #endif        
/*    
       EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
       EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
       currentTask=PROCESSID; 
       
       SetPlayBlock();
*/
       GetAndPlayBlock();       
       debouncemax(button_up);
     }
#endif
#if defined(BLOCKMODE) && defined(btnRoot_AS_PIVOT)
     if(button_up() && start==1 && pauseOn==1 && button_root()) {  // up block half-interval search

/*
       bytesRead=11;                     // for tzx skip header(10) + GETID(11)
       currentTask=PROCESSID;
*/
/*
       bytesRead=0;                       // for both tap and tzx, no header for tap
       currentTask=GETFILEHEADER;         //First task (default): search for tzx header
*/

        if (block >oldMinBlock) {
          oldMaxBlock = block;
          block = oldMinBlock + (oldMaxBlock - oldMinBlock)/2;
        }
/*               
       EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
       EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
       currentTask=PROCESSID; 
       
       SetPlayBlock();
*/
       GetAndPlayBlock();    
       debounce(button_up);
     }
#endif

     if(button_up() && start==0
                                      #ifdef btnRoot_AS_PIVOT
                                            && !button_root()
                                      #endif
                                            ){                         // up dir sequential search                                           

       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Move up a file in the directory
       scrollTime=millis()+scrollWait;
       scrollPos=0;
       upFile();
       debouncemax(button_up);
     }

#ifdef btnRoot_AS_PIVOT
     if(button_up() && start==0 && button_root()) {      // up dir half-interval search
       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Move up a file in the directory
       scrollTime=millis()+scrollWait;
       scrollPos=0;
       upHalfSearchFile();
       
       debounce(button_up);
     }
#endif
#if defined(BLOCKMODE) && defined(BLKSJUMPwithROOT)
     if(button_root() && start==1 && pauseOn==1){      // change blocks to jump 
      if (jblks==BM_BLKSJUMP) jblks=1; else jblks=BM_BLKSJUMP;
       #ifdef LCDSCREEN16x2
          lcd.setCursor(15,0); if (jblks==BM_BLKSJUMP) lcd.print(F("^")); else lcd.print(F("\'"));
       #endif
       #ifdef OLED1306
          #ifdef XY2
                if (jblks==BM_BLKSJUMP) sendStrXY("^",15,0); else sendStrXY("\'",15,0);  
          #else
                setXY(15,0);if (jblks==BM_BLKSJUMP) sendChar('^'); else sendChar('\'');  
          #endif
       #endif      
      debounce(button_root);
     }
#endif
#ifdef BLOCKMODE
     if(button_down() && start==1 && pauseOn==1
                                                      #ifdef btnRoot_AS_PIVOT
                                                            && !button_root()
                                                      #endif
                                                            ){      // down block sequential search                                                           

/*
       bytesRead=11;                     // for tzx skip header(10) + GETID(11)
       currentTask=PROCESSID;
*/
/*
       bytesRead=0;                       // for both tap and tzx, no header for tap
       currentTask=GETFILEHEADER;         //First task (default): search for tzx header
*/
       #ifdef BLOCKID_INTO_MEM
         oldMinBlock = 0;
         oldMaxBlock = maxblock;
       /*  if (firstBlockPause) {           
            if (block > 1) block = block -2;
            else if (block == 1) block = maxblock;
            else block = maxblock -1;  // block == 0
            firstBlockPause = false;
         }           
         if (block < maxblock) block++;
         else block = 0;   */
         if (firstBlockPause) {
            firstBlockPause = false;
            if (block > 0) block--;
            else block = maxblock;
         } else {
            if (block < maxblock) block++;
            else block = 0;       
         }             
       #endif
       #if defined(BLOCK_EEPROM_PUT)
         oldMinBlock = 0;
         oldMaxBlock = 99;
         /* if (firstBlockPause) {
            if (block > 1) block = block -2;
            else if (block == 1) block = 99;
            else block = 98;  // block == 0
            firstBlockPause = false;
         }          
         if (block < 99) block++;
         else block = 0; */
         if (firstBlockPause) {
            firstBlockPause = false;
            if (block > 0) block--;
            else block = 99;
         } else {
            if (block < 99) block++;
            else block = 0;       
         }         
       #endif
       #if defined(BLOCKID_NOMEM_SEARCH)
        // oldMinBlock = 0;
        // oldMaxBlock = 99;
         /* if (firstBlockPause) {
            if (block > 1) block = block -2;
            else if (block == 1) block = 99;
            else block = 98;  // block == 0
            firstBlockPause = false;
         }          
         if (block < 99) block++;
         else block = 0; */
         if (firstBlockPause) {
            firstBlockPause = false;
            if (block > 0) block--;
            //else block = 0;
         } else {
            //if (block < 99) block++;
            //else block = 0;
            //if ((block +jblks) <256) block=block+jblks;               
            block = block + jblks;
    
         }         
       #endif
/*
       EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
       EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
       currentTask=PROCESSID;
       
       SetPlayBlock();
*/
       GetAndPlayBlock();    
       debouncemax(button_down);
     }
#endif
#if defined(BLOCKMODE) && defined(btnRoot_AS_PIVOT)
     if(button_down() && start==1 && pauseOn==1 && button_root()) {     // down block half-interval search

/*
       bytesRead=11;                     // for tzx skip header(10) + GETID(11)
       currentTask=PROCESSID;
*/
/*
       bytesRead=0;                       // for both tap and tzx, no header for tap
       currentTask=GETFILEHEADER;         //First task (default): search for tzx header
*/
        if (block <oldMaxBlock) {
          oldMinBlock = block;
          block = oldMinBlock + 1+ (oldMaxBlock - oldMinBlock)/2;
        } 
/*      
       EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
       EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
       currentTask=PROCESSID;
       
       SetPlayBlock();
*/
       GetAndPlayBlock();    
       debounce(button_down);
     }
#endif

     if(button_down() && start==0
                                        #ifdef btnRoot_AS_PIVOT
                                                && !button_root()
                                        #endif
                                              ){                    // down dir sequential search                                             

       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Move down a file in the directory
       scrollTime=millis()+scrollWait;
       scrollPos=0;
       downFile();
       debouncemax(button_down);
     }

#ifdef btnRoot_AS_PIVOT
     if(button_down() && start==0 && button_root()) {              // down dir half-interval search
       #if (SPLASH_SCREEN && TIMEOUT_RESET)
            timeout_reset = TIMEOUT_RESET;
       #endif
       //Move down a file in the directory
       scrollTime=millis()+scrollWait;
       scrollPos=0;
       downHalfSearchFile();
       
       debounce(button_down);
     }
#endif

     #ifndef NO_MOTOR
     if(start==1 && (oldMotorState!=motorState) && mselectMask==1 ) {  
       //if file is playing and motor control is on then handle current motor state
       //Motor control works by pulling the btnMotor pin to ground to play, and NC to stop
       if(motorState==1 && pauseOn==0) {
         printtext2F(PSTR("PAUSED  "),0);
   /*      #ifdef LCDSCREEN16x2
              lcd.setCursor(0,0);
              lcd.print(F("PAUSED "));    
         #endif 
         #ifdef OLED1306
              #ifdef XY
                setXY(0,0);
                sendStr("PAUSED ");
              #endif
              #ifdef XY2
                sendStrXY("PAUSED ",0,0);
              #endif
         #endif 
         #ifdef P8544
              lcd.setCursor(0,0);
              lcd.print(F("PAUSED "));                       
         #endif 
        */                
         scrollPos=0;
         scrollText(fileName);
         //lcd_clearline(0);
         //lcd.print(F("Paused "));         
         pauseOn = 1;
       } 
       if(motorState==0 && pauseOn==1) {
         printtext2F(PSTR("PLAYing "),0);
    /*     #ifdef LCDSCREEN16x2
              lcd.setCursor(0,0);
              lcd.print(F("PLAYing"));    
         #endif 
         #ifdef OLED1306
              #ifdef XY
                setXY(0,0);
                sendStr("PLAYing");
              #endif
              #ifdef XY2
                sendStrXY("PLAYing",0,0);
              #endif
         #endif 
         #ifdef P8544
              lcd.setCursor(0,0);
              lcd.print(F("PLAYing"));                       
         #endif
        */            
         scrollPos=0;
         scrollText(fileName);
         //lcd_clearline(0);
         //lcd.print(F("Playing"));   
         //delay(2250);
         pauseOn = 0;
       }
       oldMotorState=motorState;
     }
     #endif
  }
}

void upFile() {    
  // move up a file in the directory.
  // find prior index, using currentFile.
  oldMinFile = 0;
  oldMaxFile = maxFile;
  while(currentFile!=0)
  {
    currentFile--;
    // currentFile might not point to a valid entry (since not all values are used)
    // and we might need to go back a bit further
    // Do this here, so that we only need this logic in one place
    // and so we can make seekFile dumber
    entry.close();
    if (entry.open(&currentDir, currentFile, O_RDONLY))
    {
      entry.close();
      break;
    }
  }

  if(currentFile==0)
  {
    // seek up wrapped - should now be reset to point to the last file
    currentFile = maxFile;
  }
  seekFile();
}

void downFile() {    
  //move down a file in the directory
  oldMinFile = 0;
  oldMaxFile = maxFile;
  currentFile++;
  if (currentFile>maxFile) { currentFile=0; }
  seekFile();
}

void upHalfSearchFile() {    
  //move up to half-pos between oldMinFile and currentFile

  if (currentFile >oldMinFile) {
    oldMaxFile = currentFile;
/*
    #ifdef SHOW_DIRNAMES
      str4cpy(oldMaxFileName,fileName);
    #endif
*/
    currentFile = oldMinFile + (oldMaxFile - oldMinFile)/2;
    seekFile();
  }
}

void downHalfSearchFile() {    
  //move down to half-pos between currentFile amd oldMaxFile

  if (currentFile <oldMaxFile) {
    oldMinFile = currentFile;
/*    
    #ifdef SHOW_DIRNAMES
      str4cpy(oldMinFileName,fileName);
    #endif
*/    
    currentFile = oldMinFile + 1+ (oldMaxFile - oldMinFile)/2;
    seekFile();
  } 
}

void seekFile() {    
  //move to a set position in the directory, store the filename, and display the name on screen.
  entry.close(); // precautionary, and seems harmless if entry is already closed
  
  while (!entry.open(&currentDir, currentFile, O_RDONLY))
  {
    // if entry.open fails, when given an index, sdfat 2.1.2 automatically adjust curPosition to the next good entry
    // (which means that we can just retry calling open, passing in curPosition)
    // but sdfat 1.1.4 does not automatically adjust curPosition, so we cannot do that trick.
    // We cannot call openNext, because (with sdfat 2.1.2) the position has ALREADY been adjusted, so you will actually
    // miss a file.
    // so need to do this manually (to support both library versions), via incrementing currentFile manually
    currentFile++;
  }

  entry.getName(fileName,filenameLength);
  filesize = entry.fileSize();
  #ifdef AYPLAY
  ayblklen = filesize + 3;  // add 3 file header, data byte and chksum byte to file length
  #endif
  if(entry.isDir() || !strcmp(fileName, "ROOT")) { isDir=1; } else { isDir=0; }
  entry.close();

  PlayBytes[0]='\0'; 
  if (isDir==1) {
    if (subdir >0)strcpy(PlayBytes,prevSubDir);
    else strcat_P(PlayBytes,PSTR(VERSION));
/*
       else { itoa(oldMinFile,input,10); strcpy(PlayBytes,input);strcat(PlayBytes,"-");
       itoa(currentFile,input,10); strcat(PlayBytes,input);strcat(PlayBytes,"-");
       itoa(oldMaxFile,input,10); strcat(PlayBytes,input);}
*/
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
  }
  else if (fileName[0] == '\0') 
  {
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
  else
  {
      printtextF(PSTR("Playing"),0);
      //printtext(PlayBytes,0);
      //lcd_clearline(0);
      //lcd.print(PlayBytes);      
      scrollPos=0;
      pauseOn = 0;
      scrollText(fileName);
      currpct=100;
      lcdsegs=0;
      UniPlay();
        #ifdef P8544
          lcd.gotoRc(3,38);
          lcd.bitmap(Play, 1, 6);
        #endif      
      start=1;       
    }    
}

void getMaxFile() {    
  // gets the total files in the current directory and stores the number in maxFile
  // and also gets the file index of the last file found in this directory
  currentDir.rewind();
  maxFile = 0;
  while(entry.openNext(&currentDir, O_RDONLY)) {
    maxFile = currentDir.curPosition()/32-1;
    entry.close();
  }
  currentDir.rewind(); // precautionary but I think might be unnecessary since we're using currentFile everywhere else now
  oldMaxFile = maxFile;
}

void changeDir() {    
  // change directory (to whatever is currently the selected fileName)
  // if fileName="ROOT" then return to the root directory
                      
  if(!strcmp(fileName, "ROOT"))
  {
    subdir=0;    
    changeDirRoot();
  } else {
     //if (subdir >0) entry.cwd()->getName(prevSubDir[subdir-1],filenameLength); // Antes de cambiar
     if (subdir < nMaxPrevSubDirs) {
      DirFilePos[subdir] = currentFile;
      subdir++;
      // but, also, stash the current filename as the parent (prevSubDir) for display
      // (but we only keep one prevSubDir, we no longer need to store them all)
      fileName[SCREENSIZE] = '\0';
      strcpy(prevSubDir, fileName);
    }
    tmpdir.open(&currentDir, currentFile, O_RDONLY);
    currentDir = tmpdir;
    tmpdir.close();
  }
  getMaxFile();
  currentFile=0;
  oldMinFile=0;
  seekFile();
}

void changeDirParent()
{
  // change up to parent directory.
  // get to parent folder via re-navigating the same sequence of directories, starting at root
  // (slow-ish but, honestly, not very slow. and requires very little memory to keep a deep stack of parents
  // since you only need to store one file index, not a whole filename, for each directory level)
  // Note to self : does sdfat now support any better way of doing this e.g. is there a link back to parent like ".."  ?
  subdir--;
  uint16_t this_directory=DirFilePos[subdir]; // remember what directory we are in currently

  changeDirRoot();
  if(subdir>0)
  {
    for(int i=0; i<subdir; i++)
    {
      tmpdir.open(&currentDir, DirFilePos[i], O_RDONLY);
      currentDir = tmpdir;
      tmpdir.close();
    }
    // get the filename of the last directory we opened, because this is now the prevSubDir for display
    currentDir.getName(fileName, filenameLength);
    fileName[SCREENSIZE] = '\0'; // copy only the piece we need. small cheat here - we terminate the string where we want it to end...
    strcpy(prevSubDir, fileName);
  }
   
  getMaxFile();
  currentFile = this_directory; // select the directory we were in, as the current file in the parent
  oldMinFile=0;
  seekFile(); // don't forget that this will put the real filename back into fileName 
}

void changeDirRoot()
{
  currentDir.close();
  currentDir.open("/", O_RDONLY);                    //set SD to root directory
}

void scrollText(char* text){
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

void printtext2F(const char* text, int l) {  //Print text to screen. 
  
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
 //   for(x; x<16; x++) lcd.print(' ');  
    
  #endif

 #ifdef OLED1306
     #ifdef XY2
      strncpy_P(fline, text, 16);
    //  for(int i=strlen(fline);i<16;i++) fline[i]=0x20;
      sendStrXY(fline,0,l);
     #endif
     
     #ifdef XY 
      setXY(0,l);
      
      char x = 0;
      while (char ch=pgm_read_byte(text+x)) {
        sendChar(ch);
        x++;
      }
   //   for(x; x<16; x++) sendChar(' ');
     #endif
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
  //  for(int i=strlen(fline);i<14;i++) fline[i]=0x20;
    //lcd.setCursor(0,l);
    //lcd.print(F("              "));
    lcd.setCursor(0,l);
    lcd.print(fline);
    //lcd.print(reinterpret_cast <const __FlashStringHelper *> (text));
  #endif 
   
}

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
     #ifdef XY2
      strncpy_P(fline, text, 16);
      for(int i=strlen(fline);i<16;i++) fline[i]=0x20;
      sendStrXY(fline,0,l);
     #endif
     
     #ifdef XY 
      setXY(0,l);
      
      char x = 0;
      while (char ch=pgm_read_byte(text+x)) {
        sendChar(ch);
        x++;
      }
      for(x; x<16; x++) sendChar(' ');
     #endif
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
    #ifdef XY2
      for(int i=0;i<16;i++)
      {
        if(i<strlen(text))  fline[i]=text[i];
        else  fline[i]=0x20;
      }    
      sendStrXY(fline,0,l);
    #endif
    
    #ifdef XY
      setXY(0,l); 

      char ch;
      const char len = strlen(text);
       for(char x=0;x<16;x++)
      {
        if(x<len)  ch=text[x];
        else  ch=0x20;
        sendChar(ch);
      }       
    #endif
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
    #ifdef OLED1306_128_64
      setXY(0,7);
      //sendChar(48+BAUDRATE/1000); sendChar(48+(BAUDRATE/100)%10);sendChar(48+(BAUDRATE/10)%10);sendChar(48+BAUDRATE%10);
      itoa(BAUDRATE,(char *)input,10);sendStr((char *)input);
      setXY(5,7);
      if(mselectMask==1) sendStr(" M:ON");
      else sendStr("m:off");    
      setXY(11,7); 
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) sendStr(" %^ON");
      else sendStr("%^off");    
    #else
      setXY(0,3);
      //sendChar(48+BAUDRATE/1000); sendChar(48+(BAUDRATE/100)%10);sendChar(48+(BAUDRATE/10)%10);sendChar(48+BAUDRATE%10);
      itoa(BAUDRATE,(char *)input,10);sendStr((char *)input);
      setXY(5,3);
      if(mselectMask==1) sendStr(" M:ON");
      else sendStr("m:off");    
      setXY(11,3); 
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) sendStr(" %^ON");
      else sendStr("%^off");
    #endif
  #endif
  #ifdef XY2                        // Y with double value
    #ifdef OLED1306_128_64          // 8 rows supported
        sendStrXY("ID:   BLK:",4,4);
         
        //sendChar(48+BAUDRATE/1000); sendChar(48+(BAUDRATE/100)%10);sendChar(48+(BAUDRATE/10)%10);sendChar(48+BAUDRATE%10);
        itoa(BAUDRATE,(char *)input,10);sendStrXY((char *)input,0,6);
        
        if(mselectMask==1) sendStrXY(" M:ON",5,6);
        else sendStrXY("m:off",5,6);  
          
        if (TSXCONTROLzxpolarityUEFSWITCHPARITY == 1) sendStrXY(" %^ON",11,6);
        else sendStrXY("%^off",11,6);           
    #else
    
    #endif      
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
         lcd.print(F("BLK:"));
       //  lcd.print(block%100);lcd.print(' ');
         lcd.print(block);lcd.print(' ');
         //if (bytesRead > 0){
           lcd.print(F("ID:"));lcd.print(currentID,HEX); // Block ID en hex
    //       lcd.print(' ');lcd.print(bytesRead,HEX);

         //}
       #endif
       #ifdef OLED1306
          #ifdef XY2
              utoa(block, (char *)input, 10);
            #if defined(OLEDBLKMATCH)              
              if (block <10) {input[1]=input[0];input[0]='0';input[2]=0;}
              if (block < 100) sendStrXY((char *)input,14,4);
              else sendStrXY((char *)(input+1),14,4);
            #endif                         
              sendStrXY("BLK:",0,0);
            //  input[0]=48+(block/10)%10;input[1]=48+block%10;input[2]=0;sendStrXY((char *)input,4,0);
              sendStrXY((char *)input,4,0);//sendChar(' ');
              
              //if (bytesRead > 0){              
                if (block < 100) sendStrXY(" ID:", 6,0);
                else sendStrXY(" ID:", 7,0);
                
                if (currentID/16 < 10) input[0]=48+currentID/16;
                else input[0]=55+currentID/16;
                if (currentID%16 < 10) input[1]=48+currentID%16;
                else input[1]=55+currentID%16;
                                
                input[2]=0;
                if (block < 100) sendStrXY((char *)input,10,0);
                else sendStrXY((char *)input,11,0);
                
                //utoa(currentID,(char *)input,16);sendStrXY(strupr((char *)input),10,0); // Block ID en hex
                //sendChar(' ');utoa(bytesRead,input,16);sendStr(strupr(input));
         
              //}          
          #else
        //      setXY(11,0);sendStr(" <<>>");
        //      setXY(11,0);sendStr(" Paus"); 
              utoa(block, (char *)input, 10);
            #if defined(OLEDBLKMATCH)              
              setXY(14,2);
              if (block <10) sendChar('0');
              if (block < 100) sendStr((char *)input);
              else sendStr((char *)(input+1));
            #endif                           
              setXY(0,0);
              sendStr("BLK:");
              //input[0]=48+(block/10)%10;input[1]=48+block%10;input[2]=0;sendStr((char *)input);

              sendStr((char *)input);//sendChar(' ');
              //if (bytesRead > 0){
            //    setXY(11,0);
                sendStr(" ID:");

                if (currentID/16 < 10) input[0]=48+currentID/16;
                else input[0]=55+currentID/16;
                if (currentID%16 < 10) input[1]=48+currentID%16;
                else input[1]=55+currentID%16;
                                
                input[2]=0;sendStr((char *)input);
                
                //utoa(currentID,(char *)input,16);sendStr(strupr((char *)input)); // Block ID en hex
         //       sendChar(' ');utoa(bytesRead,input,16);sendStr(strupr(input));
           
              //}
          #endif
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
       //TZXPlay();

       //reset data block values
       //clearBuffer();
       currpct=100; 
//       newpct=(100 * bytesRead)/filesize;

       lcdsegs=0;       
       currentBit=0;                               // fallo reproducción de .tap tras .tzx
       pass=0;
              

/*
       currentBlockTask = READPARAM;               //First block task is to read in parameters
       //currentTask=PROCESSID;
       //currentID=TAP;
       currentTask=GETFILEHEADER;                  //First task: search for header
*/


//  Timer1.stop();                              //Stop timer interrupt

//  bytesRead=0;                                //start of file

  if (!casduino) {
    currentBlockTask = READPARAM;               //First block task is to read in parameters
//    clearBuffer2();                               // chick sound with CASDUINO clearBuffer()
//    isStopped=false;
//    pinState=LOW;                               //Always Start on a LOW output for simplicity
//    count = 255;                                //End of file buffer flush
//    EndOfFile=false;
//    digitalWrite(outputPin, pinState);
  #if defined(__AVR__) || defined(__SAMD21__)
    Timer1.setPeriod(1000);                     //set 1ms wait at start of a file.
  #elif defined(__arm__) && defined(__STM32F1__) 
    timer.setSTM32Period(1000);
  #else
    #error unknown timer
  #endif
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
   #ifdef BLOCKID_INTO_MEM
      bytesRead=blockOffset[block%maxblock];
      currentID=blockID[block%maxblock];   
   #endif
   #ifdef BLOCK_EEPROM_PUT
      #if defined(__AVR__)
        EEPROM.get(BLOCK_EEPROM_START+5*block, bytesRead);
        EEPROM.get(BLOCK_EEPROM_START+4+5*block, currentID);
      #elif defined(__arm__) && defined(__STM32F1__)
        EEPROM_get(BLOCK_EEPROM_START+5*block, &bytesRead);
        EEPROM_get(BLOCK_EEPROM_START+4+5*block, &currentID);
      #endif      
   #endif
   #ifdef BLOCKID_NOMEM_SEARCH 
      //block=1; //forcing block number for debugging
      
      unsigned long oldbytesRead;           //TAP
      bytesRead=0;                          //TAP 
      if (currentID!=TAP) bytesRead=10;   //TZX with blocks skip TZXHeader

      //int i=0;
      #ifdef BLKBIGSIZE
        int i = 0;
      #else
        byte i = 0;
      #endif      
      while (i<= block) {

        //if (currentID!=TAP) if(ReadByte(bytesRead)==1) currentID = outByte;  //TZX with blocks GETID
        if(ReadByte(bytesRead)==1){
          oldbytesRead = bytesRead-1;
          if (currentID!=TAP) currentID = outByte;  //TZX with blocks GETID
          if (currentID==TAP) bytesRead--;
        }
        else {block = i-1;break;}
        
        switch(currentID) {
          case ID10:  bytesRead+=2;     //Pause                
                      if(ReadWord(bytesRead)==2) bytesRead += outWord; //Length of data that follow
                      #if defined(OLEDBLKMATCH)
                        i++;
                      #endif
                      break;
          case ID11:  bytesRead+=15; //lPilot,lSynch1,lSynch2,lZ,lO,lP,LB,Pause
                      if(ReadLong(bytesRead)==3) bytesRead += outLong;
                      #if defined(OLEDBLKMATCH)
                        i++;
                      #endif                      
                      break;
          case ID12:  bytesRead+=4;
                      break;
          case ID13: if(ReadByte(bytesRead)==1) bytesRead += (long(outByte) * 2);
                      break;
          case ID14:  bytesRead+=7;
                      if(ReadLong(bytesRead)==3) bytesRead += outLong;
                      break;
          case ID15:  bytesRead+=5;
                      if(ReadLong(bytesRead)==3) bytesRead += outLong; 
                      break;
          case ID19:  if(ReadDword(bytesRead)==4) bytesRead += outLong;
                      #if defined(OLEDBLKMATCH) //&& defined(BLOCKID19_IN)
                        i++;
                      #endif          
                      break;                      
          case ID20:  bytesRead+=2;
                      break;
          case ID21:  if(ReadByte(bytesRead)==1) bytesRead += outByte;
                      #if defined(OLEDBLKMATCH) && defined(BLOCKID21_IN)
                        i++;
                      #endif          
                      break;
          case ID22:  
                      //#if defined(OLEDBLKMATCH) && defined(BLOCKID21_IN)
                      //  i++;
                      //#endif          
                      break;
          case ID24:  bytesRead+=2;
                      break;                                                                                
          case ID25:
                      break;
          case ID2A:  bytesRead+=4;
                      break;
          case ID2B:  bytesRead+=5;
                      break;
          case ID30:  if (ReadByte(bytesRead)==1) bytesRead += outByte;                                            
                      break;
          case ID31:  bytesRead+=1;         
                      if(ReadByte(bytesRead)==1) bytesRead += outByte; 
                      break;
          case ID32:  if(ReadWord(bytesRead)==2) bytesRead += outWord;
                      break;
          case ID33:  if(ReadByte(bytesRead)==1) bytesRead += (long(outByte) * 3);
                      break;
          case ID35:  bytesRead += 0x10;
                      if(ReadDword(bytesRead)==4) bytesRead += outLong;
                      break;
          case ID4B:  if(ReadDword(bytesRead)==4) bytesRead += outLong;
                      #if defined(OLEDBLKMATCH)
                        i++;
                      #endif          
                      break;
          case TAP:   if(ReadWord(bytesRead)==2) bytesRead += outWord;
                      #if defined(OLEDBLKMATCH) && defined(BLOCKTAP_IN)
                        i++;
                      #endif           
                      break;
        }
        #if not defined(OLEDBLKMATCH)
          i++;
        #endif  
      }

      //bytesRead=142; //forcing bytesRead for debugging      
      //ltoa(bytesRead,PlayBytes,16);printtext(PlayBytes,lineaxy);
              
   #endif   

   bytesRead= oldbytesRead;
   if (currentID==TAP) currentTask=PROCESSID;
   else {
    currentTask=GETID;    //Get new TZX Block
    if(ReadByte(bytesRead)==1) {currentID = outByte;currentTask=PROCESSID;}  //TZX with blocks GETID
   }
   
   SetPlayBlock(); 
}

void str4cpy (char *dest, char *src)
{
  char x =0;
  while (*(src+x) && (x<4)) {
       dest[x] = src[x];
       x++;
  }
  for(x; x<4; x++) dest[x]=' ';
  dest[4]=0; 
}

void GetFileName(uint16_t pos)
{
  entry.close(); // precautionary, and seems harmless if entry is already closed
  if (entry.open(&currentDir, pos, O_RDONLY))
  {
    entry.getName(fileName,filenameLength);
  }
  entry.close();
}
