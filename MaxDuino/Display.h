#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

#include "configs.h"

#ifdef LCDSCREEN16x2
  #define SCREENSIZE 16  
  #include "LiquidCrystal_I2C_Soft.h"
  extern LiquidCrystal_I2C lcd;

#elif defined(OLED1306)
  #define SCREENSIZE 16
  void sendcommand(unsigned char com);
  void SendByte(unsigned char data);
  void sendChar(unsigned char data);
  void setXY(unsigned char col,unsigned char row);
  void sendStr(const char *string);
  void sendStrXY(const char *string, int X, int Y);
  void reset_display(void);
  void displayOn(void);
  void displayOff(void);
  void clear_display(void);
  void init_OLED(void);

  #if defined(XY2) && not defined(DoubleFont)
    PROGMEM const byte DFONT[16] = { 0x00, 0x03, 0x0C, 0x0F, 0x30, 0x33, 0x3C, 0x3F, 0xC0, 0xC3, 0xCC, 0xCF, 0xF0, 0xF3, 0xFC, 0xFF };
  #endif

  #ifndef lineaxy
    #if defined(XY)
      #define lineaxy 1
    #elif defined(XY2)
      #define lineaxy 2
    #endif
  #endif


#elif defined(P8544)
  #define SCREENSIZE 14
  void bitmap2(uint8_t bdata[], uint8_t rows, uint8_t columns);
  void P8544_splash (void);

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
  // default SCREENSIZE e.g. for Serial
  #define SCREENSIZE 16
#endif // defined(P8544)

void scrollText(char* text, bool is_dir);
void scrollText(char* text, bool is_dir, byte scroll_pos);
void scrollTextReset();
void printtext2F(const char* text, int l);
void printtextF(const char* text, int l);
void printtext(char* text, int l);

#if defined(OLED1306)
void OledStatusLine();
extern PROGMEM const byte HEX_CHAR[];
#endif

#endif // DISPLAY_H_INCLUDED
