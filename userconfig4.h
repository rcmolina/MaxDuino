////////////////                CONFIG FOR CLASSIC ARDUINO NANO OR PRO MINI                  //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                 Add // at the beginning of lines to comment and remove selected option                                */
//**************************************  OPTIONAL USE TO SAVE SPACE  ***************************************************//
#define Use_MENU                          // removing menu saves space
//#define AYPLAY
#define MenuBLK2A
#define DIRECT_RECORDING
#define ZX81SPEEDUP
//#define ID19REW                           // trace id19 block for zx81 .tzx to be rewinded
#define tapORIC
    //#define ORICSPEEDUP
#define Use_CAS                           // .cas files playback on MSX / Dragon / CoCo Tandy computers
    //#define Use_DRAGON
        #define Use_Dragon_sLeader        // short Leader of 0x55 allowed for loading TOSEC files
            //#define Expand_All            // Expand short Leaders in ALL file header blocks. 
//#define Use_UEF                           // .uef files playback on BBC Micro / Electron / Atom computers
    #define Use_c112                      // integer gap chunk for .uef
    #define Use_hqUEF                     // .hq.uef files playback on BBC Micro / Electron / Atom computers
        #define Use_c104                  // defined tape format data block: data bits per packet/parity/stop bits    
        //#define Use_c114                // security cycles replaced with carrier tone
        //#define Use_c116                // floating point gap chunk for .hq.uef
        //#define Use_c117                // data encoding format change for 300 bauds
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
byte mselectMask = 1;                   // Motor control state 1=on 0=off
byte TSXCONTROLzxpolarityUEFSWITCHPARITY = 1;  // Multiple flag: rpolarity needed for zx games: Basil the Great Mouse Detective, 
                                        //            Mask // SpeedControl for .tsx // UEF Switch Parity
byte skip2A = 0;                        // Pause on for BLK:2A
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                   Configure your screen settings here                                                  */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Set defines for various types of screen

//#define SERIALSCREEN              // For testing and debugging

#define LCD_I2C_ADDR    0x27        // Set the i2c address of your 1602LCD usually 0x27
//#define LCD_I2C_ADDR    0x3f        // Set the i2c address of your 1602LCD usually 0x3f
//#define LCDSCREEN16x2             // Set if you are using a 1602 LCD screen

//#define OLED_SETCONTRAS   0xcf      // Override default value inside Diplay.ino, bigger to increase output current per segment
#define OLED_ROTATE180
#define OLED_address   0x3C           //0x3C or 0x3D
#define OLED1306                      // Set if you are using OLED 1306 display
    //#define OLED1306_128_64         // 128x64 resolution with 8 rows
    //#define OLED1106_1_3            // Use this line as well if you have a 1.3" OLED screen
    //#define video64text32
//#define P8544                       // Set if you are Display Nokia 5110 display

//#define btnRoot_AS_PIVOT
  #define SHOW_DIRPOS
      //#define SHOW_STATUS_LCD
      //#define SHOW_DIRNAMES
      
  #define SHOW_BLOCKPOS_LCD
  
#define XY                         // use original settings for Oled line 0,1 and status for menu
//#define XY2                      // use double size font wihtout status line for menu
//#define XY2force                    // Use with care: delay interrupts and crash with other options, needs I2CFAST
    //#define XY2shadedplay 
#ifdef XY
byte lineaxy=1;
#endif
#ifdef XY2
byte lineaxy=2;
#endif

#define SHOW_CNTR
#define SHOW_PCT
#define CNTRBASE 100                // 100 for sss, 60 for m:ss (sorry, no space for separator)
#define MAXPAUSE_PERIOD   8191      // millis, max 8191

//#define ONPAUSE_POLCHG               //
#define BLOCKMODE                   // REW or FF a block when in pause and Play to select it
#define BLKSJUMPwithROOT            // use menu button in pause mode to switch blocks to jump
#define BM_BLKSJUMP 20               // when menu pressed in pause mode, how may blocks to jump with REW OR FF
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define SPLASH_SCREEN   1  // Displays the logo and welcome text at the initialization and remains until a button is pressed.
#define TIMEOUT_RESET   60 // Timeout for reset tzxduino (without pause or play activated), comment to not reset.
//#define BLOCK_EEPROM_PUT            // must be disabled if loading many turbo short blocks, as in Amstrad cpc demo Breaking Baud
//#define BLOCKID_INTO_MEM              // enable for blockid recording and later rewinding if EEPROM_PUT is disabled.
#define BLOCKID_NOMEM_SEARCH          // Loop and search for a block
#define maxblock 19                   // maxblock if not using EEPROM
//#define BLOCKID21_IN
#define BLOCKTAP_IN
#define OLEDPRINTBLOCK 
#define BLOCK_EEPROM_START 512
#define LOAD_EEPROM_SETTINGS
#define EEPROM_CONFIG_BYTEPOS  1023     // Byte position to save configuration

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EEPROM LOGO. How to move to EEPROM, saving memory:
// Phase 1: Uncomment RECORD_EEPROM_LOGO define , this copies logo from memory to EEPROM. Compile the sketch.
// Phase 2:  Comment RECORD_EEPROM define, uncomment LOAD_EEPROM define. Complile the sketch again 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Also it's posible to select record and load both for better testing new logo activation, pressing MENU simulates a reset.
// And both can be deactivated also showing a black screen.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define COMPRESS_REPEAT_ROW
//#define EEPROM_LOGO_COMPRESS
//#define LOAD_MEM_LOGO             // legacy, logo is not in EEPROM then wasting memory.
//#define RECORD_EEPROM_LOGO        // Uncommenting RECORD_EEPROM deactivates #define Use_MENU
//#define LOAD_EEPROM_LOGO 

#if defined(OLED1306_128_64) || defined(video64text32)
  //#define EmptyLogo               // to erase
  //#define Maxduino2Alf            // new Maxduino2 logo 128x64 by Alfredo Retrocant
  //#define CosmicCruiser             // Dragon32 CosmicCruiser by rcmolina
  //#define Batman
  //#define Superman
  //#define Spider
  //#define Arduino
  //#define Audi
  //#define Apple
  //#define Nike
  //#define WWF
  //#define GNU
  //#define LinuxGnu
  //#define Bentley
  //#define dragon
  //#define heckbert
  //#define cassette
  //#define Audiocable
  //#define Digital
  //#define dragon32Max
  //#define Maxduino7
  //#define Maxduino14
  //#define Maxduino15
  //#define Maxduino19
  //#define Maxduino37
  //#define Maxduino42
  //#define Horse
  //#define Cat
  //#define chimpeyes
  //#define buda
  //#define AbuSimbel
  //#define Penetrat
  //#define Goody1
  //#define Goody2
  //#define Goody3
  //#define GameOver
  //#define Dizzy2017
  //#define Sega
  //#define ParisToDakar
  //#define BatmanTheMovie
  //#define Phantomas
  //#define Castle
  //#define Marilyn
  //#define LadyGaga
  //#define lionface
  //#define MichaelJackson
  //#define CheGuevara
  //#define yinyang
  //#define OLEDmeter
  //#define fuelstation
  //#define tanque2
  //#define tanque3
  //#define tanque4
  //#define tanque1
  //#define dostanques
  #define cablemax
  //#define sony
  //#define miniduino  
#else
//#define LOGOARDUITAPE            // original arduitape logo
#define LOGOMAXDUINO             // new logo created by Spirax
//#define LOGOMAXDUINO2              // new logo2 created by rcmolina
//#define LOGOMAXDUINO3            // new logo3 created by rcmolina
//#define LOGOMAXDUINO4            // new logo4 created by rcmolina
//#define SUGARLESS_1              // new logo cretaed bt YoxxxoY
//#define SUGARLESS_2              // new logo created by YoxxxoY
//#define LOGOMINIDUINO            // new logo for A.Villena's Miniduino by rcmolina
#endif
/////////////////////
//      FONTS      //
/////////////////////
//#define DoubleFont

#ifdef DoubleFont
//#define Retro8x16
#define Led8x16
//#define rconsole8x16
//#define symon8x16
//#define uniscr8x16
//#define atariST8x16
#else
//#define BasicFont
//#define BoldFont
//#define c64Font
//#define zxFont
//#define borderFont
#define cartoonFont
//#define CGAFont
//#define ComputerFont
//#define MilitaryFont
//#define MSXFont
//#define RoundFont
//#define minimalFont
//#define DigitalFont
//#define TinyFont
//#define BroadwayFont
//#define VeniceFont
//#define MagneticFont
//#define CinemaFont
//#define BubblegumFont
//#define StarsFont
//#define PreludeFont
//#define EgyptianFont
//#define LapseFont
//#define TentacleFont
//#define ChicagoFont
//#define GenevaMonoFont
//#define PAW17Font
//#define AmstradCPCFont
//#define SpectrumFont
//#define SinclairOutlineFont
//#define LittleShadowFont
//#define ClairsysFont
#endif
