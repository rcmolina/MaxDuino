#define Use_SoftWire
#define I2CFAST

#if defined(I2CFAST)
  #define I2C_FASTMODE  1
  #define I2CCLOCK  400000L   //100000L for StandarMode, 400000L for FastMode and 1000000L for FastModePlus
#else
  #define I2C_FASTMODE  0
  #define I2CCLOCK  100000L   //100000L for StandarMode, 400000L for FastMode and 1000000L for FastModePlus
#endif

#if defined(__AVR_ATmega4809__) || defined (__AVR_ATmega4808__)
  #define SDFat
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
  #define SDFat
  //#define TimerOne  
#else  //__AVR_ATmega328P__
  #define SDFat
  //#define TimerOne
#endif

#ifdef TimerOne
  #include <TimerOne.h>
#elif defined(__arm__) && defined(__STM32F1__)
  //HardwareTimer timer(2); // channel 2
  #include "TimerCounter.h"
 
/* class TimerCounter: public HardwareTimer
{
  public:
    TimerCounter(uint8 timerNum) : HardwareTimer(timerNum) {};
    void setSTM32Period(unsigned long microseconds) __attribute__((always_inline)) {}
};*/
TimerCounter timer(2);

  #include <itoa.h>  
  #define strncpy_P(a, b, n) strncpy((a), (b), (n))
  #define memcmp_P(a, b, n) memcmp((a), (b), (n))  
#else
  #include "TimerCounter.h"
  TimerCounter Timer1;              // preinstatiate
  
  unsigned short TimerCounter::pwmPeriod = 0;
  unsigned char TimerCounter::clockSelectBits = 0;
  void (*TimerCounter::isrCallback)() = NULL;
  
  // interrupt service routine that wraps a user defined function supplied by attachInterrupt
  #if defined(__AVR_ATmega4809__) || defined (__AVR_ATmega4808__)
    ISR(TCA0_OVF_vect)
    {
      Timer1.isrCallback();
    /* The interrupt flag has to be cleared manually */
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
    }  
  #else //__AVR_ATmega328P__
    ISR(TIMER1_OVF_vect)
    {
      Timer1.isrCallback();
    }
  #endif
#endif

#ifdef SDFat
  #include <SdFat.h>
#else
  #include <SD.h>
  #define SdFile File
  #define SdFat SDClass
  #define chdir open
  #define openNext openNextFile
  #define isDir() isDirectory()
  #define fileSize size
  #define seekSet seek
  File cwdentry;
#endif

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
  #ifdef Use_SoftWire
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
