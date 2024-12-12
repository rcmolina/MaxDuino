#include "configs.h"
#include "compat.h"
#include "Arduino.h"
#include "Fonts.h"
#include "Logos.h"
#include "Display.h"
#include "i2c.h"
#include "preferences.h"
#include "current_settings.h"

#if defined(RECORD_EEPROM_LOGO) || defined(LOAD_EEPROM_LOGO)
#include "EEPROM_wrappers.h"
#endif

#ifdef LCDSCREEN16x2

  LiquidCrystal_I2C lcd(LCD_I2C_ADDR,16,2); // set the LCD address, and configure for a 16 chars and 2 line display

#elif defined(OLED1306)

  PROGMEM const byte HEX_CHAR[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  #if defined(LOAD_EEPROM_LOGO)
    #include "EEPROM_wrappers.h"
  #endif

  //==========================================================//
  // Used to send commands to the display.
  void sendcommand(unsigned char com)
  {
    mx_i2c_start(OLED_address); //begin transmitting
    mx_i2c_write(0x80); //command mode
    mx_i2c_write(com);
    mx_i2c_end(); // stop transmitting
  }

  //==========================================================//
  // Actually this sends a byte, not a char to draw in the display.
  // Display's chars uses 8 byte font
  void SendByte(unsigned char data)
  {
    mx_i2c_start(OLED_address); //begin transmitting
    mx_i2c_write(0x40); //data mode
    mx_i2c_write(data);
    mx_i2c_end(); // stop transmitting
  }

  //==========================================================//
  // Prints a display char (not just a byte)
  // being multiples of 8. This means we have 16 COLS (0-15)
  // and 8 ROWS (0-7).
  void sendChar(unsigned char data)
  {
    mx_i2c_start(OLED_address);
    mx_i2c_write(0x40);
    for(int i=0;i<8;i++) {
      mx_i2c_write(pgm_read_byte(myFont[data-0x20]+i));
    }
    mx_i2c_end();
  } 

  //==========================================================//
  // Set the cursor position in a 16 COL * 2 ROW map.
  void setXY(unsigned char col,unsigned char row)
  {
    mx_i2c_start(OLED_address);
    mx_i2c_write(0x80);  
    mx_i2c_write(0xb0+(row)); //set page address (row)
    mx_i2c_write(0x80); //command mode
    #ifdef OLED1106_1_3            
      //mx_i2c_write(0x02+(8*col&0x0f)); //set low col address
      //mx_i2c_write(0x02+(8*col %16)); //set low col address
      mx_i2c_write( 0x02+(8*(col %2)) ); //set low col address
    #else
      //mx_i2c_write(0x00+(8*col&0x0f)); //set low col address
      //mx_i2c_write(0x00+(8*col %16)); //set low col address
      mx_i2c_write( 0x00+(8*(col %2)) ); //set low col address
    #endif
    mx_i2c_write(0x80); 
    //mx_i2c_write(0x10+((8*col>>4)&0x0f)); //set high col address
    mx_i2c_write(0x10+((col /2) %16)); //set high col address 
    mx_i2c_end();         
  }   

  //==========================================================//
  // Prints a string regardless the cursor position.
  void sendStr(const char *string)
  {
    unsigned char i=0;
    while(*string)
    {
      for(i=0;i<8;i++) {
        SendByte(pgm_read_byte(myFont[*string-0x20]+i));
      }
      string++;
    }
 
 /*
      const char *stringC=string;
      
      while(*stringC) {
        mx_i2c_start(OLED_address);
        mx_i2c_write(0x40);
        for(int i=0;i<8;i++){
          mx_i2c_write(pgm_read_byte(myFont[*stringC-0x20]+i));
        }
        mx_i2c_end();
        stringC++;
      } 
 */   
  }

  //==========================================================//
  // Prints a string in coordinates X Y, being multiples of 8.
  // This means we have 16 COLS (0-15) and 8 ROWS (0-7).
  void sendStrXY(const char *string, int X, int Y)
  {
    #ifdef XY
      setXY(X,Y);
      unsigned char i=0;
      while(*string)
      {
        #ifdef OLED1306_128_64
          for(i=0;i<8;i++)  SendByte(pgm_read_byte(myFont[*string-0x20]+i));
        #else
          for(i=0;i<4;i++)  SendByte(pgm_read_byte(myFont[*string-0x20]+i));    
        #endif
        string++;
      }
    #endif
  
    #if defined(XY2) && not defined(DoubleFont)
      int Xh=X, Xl=X;
      const char *stringL=string, *stringH=string;

      setXY(Xl,Y);
      while(*stringL) {
        mx_i2c_start(OLED_address);
        mx_i2c_write(0x40);

        for(int i=0;i<8;i++){
          int ril=(pgm_read_byte(myFont[*stringL-0x20]+i));
          //int il=(pgm_read_byte(&DFONT[ril & 0x0F]));
      //    int il=DFONT[ril %16];
          int il=pgm_read_byte(DFONT+(ril %16));
 /*
          for(int ib=0;ib<4;ib++){
            if (bitRead (ril,ib)){
              il |= (1 << ib*2);
              il |= (1 << (ib*2)+1);
            }
          }
*/
/*
          if (bitRead(ril,0)) il|= 1+2;
          if (bitRead(ril,1)) il|= 4+8;
          if (bitRead(ril,2)) il|= 16+32;
          if (bitRead(ril,3)) il|= 64+128;
*/

          //mx_i2c_start(OLED_address);
          //mx_i2c_write(0x40);
          mx_i2c_write(il);
          //mx_i2c_end();
        }

        mx_i2c_end();
        Xl++;    
        stringL++;
      }
    
      setXY(Xh,Y+1);
      while(*stringH){      
        mx_i2c_start(OLED_address);
        mx_i2c_write(0x40);           
        
        for(int i=0;i<8;i++){
          int rih=(pgm_read_byte(myFont[*stringH-0x20]+i));
          //int ih=(pgm_read_byte(&DFONT[rih >>4]));
       //   int ih=DFONT[rih / 16];
          int ih=pgm_read_byte(DFONT+(rih / 16));       
/*
          for(int ic=4;ic<8;ic++){
            if (bitRead (rih,ic)) {
              ih |= (1 << (ic-4)*2);
              ih |= (1 << ((ic-4)*2)+1);
            }   
          }
*/
/*
          if (bitRead(rih,4)) ih|= 1+2;
          if (bitRead(rih,5)) ih|= 4+8;
          if (bitRead(rih,6)) ih|= 16+32;
          if (bitRead(rih,7)) ih|= 64+128;
*/
          //mx_i2c_start(OLED_address);
          //mx_i2c_write(0x40);  
          mx_i2c_write(ih);
          //mx_i2c_end();          
        }

        mx_i2c_end();
        Xh++;    
        stringH++;
      }
    
    #endif // defined(XY2) && not defined(DoubleFont)

  #if defined(XY2) && defined(DoubleFont)
    int Xh=X, Xl=X;
    const char *stringL=string, *stringH=string;
  
    setXY(Xl,Y);
    while(*stringL) {
      mx_i2c_start(OLED_address);
      mx_i2c_write(0x40); 
    
      for(int i=0;i<8;i++){
        int ril=(pgm_read_byte(myFont[*stringL-0x20]+i));
        mx_i2c_write(ril);
      }
      mx_i2c_end();
      Xl++;    
      stringL++;
    }
  
    setXY (Xh,Y+1);
    while(*stringH) {
      mx_i2c_start(OLED_address);
      mx_i2c_write(0x40); 
      for(int i=0;i<8;i++){
        int rih=(pgm_read_byte(myFont[*stringH-0x20]+i+8));
        mx_i2c_write(rih);
      }
      mx_i2c_end();
      Xh++;    
      stringH++;
    }
  
    #endif // defined(XY2) && defined(DoubleFont)
  }

  //==========================================================//
  // Resets display depending on the actual mode.
  void reset_display(void)
  {
    displayOff();
    clear_display();  
    #if defined(video64text32)     // back to 128x32
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX     
      sendcommand(0x1f);            //--1/48 duty, NEW!!! Feb 23, 2013: 128x32 OLED: 0x01f,  128x64 OLED 0x03f     
      sendcommand(0xDA);           //0xDA
      sendcommand(0x02);           //COMSCANDEC /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */      
    #endif
    displayOn();
  }

  //==========================================================//
  // Turns display on.
  void displayOn(void)
  {
    sendcommand(0xaf);    //display on
  }

  //==========================================================//
  // Turns display off.
  void displayOff(void)
  {
    sendcommand(0xae);    //display off
  }

  //==========================================================//
  // Clears the display by sending 0 to all the screen map.
  void clear_display(void)
  {
    unsigned char i,k;
    #if defined(OLED1306_128_64) || defined(video64text32)
      for(k=0;k<8;k++) // 8 LINES
    #else
      for(k=0;k<4;k++) // 4 LINES  
    #endif
    { 
      setXY(0,k);    
      {
        for(i=0;i<128;i++)
        {
          SendByte(0);         //clear all COL
        }
      }
    }
  }
    
  //==========================================================//
  // Inits oled and draws logo at startup
  void init_OLED(void)
  {
    mx_i2c_init();

    /*
    sequence := { direct_value | escape_sequence }
    direct_value := 0..254
    escape_sequence := value_255 | sequence_end | delay | adr | cs | not_used 
    value_255 := 255 255
    sequence_end = 255 254
    delay := 255 0..127
    adr := 255 0x0e0 .. 0x0ef 
    cs := 255 0x0d0 .. 0x0df 
    not_used := 255 101..254
    */  
    sendcommand(0xAE);             //DISPLAYOFF

     //sendcommand(0xD5);  // SSD1306_SETDISPLAYCLOCKDIV
     //sendcommand(0x80);  // the suggested ratio 0x80, 4 higher bits for oscilator frecuency, and 4 lower for divide ratio

    // sendcommand(0xD3);  // SSD1306_SETDISPLAYOFFSET
    // sendcommand(0x0);  // no offset

    #if defined(OLED1306_128_64) || defined(video64text32) 
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX      
      sendcommand(0x3f);            //--1/48 duty, NEW!!!  128x64 OLED: 0x3f
    #else
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX     
      sendcommand(0x1f);            //--1/48 duty, NEW!!!  128x32 OLED: 0x1f
    #endif
      
    sendcommand(0x8D);            //CHARGEPUMP, 0x14 to enable and 0x10 to turn off
    sendcommand(0x14);
    
    #ifdef OLED_ROTATE180    
      sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg 0xA1
      sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg /* c0: scan dir normal, c8: reverse */
    #else    
      sendcommand(0xA0);            //SEGREMAP   //no rotation
      sendcommand(0xC0);            //COMSCANDEC  no reverse /* c0: scan dir normal */
    #endif
      
    //COMSCANDEC /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
    #if defined(OLED1306_128_64) || defined(video64text32)             
      sendcommand(0xDA);
      sendcommand(0x12);            //0x12 for 128x64 mode, default value                           
    #else                           
      sendcommand(0xDA);
      sendcommand(0x02);            //0x02 for 128x32 mode
    #endif

    #if defined(OLED_SETCONTRAS)
      sendcommand(0x81);                    //SETCONTRAS
      sendcommand(OLED_SETCONTRAS);         // value inside userconfig.h
    #elif defined(OLED1106_1_3)
      sendcommand(0x81);                    //SETCONTRAS
      sendcommand(0x27);                    // default 0x80 : (SMALL 0x00, LARGE 0xFF)
    #else
      sendcommand(0x81);                    //SETCONTRAS
      sendcommand(0x47);                    // default 0x80 : (SMALL 0x00, LARGE 0xFF)
    #endif
      
    #if defined(XY2) 
      sendcommand(0x20);            //Set Memory Addressing Mode
      sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
    #else
      sendcommand(0x20);            //Set Memory Addressing Mode
      sendcommand(0x02);            //Set Memory Addressing Mode ab Page addressing mode      
    #endif              

     //sendcommand(0xd9); // SSD1306_SETPRECHARGE
     //sendcommand(0xF1);

    // sendcommand(0xDB); // SSD1306_SETVCOMDETECT
    // sendcommand(0x40);

    // sendcommand(0xA4); // SSD1306_DISPLAYALLON_RESUME
    // sendcommand(0xA6); // SSD1306_NORMALDISPLAY
    // sendcommand(0x2E); // SSD1306_DEACTIVATE_SCROLL

    #if defined(LOAD_EEPROM_LOGO)
      byte t;
    #endif

    #if defined(OLED1306_128_64) || defined(video64text32)
      for(int j=0;j<8;j++) {
    #else
      for(int j=0;j<4;j++) {
    #endif
      setXY(0,j);
      for(int i=0;i<128;i++)     // show 128* 32 Logo
      {
        #ifdef LOAD_MEM_LOGO
          SendByte(pgm_read_byte(logo+j*128+i));
        #endif

        #if defined(RECORD_EEPROM_LOGO) && not defined(EEPROM_LOGO_COMPRESS)
          EEPROM_put(j*128+i, pgm_read_byte(logo+j*128+i));
        #endif

        #if defined(RECORD_EEPROM_LOGO) && defined(EEPROM_LOGO_COMPRESS)
          if (i%2 == 0){
            #ifdef OLED1306_128_64
              if (j%2 == 0){
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
              EEPROM_put(j*64+i/2, pgm_read_byte(logo+j*128+i));
            #endif
          }
        #endif   

        #if defined(LOAD_EEPROM_LOGO) && not defined(EEPROM_LOGO_COMPRESS)
          EEPROM_get(j*128+i, t);
          SendByte(t);
        #endif

        #if defined(LOAD_EEPROM_LOGO) && defined(EEPROM_LOGO_COMPRESS)
          if (i%2 == 0){
            t=0;
            #ifdef OLED1306_128_64
              if (j%2 == 0) {
                byte ril=0;
                byte ib=0;
                EEPROM_get((j/2)*64+i/2, ril);

                for(ib=0;ib<4;ib++) {
                  if (bitRead (ril,ib)) {
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

                for(ic=4;ic<8;ic++) {
                  if (bitRead (rih,ic)) {
                    t |= (1 << (ic-4)*2);
                    #ifdef COMPRESS_REPEAT_ROW
                      t |= (1 << ((ic-4)*2)+1);
                    #endif
                  }
                }
              }
            #else
              EEPROM_get(j*64+i/2, t);
            #endif
          }
          SendByte(t);
        #endif   
    
      }  
    }
    #if defined(LOAD_MEM_LOGO) || defined(LOAD_EEPROM_LOGO)
      sendcommand(0xAF);    //display on
    #endif
  }

//==========================================================//
//END #if defined(OLED1306)

#elif defined(P8544)

  pcd8544 lcd(dc_pin, reset_pin, cs_pin);

  void bitmap2(uint8_t bdata[], uint8_t rows, uint8_t columns)
  {
    uint8_t row, column;
    uint16_t  i; // de tipo word para poder leer mas de 256 bytes
    uint8_t toprow = 0;
    uint8_t startcolumn = 0;
    for (row = 0, i = 0; row < rows; row++) {
      lcd.gotoRc(row+toprow, startcolumn);
      for (column = 0; column < columns; column++) {
        lcd.data(pgm_read_byte(&bdata[i++]));
      }
    }
  }

  void P8544_splash (void)
  {
    lcd.gotoRc(0, 0);
    bitmap2(logo, 6,84);
    delay(2000); 
    lcd.clear();
  }

#endif // defined(P8544)


byte scrollPos = 0;                 //Stores scrolling text position
//unsigned long scrollTime = millis() + scrollWait;
unsigned long scrollTime = 0;

void scrollText(char* text, bool is_dir){
  if(millis()<scrollTime)
    return;

  #ifdef LCDSCREEN16x2
  //Text scrolling routine.  Setup for 16x2 screen so will only display 16 chars
  if(scrollPos<0) scrollPos=0;
  char outtext[17];
  byte i=0;
  byte p=scrollPos;
  if(is_dir) {
    outtext[0]='>';
    i++;
  }
  for(;i<16;i++,p++)
  {
    if(p<strlen(text)) 
    {
      outtext[i]=text[p];
    } else {
      outtext[i]='\0';
    }
  }
  outtext[16]='\0';
  printtext(outtext,1);
  #endif

  #ifdef OLED1306
  //Text scrolling routine.  Setup for 16x2 screen so will only display 16 chars
  if(scrollPos<0) scrollPos=0;
  char outtext[17];
  byte i=0;
  byte p=scrollPos;
  if(is_dir) {
    outtext[0]='>';
    i++;
  }
  for(;i<16;i++,p++)
  {
    if(p<strlen(text)) 
    {
      outtext[i]=text[p];
    } else {
      outtext[i]='\0';
    }
  }
  outtext[16]='\0';
  printtext(outtext,lineaxy);
  #endif

  #ifdef P8544
  //Text scrolling routine.  Setup for P8544 screen so will only display 14 chars
  if(scrollPos<0) scrollPos=0;
  char outtext[15];
  byte i=0;
  byte p=scrollPos;
  if(is_dir) {
    outtext[0]='>';
    i++;
  }
  for(;i<14;i++,p++)
  {
    if(p<strlen(text)) 
    {
      outtext[i]=text[p];
    } else {
      outtext[i]='\0';
    }
  }
  outtext[14]='\0';
  printtext(outtext,1);
  #endif

  scrollTime = millis();
  scrollTime += (scrollPos? scrollSpeed : scrollWait);
  scrollPos = (scrollPos+1) %strlen(text);
}

void scrollText(char* text, bool is_dir, byte scroll_pos) {
  // this variant resets the scroll position and timer, so printing is immediate
  scrollPos = scroll_pos;
  scrollTime = 0;
  scrollText(text, is_dir);
}

char fline[17];

void printtext2F(const char* text, int l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
  Serial.println(reinterpret_cast <const __FlashStringHelper *> (text));
  #endif
  
  #ifdef LCDSCREEN16x2
    lcd.setCursor(0,l);
    char x = 0;
    while (char ch=pgm_read_byte(text+x)) {
      lcd.print(ch);
      x++;
    }
  #endif

  #ifdef OLED1306
    #ifdef XY2
      strncpy_P(fline, text, 16);
      sendStrXY(fline,0,l);
    #endif
     
    #ifdef XY 
      setXY(0,l);
      char x = 0;
      while (char ch=pgm_read_byte(text+x)) {
        sendChar(ch);
        x++;
      }
    #endif
  #endif

  #ifdef P8544
    strncpy_P(fline, text, 14);
    lcd.setCursor(0,l);
    lcd.print(fline);
  #endif 
   
}

void printtextF(const char* text, int l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
    Serial.println(reinterpret_cast <const __FlashStringHelper *> (text));
  #endif
  
  #ifdef LCDSCREEN16x2
    lcd.setCursor(0,l);
    char x = 0;
    while (char ch=pgm_read_byte(text+x)) {
      lcd.print(ch);
      x++;
    }
    for(; x<16; x++) {
      lcd.print(' ');
    }
  #endif

  #ifdef OLED1306
    #ifdef XY2
      strncpy_P(fline, text, 16);
      for(int i=strlen(fline);i<16;i++) {
        fline[i]=0x20;
      }
      sendStrXY(fline,0,l);
    #endif
     
    #ifdef XY 
      setXY(0,l);
      char x = 0;
      while (char ch=pgm_read_byte(text+x)) {
        sendChar(ch);
        x++;
      }
      for(; x<16; x++) {
        sendChar(' ');
      }
     #endif
  #endif

  #ifdef P8544
    strncpy_P(fline, text, 14);
    for(int i=strlen(fline);i<14;i++) {
      fline[i]=0x20;
    }
    lcd.setCursor(0,l);
    lcd.print(fline);
  #endif 
   
}

void printtext(char* text, int l) {  //Print text to screen. 
  
  #ifdef SERIALSCREEN
    Serial.println(text);
  #endif
  
  #ifdef LCDSCREEN16x2
    lcd.setCursor(0,l);
    char ch;
    bool end=false;
    for(byte i=0;i<16;i++) {
      if(!end)
        if(text[i]=='\0')
          end=true;
      if(!end)
        ch=text[i];
      else
        ch=' ';
      lcd.print(ch); 
    }
  #endif

  #ifdef OLED1306
    #ifdef XY2
      bool end=false;
      for(byte i=0;i<16;i++) {
        if(!end)
          if(text[i]=='\0')
            end=true;
        if(!end)
          fline[i]=text[i];
        else
          fline[i]=' ';
      }    
      sendStrXY(fline,0,l);
    #endif
    
    #ifdef XY
      setXY(0,l); 
      char ch;
      bool end=false;
      for(byte i=0;i<16;i++) {
        if(!end)
          if(text[i]=='\0')
            end=true;
        if(!end)
          ch=text[i];
        else
          ch=' ';
        sendChar(ch);
      }       
    #endif
  #endif

  #ifdef P8544
    bool end=false;
    for(byte i=0;i<16;i++) {
      if(!end)
        if(text[i]=='\0')
          end=true;
      if(!end)
        fline[i]=text[i];
      else
        fline[i]=' ';
    }  
    lcd.setCursor(0,l);
    lcd.print(fline);
  #endif 
}

#if defined(OLED1306)
void OledStatusLine() {
  #ifdef XY
    setXY(4,2);
    sendStr("ID:   BLK:");
    #ifdef OLED1306_128_64
      setXY(0,7);
      utoa(BAUDRATE,(char *)fline,10);
      sendStr((char *)fline);

      #ifndef NO_MOTOR       
        setXY(5,7);
        if(mselectMask) {
          sendStr(" M:ON");
        } else {
          sendStr("m:off");
        }
      #endif    

      setXY(11,7); 
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY) {
        sendStr(" %^ON");
      } else {
        sendStr("%^off");
      }

    #else // OLED1306_128_64 not defined

      setXY(0,3);
      utoa(BAUDRATE,(char *)fline,10);sendStr((char *)fline);
      #ifndef NO_MOTOR        
        setXY(5,3);
        if(mselectMask) {
          sendStr(" M:ON");
        } else {
          sendStr("m:off");
        }
      #endif    
      setXY(11,3); 
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY) {
        sendStr(" %^ON");
      } else {
        sendStr("%^off");
      }
    #endif
  #endif
  
  #ifdef XY2                        // Y with double value
    #ifdef OLED1306_128_64          // 8 rows supported
      sendStrXY("ID:   BLK:",4,4);        
      utoa(BAUDRATE,(char *)fline,10);
      sendStrXY((char *)fline,0,6);
      #ifndef NO_MOTOR       
        if(mselectMask) {
          sendStrXY(" M:ON",5,6);
        } else {
          sendStrXY("m:off",5,6);
        }
      #endif      
      if (TSXCONTROLzxpolarityUEFSWITCHPARITY) {
        sendStrXY(" %^ON",11,6);
      } else {
        sendStrXY("%^off",11,6);
      }
    #endif      
  #endif  

}
#endif // defined(OLED1306)

void scrollTextReset()
{
  scrollTime=millis()+scrollWait;
  scrollPos=0;
}
