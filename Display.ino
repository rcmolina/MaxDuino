#include "Fonts.h"
#include "Logos.h"

#if defined(OLED1306)

  //==========================================================//
  // Used to send commands to the display.
  static void sendcommand(unsigned char com)
  {
    #if defined(Use_SoftI2CMaster)       
      i2c_start((OLED_address<<1)|I2C_WRITE);
      i2c_write(0x80);
      i2c_write(com); 
      i2c_stop();
    #else            
      Wire.beginTransmission(OLED_address); //begin transmitting
      Wire.write(0x80); //command mode
      Wire.write(com);
      Wire.endTransmission(); // stop transmitting
    #endif
  }

  //==========================================================//
  // Actually this sends a byte, not a char to draw in the display.
  // Display's chars uses 8 byte font
  static void SendByte(unsigned char data)
  {
    #if defined(Use_SoftI2CMaster)       
      i2c_start((OLED_address<<1)|I2C_WRITE); 
      i2c_write(0x40); 
      i2c_write(data); 
      i2c_stop();
    #else                   
      Wire.beginTransmission(OLED_address); // begin transmitting
      Wire.write(0x40);//data mode
      Wire.write(data);
      Wire.endTransmission(); // stop transmitting
    #endif
  }

  //==========================================================//
  // Prints a display char (not just a byte)
  // being multiples of 8. This means we have 16 COLS (0-15)
  // and 8 ROWS (0-7).
  static void sendChar(unsigned char data)
  {
    #if defined(Use_SoftI2CMaster)       
      i2c_start((OLED_address<<1)|I2C_WRITE);
      i2c_write(0x40);
      for(int i=0;i<8;i++) {
        i2c_write(pgm_read_byte(myFont[data-0x20]+i));
      }
      i2c_stop();
    #else                   
      Wire.beginTransmission(OLED_address); // begin transmitting
      Wire.write(0x40);//data mode
      for(int i=0;i<8;i++) {
        Wire.write(pgm_read_byte(myFont[data-0x20]+i));
      }
      Wire.endTransmission(); // stop transmitting
    #endif
  } 

  //==========================================================//
  // Set the cursor position in a 16 COL * 2 ROW map.
  static void setXY(unsigned char col,unsigned char row)
  {
    #if defined(Use_SoftI2CMaster)       
      i2c_start((OLED_address<<1)|I2C_WRITE);
      i2c_write(0x80);  
      i2c_write(0xb0+(row)); //set page address (row)
      i2c_write(0x80); //command mode
      #ifdef OLED1106_1_3            
        i2c_write(0x02+(8*col&0x0f)); //set low col address
      #else
        i2c_write(0x00+(8*col&0x0f)); //set low col address
      #endif
      i2c_write(0x80); 
      i2c_write(0x10+((8*col>>4)&0x0f)); //set high col address 
      i2c_stop();         
    #else                              
      Wire.beginTransmission(OLED_address); //begin transmitting
      Wire.write(0x80); //command mode
      Wire.write(0xb0+(row)); //set page address (row)
      Wire.write(0x80); //command mode
      #ifdef OLED1106_1_3            
        Wire.write(0x02+(8*col&0x0f)); //set low col address
      #else
        Wire.write(0x00+(8*col&0x0f)); //set low col address
      #endif
      Wire.write(0x80); //command mode   
      Wire.write(0x10+((8*col>>4)&0x0f)); //set high col address   
      Wire.endTransmission(); // stop transmitting                 
    #endif
  }   

  //==========================================================//
  // Prints a string regardless the cursor position.
  static void sendStr(const char *string)
  {
    unsigned char i=0;
    while(*string)
    {
      for(i=0;i<8;i++) {
        SendByte(pgm_read_byte(myFont[*string-0x20]+i));
      }
      string++;
    }
  }

  //==========================================================//
  // Prints a string in coordinates X Y, being multiples of 8.
  // This means we have 16 COLS (0-15) and 8 ROWS (0-7).
  static void sendStrXY(const char *string, int X, int Y)
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
        #if defined(Use_SoftI2CMaster)       
          i2c_start((OLED_address<<1)|I2C_WRITE);
          i2c_write(0x40);
        #else                  
          Wire.beginTransmission(OLED_address); // begin transmitting
          Wire.setClock(I2CCLOCK);     
          Wire.write(0x40);//data mode
        #endif

        for(int i=0;i<8;i++){
          int il=0;
          int ril=(pgm_read_byte(myFont[*stringL-0x20]+i));
          
          for(int ib=0;ib<4;ib++){
            if (bitRead (ril,ib)){
              il |= (1 << ib*2);
              il |= (1 << (ib*2)+1);
            }
          }
  
          #if defined(Use_SoftI2CMaster)           
            i2c_write(il);
          #else          
            Wire.write(il);
          #endif
        }

        #if defined(Use_SoftI2CMaster) 
          i2c_stop();
        #else       
          Wire.endTransmission(); // stop transmitting
        #endif   

        Xl++;    
        stringL++;
      }
    
      setXY(Xh,Y+1);
      while(*stringH){      
        #if defined(Use_SoftI2CMaster) 
          i2c_start((OLED_address<<1)|I2C_WRITE);
          i2c_write(0x40);           
        #else
          Wire.beginTransmission(OLED_address); // begin transmitting
          Wire.setClock(I2CCLOCK);
          Wire.write(0x40);//data mode        
        #endif
        
        for(int i=0;i<8;i++){
          int ih=0;
          int rih=(pgm_read_byte(myFont[*stringH-0x20]+i));
        
          for(int ic=4;ic<8;ic++){
            if (bitRead (rih,ic)) {
              ih |= (1 << (ic-4)*2);
              ih |= (1 << ((ic-4)*2)+1);
            }   
          }
          #if defined(Use_SoftI2CMaster)           
            i2c_write(ih);
          #else            
            Wire.write(ih);
          #endif
        }

        #if defined(Use_SoftI2CMaster)       
          i2c_stop();
        #else      
          Wire.endTransmission(); // stop transmitting
        #endif
      
        Xh++;    
        stringH++;
      }
    
    #endif // defined(XY2) && not defined(DoubleFont)

  #if defined(XY2) && defined(DoubleFont)
    int Xh=X, Xl=X;
    char *stringL=string, *stringH=string;
  
    setXY(Xl,Y);
    while(*stringL) {
      #if defined(Use_SoftI2CMaster)       
        i2c_start((OLED_address<<1)|I2C_WRITE);
        i2c_write(0x40); 
      #else            
        Wire.beginTransmission(OLED_address); // begin transmitting
        Wire.setClock(I2CCLOCK);
        Wire.write(0x40);//data mode
      #endif
    
      for(int i=0;i<8;i++){
        int ril=(pgm_read_byte(myFont[*stringL-0x20]+i));
        #if defined(Use_SoftI2CMaster)          
          i2c_write(ril);
        #else          
          Wire.write(ril);
        #endif 
      }
      #if defined(Use_SoftI2CMaster)       
        i2c_stop(); 
      #else        
        Wire.endTransmission(); // stop transmitting
      #endif
    
      Xl++;    
      stringL++;
    }
  
    setXY (Xh,Y+1);
    while(*stringH) {
      #if defined(Use_SoftI2CMaster)        
        i2c_start((OLED_address<<1)|I2C_WRITE);
        i2c_write(0x40); 
      #else                 
        Wire.beginTransmission(OLED_address); // begin transmitting
        Wire.setClock(I2CCLOCK);
        Wire.write(0x40);//data mode 
      #endif    
      for(int i=0;i<8;i++){
        int rih=(pgm_read_byte(myFont[*stringH-0x20]+i+8));
        #if defined(Use_SoftI2CMaster)           
          i2c_write(rih);
        #else            
          Wire.write(rih);
        #endif         
      }
      #if defined(Use_SoftI2CMaster)       
        i2c_stop();
      #else        
        Wire.endTransmission(); // stop transmitting
      #endif
    
      Xh++;    
      stringH++;
    }
  
    #endif // defined(XY2) && defined(DoubleFont)
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
  static void clear_display(void)
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
    sendcommand(0xAE);             //DISPLAYOFF
      
    #if defined(OLED1306_128_64) || defined(video64text32) 
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX      
      sendcommand(0x3f);            //--1/48 duty, NEW!!!  128x64 OLED: 0x3f
    #else
      sendcommand(0xA8);            //SSD1306_SETMULTIPLEX     
      sendcommand(0x1f);            //--1/48 duty, NEW!!!  128x32 OLED: 0x1f
    #endif
      
    sendcommand(0x8D);            //CHARGEPUMP
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
              if (j%2 == 0) {
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
    #if defined(LOAD_MEM_LOGO) || defined(LOAD_EEPROM_LOGO)
      sendcommand(0xAF);    //display on
    #endif
  }

//==========================================================//
//END #if defined(OLED1306)

#elif defined(P8544)

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
    bitmap2(logo, 6,84);
    delay(2000); 
    lcd.clear();
  }

#endif // defined(P8544)
