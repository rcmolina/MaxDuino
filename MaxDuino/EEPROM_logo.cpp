#include "configs.h"
#include "EEPROM_logo.h"
#include "EEPROM_wrappers.h"

#ifdef RECORD_EEPROM_LOGO
// This build variant will just write the 'hardcoded' logo to eeprom
// to save firmware space; and you can then flash a standard image configured
// to load the image from eeprom
void write_logo_to_eeprom()
{
#if defined(OLED1306_128_64) || defined(video64text32)
  // logo is 128 x 64
  for(int j=0;j<8;j++)
#else
  // logo is 128 x 32
  for(int j=0;j<4;j++)
#endif
  {
    for(int i=0;i<128;i++)     // show 128* 32 Logo
    {
#if not defined(EEPROM_LOGO_COMPRESS)
      EEPROM_put(j*128+i, pgm_read_byte(logo+j*128+i));
#else
      if (i%2 == 0)
      {
      #ifdef OLED1306_128_64
        if (j%2 == 0)
        {
          byte nl=0;
          byte rnl=0;
          byte nb=0;
          rnl = pgm_read_byte(logo+j*128+i);
          for(nb=0;nb<4;nb++) {
            if (bitRead (rnl,nb*2)) {
              nl |= (1 << nb);
            }
          }
          byte nh=0;
          byte rnh=0;
          byte nc=0;
          rnh = pgm_read_byte(logo+(j+1)*128+i);
          for(nc=0;nc<4;nc++) {
            if (bitRead (rnh,nc*2)) {
              nh |= (1 << nc);
            }
          }

          EEPROM_put((j/2)*64+i/2,nl+nh*16);
        } 
      #else
        // no compression for 128x32 logos. sorry.
        EEPROM_put(j*64+i/2, pgm_read_byte(logo+j*128+i));
      #endif
      }
    #endif 
    }
  }
}        

#elif defined(RECORD_EEPROM_LOGO_FROM_SDCARD)
// This build variant will let you browse bitmaps and select one to view it and write to eeprom
#include "file_utils.h"
#include "Display.h"
#include "buttons.h"

bool handle_load_logo_file()
{
  // gets called when user clicks on a file
  // check to see if it is a logo file (bitmap, etc)
  // returns true if this was a logo file (regardless of whether it was written to eeprom or not)
  // so that it doesn't get handled like an audio file

  if (strcasecmp_P(filenameExt, PSTR("bmp")) != 0)
    return false; // not a .bmp file

// #if defined(OLED1306_128_64) || defined(video64text32)
//   // 128x64
//   if ((!strcasecmp_P(filenameExt, PSTR("bmp"))) && (filesize==1086))
// #else
//   // 128x32
//   if ((!strcasecmp_P(filenameExt, PSTR("bmp"))) && (filesize==574))
// #endif
//   {
//     readfile(2, 0);
//     if (filebuffer[0] == 'B' && filebuffer[1] == 'M') {
//       read_display_sdcard_logo();
//       return true;
//     }
//   }

//   return false;
  byte invert = 0;
  clear_display();
  read_display_sdcard_logo(invert, false);
  while(!button_stop() || lastbtn)
  {
    if (button_play())
    {
      printtextF(PSTR("WRITING EEPROM.."),0);
      read_display_sdcard_logo(invert, true);
      printtextF(PSTR("SAVED TO EEPROM"),0);
      delay(1500);
      return true;
    }
    if (button_up())
    {
      invert ^= 0xff;
      read_display_sdcard_logo(invert, false);
    }
    checkLastButton();
  }
  debounce(button_stop);
  return true;
}

void read_display_sdcard_logo(byte invert, bool eeprom_write)
{
  // this is a whole separate event loop, like Menu.
  // we read the bitmap and display it, and then wait for a key instruction to either write it
  // to EEPROM, or return to file browser
  #if defined(OLED1306_128_64) || defined(video64text32)
    const byte J = 8;  // 8 LINES
  #else
    const byte J = 4;  // 4 LINES
  #endif
  for(byte j=0;j<J;j++) 
  { 
    setXY(0,j);    
    {
      for(byte i=0;i<128;i++)
      {
        byte v = 0;
        for(byte bit=0; bit<8; bit++)
        {
          readfile(1, 0x3E + (i/8) + (128/8)*bit + 128*(J-j));
          v <<= 1;
          v += (filebuffer[0] >> (7-(i & 7))) & 0x01;
        }
        v ^= invert;
        if (eeprom_write)
        {
          EEPROM_put(j*128+i, v);
        }
        else
        {
          SendByte(v);
        }
      }
    }
  }
}

#endif

#ifdef LOAD_EEPROM_LOGO
#ifdef EEPROM_LOGO_COMPRESS
void EEPROM_get_compressed(int i, int j)
{
  if (i%2 == 0)
  {
    t=0;
  #ifdef OLED1306_128_64
    if (j%2 == 0) {
      byte ril=0;
      byte ib=0;
      EEPROM_get((j/2)*64+i/2, ril);

      for(ib=0;ib<4;ib++)
      {
        if (bitRead (ril,ib))
        {
          t |= (1 << ib*2);
        #ifdef COMPRESS_REPEAT_ROW
          t |= (1 << (ib*2)+1);
        #endif
        }
      }
    } else {
      byte rih=0;
      byte ic=0;
      EEPROM_get((j/2)*64+i/2, rih);

      for(ic=4;ic<8;ic++)
      {
        if (bitRead (rih,ic))
        {
          t |= (1 << (ic-4)*2);
        #ifdef COMPRESS_REPEAT_ROW
          t |= (1 << ((ic-4)*2)+1);
        #endif
        }
      }
    }
  #else
    // no compression for 128x32 logos. sorry.
    EEPROM_get(j*64+i/2, t);
  #endif
  }
}

}
#endif
#endif
