#define Use_SoftI2CMaster
//#define Use_SoftWire
#define I2CFAST

#if defined(I2CFAST)
  #define I2C_FASTMODE  1
  #define I2CCLOCK  400000L   //100000L for StandarMode, 400000L for FastMode and 1000000L for FastModePlus
#else
  #define I2C_FASTMODE  0
  #define I2CCLOCK  100000L   //100000L for StandarMode, 400000L for FastMode and 1000000L for FastModePlus
#endif

// default SPI clock speed
#ifndef SD_SPI_CLOCK_SPEED
  #if defined(ESP32) || defined(ESP8266)
    #ifndef SD_SPI_CLOCK_SPEED
      #define SD_SPI_CLOCK_SPEED SD_SCK_MHZ(4)
    #endif
  #endif

  #ifndef SD_SPI_CLOCK_SPEED
    #define SD_SPI_CLOCK_SPEED SPI_FULL_SPEED
  #endif
#endif

#if defined(__AVR_ATmega4809__) || defined (__AVR_ATmega4808__)
  #ifdef Use_SoftI2CMaster
    #undef Use_SoftI2CMaster
    //#error This chip does not support SoftI2CMaster. Please undefine Use_SoftI2CMaster
  #endif
  #ifdef Use_SoftWire
    #undef Use_SoftWire
    //#error This chip does not support Softwire. Please undefine Use_SoftWire
  #endif  
  
                          //
                          // PATCH PROCERURE NEEDED FOR NANO EVERY AND THINARY TO SWAP TIMERS AND AVOID HANGING
                          //
                          // In C:\Users\Rafael\AppData\Local\Arduino15\packages\arduino\hardware\megaavr\1.8.4
                          // change .\variants\nona4809\timers.h , .\variants\nona4809\variant.c , .\cores\arduino\wiring.c
                          //
                          // In C:\Users\Rafael\AppData\Local\Arduino15\packages\thinary\hardware\avr\1.0.0
                          // change .\variants\nona4808\timers.h , .\variants\nona4808\variant.c , .\cores\arduino\wiring.c
                          //
                          // Needs 2 patches, check your version:
                          //
                          // SDFat 20150201 (old and very slow):
                          // 1. In SdFatConfig.h change line 84 #define SD_SPI_CONFIGURATION 0
                          //    with: #define SD_SPI_CONFIGURATION 1
                          // 2. In SdSpi.h change line 292 #ifdef __AVR__
                          //    with: #if defined(__AVR__) && not defined(__AVR_ATmega4809__)&& not defined(__AVR_ATmega4808__)
                          //
                          // SDFat 1.1.0 (faster and recommended):
                          // 1. In SdFatConfig.h change line 216 #elif defined(__AVR__)\
                          //    with: #elif defined(__AVR__) && not defined(__AVR_ATmega4809__)&& not defined(__AVR_ATmega4808__)\
                          //
                          // 2. In SpiDriver/SdSpiDriver.h change line 374 #ifdef __AVR__
                          //    with: #if defined(__AVR__) && not defined(__AVR_ATmega4809__)&& not defined(__AVR_ATmega4808__)
                         
  //#define TimerOne 
#elif defined(__arm__) && defined(__STM32F1__)
  #ifdef Use_SoftI2CMaster
    #undef Use_SoftI2CMaster
  #endif
  #ifdef Use_SoftWire
    #undef Use_SoftWire
  #endif
     
  //#define TimerOne  
#elif defined(__SAMD21__) || defined(ESP32) || defined(ESP8266)
  #ifdef Use_SoftI2CMaster
    #undef Use_SoftI2CMaster
  #endif
  #ifdef Use_SoftWire
    #undef Use_SoftWire
  #endif

#elif defined(__AVR_ATmega32U4__)
//#undef Use_SoftI2CMaster
//#undef Use_SoftWire
//#undef I2CFAST

#elif defined(__AVR_ATmega328P__)
  //#define TimerOne

#else
  #error I2C definitions (SoftI2CMaster/SoftWire/etc) not defined for board
#endif


#ifdef TimerOne
  #include <TimerOne.h>
  const TimerOne &Timer = Timerl;
#else
  #include "TimerCounter.h"
#endif


#if defined(__arm__) && defined(__STM32F1__) 
  #include <itoa.h>  
  #define strncpy_P(a, b, n) strncpy((a), (b), (n))
  #define memcmp_P(a, b, n) memcmp((a), (b), (n)) 
  #define strcasecmp_P(a,b) strcasecmp((a), (b)) 
#endif


#include <SdFat.h>

#define scrollSpeed   250           //text scroll delay
#define scrollWait    3000          //Delay before scrolling starts

#ifdef LCDSCREEN16x2
  //#include <Wire.h>
  #include "LiquidCrystal_I2C_Soft.h"
  LiquidCrystal_I2C lcd(LCD_I2C_ADDR,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
  char indicators[] = {'|', '/', '-',0};
  uint8_t SpecialChar [8]= { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 };
  #define SCREENSIZE 16  

#elif defined(OLED1306)
  #if defined(Use_SoftI2CMaster) && defined(__AVR_ATmega2560__) 
    #define SDA_PORT PORTD
    #define SDA_PIN 1 
    #define SCL_PORT PORTD
    #define SCL_PIN 0 
    #include <SoftI2CMaster.h>         
  #elif defined(Use_SoftI2CMaster) && defined(__AVR_ATmega32U4__) 
    #define SDA_PORT PORTD
    #define SDA_PIN 1 
    #define SCL_PORT PORTD
    #define SCL_PIN 0 
    #include <SoftI2CMaster.h> 
  #elif defined(Use_SoftI2CMaster) 
    #define SDA_PORT PORTC
    #define SDA_PIN 4 
    #define SCL_PORT PORTC
    #define SCL_PIN 5 
    #include <SoftI2CMaster.h>        
  #elif defined(Use_SoftWire) && defined(__AVR_ATmega2560__) 
    #define SDA_PORT PORTD
    #define SDA_PIN 1 
    #define SCL_PORT PORTD
    #define SCL_PIN 0 
    #include <SoftWire.h>
    SoftWire Wire = SoftWire();
   #elif defined(Use_SoftI2CMaster) && defined(__AVR_ATmega32U4__) 
    #define SDA_PORT PORTD
    #define SDA_PIN 1 
    #define SCL_PORT PORTD
    #define SCL_PIN 0 
    #include <SoftWire.h>
    SoftWire Wire = SoftWire();  
  #elif defined(Use_SoftWire) 
    #define SDA_PORT PORTC
    #define SDA_PIN 4 
    #define SCL_PORT PORTC
    #define SCL_PIN 5 
    #include <SoftWire.h>
    SoftWire Wire = SoftWire();  
  #else
    #include <Wire.h>
  #endif
  //#include "U8glib.h"
  //U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 
  //char line0[17];
  //char line1[17];
  char indicators[] = {'|', '/', '-',92};
  #define SCREENSIZE 16  
#elif defined(P8544)
  #include <pcd8544.h>
  #define ADMAX 1023
  #define ADPIN 0
  #include <avr/pgmspace.h>
  byte dc_pin = 5;
  byte reset_pin = 3;
  byte cs_pin = 4;
  pcd8544 lcd(dc_pin, reset_pin, cs_pin);
  #define backlight_pin 2
  char indicators[] = {'|', '/', '-',0};
  uint8_t SpecialChar [8]= { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 };
  #define SCREENSIZE 14
  
  const byte Play [] PROGMEM = {
    0x00, 0x7f, 0x3e, 0x1c, 0x08, 0x00, 0x00
  };
  const byte Paused [] PROGMEM = {
    0x00, 0x7f, 0x7f, 0x00, 0x7f, 0x7f, 0x00
  };
  const byte Stop [] PROGMEM = {
    0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e
  };
#else
  #define SCREENSIZE 16
#endif
