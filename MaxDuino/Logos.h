#ifndef LOGOS_H_INCLUDED
#define LOGOS_H_INCLUDED

#include "configs.h"

// Multiple ways to specify your chosen logo, in order of precedence:
// 1. in userconfig.h , by defining LOGO (interpreted as a name file under logos/128x64 , logos/128x32 or logos/84x48 as appropriate)
// 2. in commandline/platformio.ini as -DLOGO
// 3. in userconfig.h , by defining LOGO_128_64, LOGO_128_32 and/or LOGO_84_48  (obviously only one will be used)
// 4. in commandline/platformio.ini as -DLOGO_128_64, -DLOGO_128_32 and/or -DLOGO_84_48
// 5. as a global default via commandline/platformio.ini as -DLOGO_128_64_DEFAULT, -DLOGO_128_32_DEFAULT, -DLOGO_84_48_DEFAULT
// 6. a global hardcoded fallback (see below) if none of the above are defined
//
// To create multiple variants via platformio.ini , option 2 is recommended (in conjunction with existing userconfig
// files that are already using option 3)

#define __LOGO_header(x) #x
#define _LOGO_header(p,x) __LOGO_header(p ## x.h)
#define LOGO_header(p,x) _LOGO_header(p,x)

#ifndef LOGO_128_64_DEFAULT
#define LOGO_128_64_DEFAULT cablemax
#endif

#ifndef LOGO_128_32_DEFAULT
#define LOGO_128_32_DEFAULT LOGOMAXDUINO
#endif

#ifndef LOGO_84_48_DEFAULT
#define LOGO_84_48_DEFAULT LOGOMAXDUINO
#endif


#ifdef OLED1306

  #if defined(OLED1306_128_64) || defined(video64text32)
    #define _LOGOPATH logos/128x64/logo_
    #ifdef LOGO
      #define _LOGO LOGO
    #elif defined(LOGO_128_64)
      #define _LOGO LOGO_128_64
    #else
      #define _LOGO LOGO_128_64_DEFAULT
    #endif

  #else

    #define _LOGOPATH logos/128x32/logo_
    #ifdef LOGO
      #define _LOGO LOGO
    #elif defined(LOGO_128_32)
      #define _LOGO LOGO_128_32
    #else
      #define _LOGO LOGO_128_32_DEFAULT
    #endif
  
  #endif // 128x64 vs 128x32

  const unsigned char logo [] PROGMEM = {
  #include LOGO_header(_LOGOPATH, _LOGO)
  };

#endif  // OLED1306

//==========================================================//

#ifdef P8544
#define _LOGOPATH logos/84x48/logo_
#define _LOGO1PATH logos/84x48/logo1_
#define _LOGO2PATH logos/84x48/logo2_
#ifdef LOGO
#define _LOGO LOGO
#elif defined(LOGO_84_48)
#define _LOGO LOGO_84_48
#else
#define _LOGO LOGO_84_48_DEFAULT
#endif

const unsigned char logo1 [] PROGMEM = {
#include LOGO_header(_LOGO1PATH, _LOGO)
}

const unsigned char logo2 [] PROGMEM = {
#include LOGO_header(_LOGO2PATH, _LOGO)
};

const unsigned char logo [] PROGMEM = {
#include LOGO_header(_LOGOPATH, _LOGO)
}

#endif // P8544

#endif // LOGOS_H_INCLUDED
