#ifndef FONTS_H_INCLUDED
#define FONTS_H_INCLUDED

#include "configs.h"

// Multiple ways to specify your chosen font, in order of precedence:
// 1. in userconfig.h , by defining FONT (interpreted as either 8x8 or 8x16 as required)
// 2. in commandline/platformio.ini as -DFONT
// 3. in userconfig.h , by defining FONT8x8 and/or FONT8x16 (only one will be used, depending on #ifdef DoubleFont)
// 4. in commandline/platformio.ini as -DFONT8x8 and/or -DFONT8x16
// 5. as a global default via commandline/platformio.ini as -DFONT8X8_DEFAULT and/or -DFONT8x16_DEFAULT
// 6. a global hardcoded fallback (see below) if none of the above are defined
//
// To create multiple variants via platformio.ini , option 2 is recommended (in conjunction with existing userconfig
// files that are already using option 3)

#define __FONT_header(x) #x
#define _FONT_header(p,x) __FONT_header(p ## x.h)
#define FONT_header(p,x) _FONT_header(p,x)

#ifndef FONT8x16_DEFAULT
#define FONT8x16_DEFAULT cartoon8x16
#endif

#ifndef FONT8x8_DEFAULT
#define FONT8x8_DEFAULT cartoonFont
#endif

#ifdef OLED1306

#ifdef DoubleFont
  #define _FONTHEIGHT 16
  #define _FONTPATH fonts/8x16/font_
  #ifdef FONT
    #define _FONT FONT
  #elif defined(FONT8x16)
    #define _FONT FONT8x16
  #else
    #define _FONT FONT8x16_DEFAULT
  #endif
#else
  #define _FONTHEIGHT 8
  #define _FONTPATH fonts/8x8/font_
  #ifdef FONT
    #define _FONT FONT
  #elif defined(FONT8x8)
    #define _FONT FONT8x8
  #else
    #define _FONT FONT8x8_DEFAULT
  #endif
#endif

const unsigned char myFont[][_FONTHEIGHT] PROGMEM = {
#include FONT_header(_FONTPATH, _FONT)
};

#endif  // OLED1306

#endif // FONTS_H_INCLUDED
