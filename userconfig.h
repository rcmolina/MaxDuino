//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                    Add // at the beginning of lines to comment and remove selected option                    */
//*********************************  OPTIONAL USE TO SAVE SPACE  ***********************************************//
#define Use_MENU 1                  // removing menu saves space
#define Use_CAS 1                   // .cas files playback on MSX / Dragon / CoCo Tandy computers
#define Use_UEF 1                   // .uef files playback on BBC Micro / Electron / Atom computers
    //#define Use_c112 1            // integer gap chunk for .uef
    #define Use_hqUEF 1             // .hq.uef files playback on BBC Micro / Electron / Atom computers
        //#define Use_c116 1        // floating point gap chunk for .hq.uef
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//         UEF file instructions: UEF files are compressed and can not be executed directly in MAXDUINO,       //
/*         for that you have to decompress these files manually.                                               */
/*         linux / mac os: gunzip -c game.uef> game.uef.tmp && mv game.uef.tmp game.uef                        */
/*         windows os: add .gz to file name, then click to extract with winrar                                 */
//*************************************************************************************************************//
//                                       Set Acorn UEF default speed                                           //
//#define STDBAUD1200               // Standard acorn speed
#define TURBOBAUD1500               // default setting
//#define TURBOBAUD1550
//#define TURBOBAUD1600
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//** If Use_MENU commented to remove Menu, then vars need setting preference cause no EEPROM for read/write **//
int BAUDRATE = 3600;
byte scale = 2;                     // 1 for BAUDRATE 1200
int period = 70;                    // 208 for BAUDRATE=1200
byte mselectMask = 0;               // Motor control state 1=on 0=off
byte tsxSPEEDzxPOL = 1;             // Dual flag: rpolarity needed for zx games: Basil the Great Mouse Detective, Mask, 
                                    //            .. and SpeedControl for .tsx
byte skip2A = 0;                    // Pause on for BLK:2A
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                   Configure your screen settings here                                       */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Set defines for various types of screen, currently only 16x2 I2C LCD is supported
//#define SERIALSCREEN  1           // For testing and debugging

#define LCD_I2C_ADDR    0x3f        // Set the i2c address of your 1602LCD usually 0x27 or 0x3f*/
//#define LCDSCREEN16x2   1         // Set if you are using a 1602 LCD screen

#define OLED1306   1                // Set if you are using OLED 1306 display
//#define P8544   1                 // Set if you are Display Nokia 5110 display

#define CNTRBASE 100                // 100 for sss, 60 for m:ss
//#define  cntrMSS                  // if not defined use sss format, if defined m:ss

#define XY  1                       // use original settings for Oled line 0,1 and status for menu
//#define XY2 1                     // use double size font wihtout status line for menu
#ifdef XY
byte lineaxy=1;
#endif
#ifdef XY2
byte lineaxy=2;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EEPROM LOGO. How to move to EEPROM, saving memory:
// Phase 1: Uncomment RECORD_EEPROM_LOGO define , this copies logo from memory to EEPROM. Compile the sketh.
// Phase 2:  Comment RECORD_EEPROM define, uncomment LOAD_EEPROM define. Complile the sketch again 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define LOAD_MEM_LOGO 1  // legacy, logo is not in EEPROM then wasting memory.
#define BLOCK_EEPROM_START 512
//#define RECORD_EEPROM_LOGO 1
#define LOAD_EEPROM_LOGO 1

//#define LOGOARDUITAPE 1           // original logo from arduitape
//#define LOGOMAXDUINO 1            // new logo made by Spirax
#define LOGOMAXDUINO2 1             // new logo2 made by rcmolina
//#define LOGOMAXDUINO3 1           // new logo3 made by rcmolina
//#define LOGOMAXDUINO4 1           // new logo4 made by rcmolina

