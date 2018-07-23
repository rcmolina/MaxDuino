////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                          Add // at the beginning of lines to comment and remove selected option                        */
//*******************************************  OPTIONAL USE TO SAVE SPACE  ***********************************************//
#define Use_MENU                           // removing menu saves space
#define Use_CAS                           // .cas files playback on MSX / Dragon / CoCo Tandy computers
#define Use_UEF                           // .uef files playback on BBC Micro / Electron / Atom computers
    //#define Use_c112                    // integer gap chunk for .uef
    #define Use_hqUEF                     // .hq.uef files playback on BBC Micro / Electron / Atom computers
        //#define Use_c116                // floating point gap chunk for .hq.uef
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
byte TSXCONTROLzxpolarityUEFTURBO = 1;  // Multiple flag: rpolarity needed for zx games: Basil the Great Mouse Detective, 
                                        //            Mask // SpeedControl for .tsx // UEF turbo mode
byte skip2A = 0;                        // Pause on for BLK:2A
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                   Configure your screen settings here                                                  */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Set defines for various types of screen, currently only 16x2 I2C LCD is supported
//#define SERIALSCREEN              // For testing and debugging

#define LCD_I2C_ADDR    0x3f        // Set the i2c address of your 1602LCD usually 0x27 or 0x3f
//#define LCDSCREEN16x2             // Set if you are using a 1602 LCD screen

#define OLED1306                    // Set if you are using OLED 1306 display
    #define OLED1106_1.3              // Use this line as well if you have a 1.3" OLED screen but using 128x32 and not 128x64
//#define P8544                     // Set if you are Display Nokia 5110 display

#define CNTRBASE 100                // 100 for sss, 60 for m:ss
//#define  cntrMSS                  // if not defined use sss format, if defined m:ss

#define XY                         // use original settings for Oled line 0,1 and status for menu
//#define XY2                      // use double size font wihtout status line for menu
#ifdef XY
byte lineaxy=1;
#endif
#ifdef XY2
byte lineaxy=2;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define SPLASH_SCREEN   1  // Displays the logo and welcome text at the initialization and remains until a button is pressed.
#define TIMEOUT_RESET   60 // Timeout for reset tzxduino (without pause or play activated), comment to not reset.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EEPROM LOGO. How to move to EEPROM, saving memory:
// Phase 1: Uncomment RECORD_EEPROM_LOGO define , this copies logo from memory to EEPROM. Compile the sketh.
// Phase 2:  Comment RECORD_EEPROM define, uncomment LOAD_EEPROM define. Complile the sketch again 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define LOAD_MEM_LOGO             // legacy, logo is not in EEPROM then wasting memory.
#define BLOCK_EEPROM_START 512
//#define RECORD_EEPROM_LOGO 
#define LOAD_EEPROM_LOGO 

//#define LOGOARDUITAPE            // original arduitape logo
//#define LOGOMAXDUINO             // new logo created by Spirax
#define LOGOMAXDUINO2              // new logo2 created by rcmolina
//#define LOGOMAXDUINO3            // new logo3 created by rcmolina
//#define LOGOMAXDUINO4            // new logo4 created by rcmolina
//#define SUGARLESS_1              // new logo cretaed bt YoxxxoY
//#define SUGARLESS_2              // new logo created by YoxxxoY

