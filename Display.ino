#include "Fonts.h"
#include "Logos.h"
#ifdef OLED1306

    //==========================================================//
    // Used to send commands to the display.
    static void sendcommand(unsigned char com)
    {
    Wire.beginTransmission(OLED_address); //begin transmitting
    Wire.write(0x80); //command mode
    Wire.write(com);
    Wire.endTransmission(); // stop transmitting
    }
    //==========================================================//
    // Actually this sends a byte, not a char to draw in the display.
    // Display's chars uses 8 byte font the small ones and 96 bytes
    // for the big number font.
    static void SendByte(unsigned char data)
    {
    Wire.beginTransmission(OLED_address); // begin transmitting
    Wire.write(0x40);//data mode
    Wire.write(data);
    Wire.endTransmission(); // stop transmitting
    }
    //==========================================================//
    // Prints a display char (not just a byte)
    // being multiples of 8. This means we have 16 COLS (0-15)
    // and 8 ROWS (0-7).
    static void sendChar(unsigned char data)
    {
    Wire.beginTransmission(OLED_address); // begin transmitting
    Wire.write(0x40);//data mode
    for(int i=0;i<8;i++)  Wire.write(pgm_read_byte(myFont[data-0x20]+i));
    Wire.endTransmission(); // stop transmitting
    } 
    /*
    //==========================================================//
    // Prints a display char (not just a byte) in coordinates X Y,
    // being multiples of 8. This means we have 16 COLS (0-15)
    // and 8 ROWS (0-7).
    static void sendCharXY(unsigned char data, int X, int Y)
    {
    setXY(X, Y);
    Wire.beginTransmission(OLED_address); // begin transmitting
    Wire.write(0x40);//data mode
    for(int i=0;i<8;i++)  Wire.write(pgm_read_byte(myFont[data-0x20]+i));
    Wire.endTransmission(); // stop transmitting
    }
    */
    //==========================================================//
    // Set the cursor position in a 16 COL * 2 ROW map.
    static void setXY(unsigned char col,unsigned char row)
    {
    sendcommand(0xb0+row); //set page address (row)    
    #ifdef OLED1106_1_3
      sendcommand(0x02+(8*col&0x0f)); //set low col address
    #else
      sendcommand(0x00+(8*col&0x0f)); //set low col address
    #endif    
      sendcommand(0x10+((8*col>>4)&0x0f)); //set high col address

    //sendcommand(0x21); // set column start and end address
    //sendcommand(8*col);   // set column start address
    //sendcommand(0x7f);  // set column end address
    //sendcommand(0x22);  // set row start and end address
    //sendcommand(row); // set row start address
    //sendcommand(0x07);  // set row end address           
    }   
    //==========================================================//
    // Prints a string regardless the cursor position.
    static void sendStr(unsigned char *string)
    {
    unsigned char i=0;
    while(*string)
    {
    for(i=0;i<8;i++)
    {
    SendByte(pgm_read_byte(myFont[*string-0x20]+i));
    }
    string++;
    }
    }
    //==========================================================//
    // Prints a string in coordinates X Y, being multiples of 8.
    // This means we have 16 COLS (0-15) and 8 ROWS (0-7).
    static void sendStrXY( char *string, int X, int Y)
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
    char *stringL=string, *stringH=string;

    #if defined(OLED1106_1_3)
      setXY(Xl,Y);
    #else
      setXY(Xl,Y);   
      //sendcommand(0x22); // set row start and end address
      //sendcommand(Y);   // set row start address
      //sendcommand(Y+1);  // set row end address         
      //sendcommand(0x21); // set column start and end address
      //sendcommand(8*X);   // set column start address
      //sendcommand(8*(X+strlen(string))-1);  // set column end address
    #endif 
    while(*stringL){
      //setXY(Xl,Y);
      Wire.beginTransmission(OLED_address); // begin transmitting
      Wire.write(0x40);//data mode
      for(int i=0;i<8;i++){
          int il=0;
          int ril=(pgm_read_byte(myFont[*stringL-0x20]+i));
          
          for(int ib=0;ib<4;ib++){
              if (bitRead (ril,ib)){
                  il |= (1 << ib*2);
                  il |= (1 << (ib*2)+1);
              }
          }
          //SendByte(il);
          Wire.write(il);
      }
      Wire.endTransmission(); // stop transmitting
      Xl++;    
      stringL++;
    }
    
    #if defined(OLED1106_1_3)
      setXY (Xh,Y+1);
    #else
      setXY (Xh,Y+1);    
    #endif       
    while(*stringH){      
      //setXY (Xh,Y+1);      
      Wire.beginTransmission(OLED_address); // begin transmitting
      Wire.write(0x40);//data mode          
      for(int i=0;i<8;i++){
         int ih=0;
         int rih=(pgm_read_byte(myFont[*stringH-0x20]+i));
        
         for(int ic=4;ic<8;ic++){
            if (bitRead (rih,ic)) {
                ih |= (1 << (ic-4)*2);
                ih |= (1 << ((ic-4)*2)+1);
            }   
          }
          //SendByte(ih);
          Wire.write(ih);          
      }
      Wire.endTransmission(); // stop transmitting      
      Xh++;    
      stringH++;
    }
    
 /*   while(*string){

      setXY(X,Y);    
      for(int i=0;i<8;i++){
          unsigned int il=0;
          unsigned int ril=(pgm_read_byte(myFont[*string-0x20]+i));
          
          for(int ib=0;ib<4;ib++){
              if (bitRead (ril,ib)){
                  il |= (1 << ib*2);
                  il |= (1 << (ib*2)+1);
              }
          }
          SendByte(il);
      }
      
      setXY (X,Y+1);
      for(int i=0;i<8;i++){
         unsigned int ih=0;
         unsigned int rih=(pgm_read_byte(myFont[*string-0x20]+i));
        
         for(int ic=4;ic<8;ic++){
            if (bitRead (rih,ic)) {
                ih |= (1 << (ic-4)*2);
                ih |= (1 << ((ic-4)*2)+1);
            }   
          }
          SendByte(ih);
      }
      X++;    
     string++;
    } */
    #endif

    #if defined(XY2) && defined(DoubleFont)
    int Xh=X, Xl=X;
    char *stringL=string, *stringH=string;
    
    #if defined(OLED1106_1_3)
      setXY(Xl,Y);
    #else
      setXY(Xl,Y);       
      //sendcommand(0x22); // set row start and end address
      //sendcommand(Y);   // set row start address
      //sendcommand(Y+1);  // set row end address         
      //sendcommand(0x21); // set column start and end address
      //sendcommand(8*X);   // set column start address
      //sendcommand(8*(X+strlen(string))-1);  // set column end address
    #endif  
    while(*stringL){
      //setXY(Xl,Y);
      Wire.beginTransmission(OLED_address); // begin transmitting
      Wire.write(0x40);//data mode
      for(int i=0;i<8;i++){
          int ril=(pgm_read_byte(myFont[*stringL-0x20]+i));
          Wire.write(ril);
      }
      Wire.endTransmission(); // stop transmitting
      Xl++;    
      stringL++;
    }
    
    #if defined(OLED1106_1_3)
      setXY (Xh,Y+1);
    #else
      setXY (Xh,Y+1);    
    #endif     
    while(*stringH){      
      //setXY (Xh,Y+1);      
      Wire.beginTransmission(OLED_address); // begin transmitting
      Wire.write(0x40);//data mode          
      for(int i=0;i<8;i++){
          int rih=(pgm_read_byte(myFont[*stringH-0x20]+i+8));
          Wire.write(rih);          
      }
      Wire.endTransmission(); // stop transmitting      
      Xh++;    
      stringH++;
    }
    
    #endif    
    }
    //==========================================================//

// Resets display depending on the actual mode.
static void reset_display(void)
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
    sendcommand(0xaf);        //display on
}

//==========================================================//
// Turns display off.
void displayOff(void)
{
  sendcommand(0xae);    //display off
}

//==========================================================//
// Clears the display by sendind 0 to all the screen map.
static void clear_display(void)
{
  unsigned char i,k;
  #if defined(OLED1306_128_64) || defined(video64text32)
    for(k=0;k<8;k++)  // 8 LINES
  #else
    for(k=0;k<4;k++) // 4 LINES  
  #endif
  { 
    setXY(0,k);    
    {
      for(i=0;i<128;i++)     //was 128
      {
        SendByte(0);         //clear all COL
        //delay(10);
      }
    }
  }
}
    
//==========================================================//
// Inits oled and draws logo at startup
static void init_OLED(void)
{
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
  //sendcommand(0xae);    //display off
  //sendcommand(0xa6);            //Set Normal Display (default) 
    // Adafruit Init sequence for 128x32 OLED module

/*    sendcommand(0xFF); // U8G_ESC_CS(0) disable chip
    sendcommand(0xd0 | ((0)&0x0f));
    sendcommand(0xFF); // 8G_ESC_ADR(0) instruction mode
    sendcommand(0xe0 | ((0)&0x0f));
    sendcommand(0xFF); // U8G_ESC_RST(1 do reset low pulse with (1*16)+2 milliseconds
    sendcommand(0xc0 | ((1)&0x0f));        
    sendcommand(0xFF); // U8G_ESC_CS(1) enable chip
    sendcommand(0xd0 | ((1)&0x0f)); */    
        
    sendcommand(0xAE);             //DISPLAYOFF
    //sendcommand(0xD5);            //SETDISPLAYCLOCKDIV
    //sendcommand(0x80);            // the suggested ratio 0x80
    
    #if defined(OLED1306_128_64) || defined(video64text32) 
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX      
      sendcommand(0x3f);            //--1/48 duty, NEW!!! Feb 23, 2013: 128x32 OLED: 0x01f,  128x64 OLED 0x03f
    #else
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX     
      sendcommand(0x1f);            //--1/48 duty, NEW!!! Feb 23, 2013: 128x32 OLED: 0x01f,  128x64 OLED 0x03f      
    #endif
    
    //sendcommand(0xD3);            //SETDISPLAYOFFSET
    //sendcommand(0x0);             //no offset
    //sendcommand(0x40 | 0x0);      //SETSTARTLINE
    sendcommand(0x8D);            //CHARGEPUMP
    sendcommand(0x14);
    //sendcommand(0x20);             //MEMORYMODE
    //sendcommand(0x02);             //com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5), Feb 23, 2013: 128x32 OLED: 0x002,  128x32 OLED 0x012

#ifdef OLED_ROTATE180    
    sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg 0xA1
    //sendcommand(0xA0);
    
    sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg /* c0: scan dir normal, c8: reverse */
    //sendcommand(0xC0);
//#endif
//#ifdef UNROTATE
#else    
    //sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg 0xA1
    sendcommand(0xA0);
    
    //sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg /* c0: scan dir normal, c8: reverse */
    sendcommand(0xC0);
#endif
    
    #if defined(OLED1306_128_64) || defined(video64text32)             
      sendcommand(0xDA);
      sendcommand(0x12);            //0x12 for 128x64 mode, default value                           
    #else                           //0x02 for 128x32 mode
      sendcommand(0xDA);            //0xDA
      sendcommand(0x02);           //COMSCANDEC /* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
    #endif

    #ifdef OLED_SETCONTRAS
          sendcommand(0x81);                    //SETCONTRAS
          sendcommand(OLED_SETCONTRAS);         // value inside userconfig.h
    #else
        #ifdef OLED1106_1_3
          sendcommand(0x81);                    //SETCONTRAS
          sendcommand(0x27);                    // default 0x80 : (SMALL 0x00, LARGE 0xFF)
        #else
          sendcommand(0x81);                    //SETCONTRAS
          sendcommand(0x47);                    // default 0x80 : (SMALL 0x00, LARGE 0xFF)
        #endif
    #endif
    
   // sendcommand(0xD9);          //SETPRECHARGE /* [2] pre-charge period 0x022/f1*/
   // sendcommand(0xf1); 
   // sendcommand(0xDB);        //SETVCOMDETECT                
   // sendcommand(0x40);
   // sendcommand(0x2E);            // stop scroll
   // sendcommand(0xA4);        //DISPLAYALLON_RESUME        
   // sendcommand(0xA6);        //NORMALDISPLAY
   #if defined(XY2) 
      sendcommand(0x20);            //Set Memory Addressing Mode
      sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
   #else
      sendcommand(0x20);            //Set Memory Addressing Mode
      sendcommand(0x02);            //Set Memory Addressing Mode ab Page addressing mode      
   #endif              
    //sendcommand(0xAF);    //display on

/*    sendcommand(0xFF); // U8G_ESC_CS(0) disable chip
    sendcommand(0xd0 | ((0)&0x0f));
    sendcommand(0xFF); // U8G_ESC_END end of sequence
    sendcommand(0xfe); */

  //----------------------------REVERSE comments----------------------------//
  //  sendcommand(0xa0);    //seg re-map 0->127(default)
  //  sendcommand(0xa1);    //seg re-map 127->0
  //  sendcommand(0xc8);
  //  delay(1000);
  //----------------------------REVERSE comments----------------------------//
  // sendcommand(0xa7);  //Set Inverse Display  
  // sendcommand(0xae);   //display off
  //sendcommand(0x20);            //Set Memory Addressing Mode
  //sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
    //sendcommand(0x02);         // Set Memory Addressing Mode ab Page addressing mode(RESET)  
  
  //clear_display();
  
  #if defined(OLED1306_128_64) || defined(video64text32)
    for(int j=0;j<8;j++)
  #else
    for(int j=0;j<4;j++)  
  #endif
  {
    setXY(0,j);
    for(int i=0;i<128;i++)     // show 128* 32 Logo
    {
   #ifdef LOAD_MEM_LOGO
     SendByte(pgm_read_byte(logo+j*128+i));
   #endif
   #if defined(RECORD_EEPROM_LOGO) && not defined(EEPROM_LOGO_COMPRESS)
     #if defined(__AVR__) 
      EEPROM.put(j*128+i, pgm_read_byte(logo+j*128+i));
     #elif defined(__arm__) && defined(__STM32F1__)
      EEPROM_put(j*128+i, pgm_read_byte(logo+j*128+i));
     #endif      
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
              #if defined(__AVR__)             
                EEPROM.put((j/2)*64+i/2,nl+nh*16);
              #elif defined(__arm__) && defined(__STM32F1__)
                EEPROM_put((j/2)*64+i/2,nl+nh*16);
              #endif                          
            } 

        #else
            #if defined(__AVR__)     
              EEPROM.put(j*64+i/2, pgm_read_byte(logo+j*128+i));
            #elif defined(__arm__) && defined(__STM32F1__)
              EEPROM_put(j*64+i/2, pgm_read_byte(logo+j*128+i));
            #endif
        #endif
     }
   #endif   
   #if defined(LOAD_EEPROM_LOGO) && not defined(EEPROM_LOGO_COMPRESS)
     #if defined(__AVR__)
      EEPROM.get(j*128+i,hdrptr);
     #elif defined(__arm__) && defined(__STM32F1__)
      EEPROM_get(j*128+i,&hdrptr);
     #endif
     SendByte(hdrptr);
   #endif
   #if defined(LOAD_EEPROM_LOGO) && defined(EEPROM_LOGO_COMPRESS)
     if (i%2 == 0){
        #ifdef OLED1306_128_64
            if (j%2 == 0){
              byte il=0;
              byte ril=0;
              byte ib=0;
              #if defined(__AVR__)
                EEPROM.get((j/2)*64+i/2,ril);
              #elif defined(__arm__) && defined(__STM32F1__)
                EEPROM_get((j/2)*64+i/2,&ril);
              #endif
              for(ib=0;ib<4;ib++) {
                if (bitRead (ril,ib)) {
                  il |= (1 << ib*2);
                  #ifdef COMPRESS_REPEAT_ROW
                    il |= (1 << (ib*2)+1);
                  #endif
                }
              }
              hdrptr = il;
            } else {
              byte ih=0;
              byte rih=0;
              byte ic=0;
              #if defined(__AVR__) 
                EEPROM.get((j/2)*64+i/2,rih);
              #elif defined(__arm__) && defined(__STM32F1__)
                EEPROM_get((j/2)*64+i/2,&rih);
              #endif
              for(ic=4;ic<8;ic++) {
                if (bitRead (rih,ic)) {
                  ih |= (1 << (ic-4)*2);
                  #ifdef COMPRESS_REPEAT_ROW
                    ih |= (1 << ((ic-4)*2)+1);
                  #endif
                }
              }
              hdrptr = ih;                           
            }
        #else
            #if defined(__AVR__)
              EEPROM.get(j*64+i/2,hdrptr);
            #elif defined(__arm__) && defined(__STM32F1__)
              EEPROM_get(j*64+i/2,&hdrptr);
            #endif
        #endif
     }
     SendByte(hdrptr);
   #endif   
   
    }  
  }
  sendcommand(0xAF);    //display on
}

#endif

//==========================================================//

#ifdef P8544

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

 static void P8544_splash (void)
{
  lcd.gotoRc(0, 0);
  /* lcd.bitmap(logo1, 3,84);
  lcd.gotoRc(3, 0);
  lcd.bitmap(logo2, 3,84); */
  bitmap2(logo, 6,84);
  
  delay(2000); 
  lcd.clear();
  
}
#endif
