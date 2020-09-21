////////////////                      CONFIG FOR NEW ARDUINO NANO EVERY                        ////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                 Add // at the beginning of lines to comment and remove selected option                                */
//**************************************  OPTIONAL USE TO SAVE SPACE  ***************************************************//
#define Use_MENU                          // removing menu saves space
#define AYPLAY
#define DIRECT_RECORDING
#define ZX81SPEEDUP
#define ID19REW                           // trace id19 block for zx81 .tzx to be rewinded
#define tapORIC
    #define ORICSPEEDUP
#define Use_CAS                           // .cas files playback on MSX / Dragon / CoCo Tandy computers
    #define Use_DRAGON
        #define Use_Dragon_sLeader        // short Leader of 0x55 allowed for loading TOSEC files
#define Use_UEF                           // .uef files playback on BBC Micro / Electron / Atom computers
    #define Use_c112                      // integer gap chunk for .uef
    #define Use_hqUEF                     // .hq.uef files playback on BBC Micro / Electron / Atom computers
        #define Use_c104                  // defined tape format data block: data bits per packet/parity/stop bits    
        //#define Use_c114                // security cycles replaced with carrier tone
        //#define Use_c116                // floating point gap chunk for .hq.uef
        #define Use_c117                // data encoding format change for 300 bauds
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//         UEF file instructions: UEF files are compressed and can not be executed directly in MAXDUINO,                 //
/*         for that you have to decompress these files manually.                                                         */
/*         linux / mac os: gunzip -c game.uef> game.uef.tmp && mv game.uef.tmp game.uef                                  */
/*         windows os: add .gz to file name, then click to extract with winrar                                           */
//***********************************************************************************************************************//
//                                       Set Acorn UEF default speed                                                     //
#define TURBOBAUD1500                 // default setting, 25% faster than 1200 baudios standard speed
//#define TURBOBAUD1550
//#define TURBOBAUD1600

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//** If Use_MENU commented to remove Menu, then vars need setting preference cause no EEPROM for read/write **//
int BAUDRATE = 3600;
byte scale = 2;                         // 1 for BAUDRATE 1200
int period = 70;                        // 208 for BAUDRATE=1200
byte mselectMask = 0;                   // Motor control state 1=on 0=off
byte TSXCONTROLzxpolarityUEFSWITCHPARITY = 1;  // Multiple flag: rpolarity needed for zx games: Basil the Great Mouse Detective, 
                                        //            Mask // SpeedControl for .tsx // UEF Switch Parity
byte skip2A = 0;                        // Pause on for BLK:2A
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                   Configure your screen settings here                                                  */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Set defines for various types of screen

//#define SERIALSCREEN              // For testing and debugging 

#define LCD_I2C_ADDR    0x27        // Set the i2c address of your 1602LCD usually 0x27
#define LCDSCREEN16x2               // Set if you are using a 1602 LCD screen

//#define OLED_SETCONTRAS   0xcf      // Override default value inside Diplay.ino, bigger to increase output current per segment
//#define OLED1306                      // Set if you are using OLED 1306 display
      //#define OLED1306_128_64         // 128x64 resolution with 8 rows
      //#define OLED1106_1_3            // Use this line as well if you have a 1.3" OLED screen

//#define btnRoot_AS_PIVOT
  #define SHOW_DIRPOS
      //#define SHOW_STATUS_LCD
      //#define SHOW_DIRNAMES
      
  #define SHOW_BLOCKPOS_LCD
  
#define XY                         // use original settings for Oled line 0,1 and status for menu
//#define XY2                      // use double size font wihtout status line for menu
#ifdef XY
byte lineaxy=1;
#endif
#ifdef XY2
byte lineaxy=2;
#endif

#define CNTRBASE 100                // 100 for sss, 60 for m:ss (sorry, no space for separator)
#define MAXPAUSE_PERIOD   8300         // millis
//#define MAXPAUSE_PERIOD   520         // millis  
#define OnPausePOLChg 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define SPLASH_SCREEN   1  // Displays the logo and welcome text at the initialization and remains until a button is pressed.
#define TIMEOUT_RESET   60 // Timeout for reset tzxduino (without pause or play activated), comment to not reset.
//#define BLOCK_EEPROM_PUT            // must be disabled if loading many turbo short blocks, as in Amstrad cpc demo Breaking Baud
#define BLOCKID_INTO_MEM              // enable for blockid recording and later rewinding if EEPROM_PUT is disabled.
#define maxblock 99                   // maxblock if not using EEPROM 
#define BLOCK_EEPROM_START 512
#define LOAD_EEPROM_SETTINGS
#define EEPROM_CONFIG_BYTEPOS  255     // Byte position to save configuration

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EEPROM LOGO. How to move to EEPROM, saving memory:
// Phase 1: Uncomment RECORD_EEPROM_LOGO define , this copies logo from memory to EEPROM. Compile the sketch.
// Phase 2:  Comment RECORD_EEPROM define, uncomment LOAD_EEPROM define. Complile the sketch again 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Also it's posible to select record and load both for better testing new logo activation, pressing MENU simulates a reset.
// And both can be deactivated also showing a black screen.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define COMPRESS_REPEAT_ROW
#define EEPROM_LOGO_COMPRESS
//#define LOAD_MEM_LOGO             // legacy, logo is not in EEPROM then wasting memory.
//#define RECORD_EEPROM_LOGO        // Uncommenting RECORD_EEPROM deactivates #define Use_MENU
#define LOAD_EEPROM_LOGO 

#ifdef OLED1306_128_64
  //#define Maxduino2Alf              // new Maxduino2 logo 128x64 by Alfredo Retrocant
  #define CosmicCruiser             // Dragon32 CosmicCruiser by rcmolina
#else
//#define LOGOARDUITAPE            // original arduitape logo
//#define LOGOMAXDUINO             // new logo created by Spirax
 //#define LOGOMAXDUINO2              // new logo2 created by rcmolina
//#define LOGOMAXDUINO3            // new logo3 created by rcmolina
//#define LOGOMAXDUINO4            // new logo4 created by rcmolina
//#define SUGARLESS_1              // new logo cretaed bt YoxxxoY
//#define SUGARLESS_2              // new logo created by YoxxxoY
#define LOGOMINIDUINO            // new logo for A.Villena's Miniduino by rcmolina
#endif

