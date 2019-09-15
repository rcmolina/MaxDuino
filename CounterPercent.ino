
void lcdSpinner() {
    if (mymillis() - timeDiff2 > 1000) {   // check switch every 50ms 
        timeDiff2 = mymillis();           // get current millisecond count
        #ifdef LCDSCREEN16x2          
          lcd.setCursor(15,0);
          lcd.print(indicators[spinpos++]);
        #endif
        #ifdef OLED1306
            //sendCharXY(indicators[spinpos++],15,0);
           setXY(15,0);sendChar(indicators[spinpos++]);                 
        #endif
        #ifdef P8544
          lcd.setCursor(8,3);
          lcd.print(indicators[spinpos++]);
        #endif                
        if (spinpos > 3) {   
          spinpos = 0;
        }    
    } 
}

void lcdTime() {
    if (mymillis() - timeDiff2 > 1000) {   // check switch every second 
        timeDiff2 = mymillis();           // get current millisecond count

  /*       switch(lcdsegs){
            case 0:
              offset=2;
              break;
            case 10:
              offset=1;
              break;
            case 100:
              offset=0;
              break;
            case 1000:
              lcdsegs=0;
              offset=2;
              break;         
          } */
          
        #ifdef LCDSCREEN16x2

            if (lcdsegs % 10 != 0) {itoa(lcdsegs%10,PlayBytes,10);lcd.setCursor(15,0);lcd.print(PlayBytes);} // ultima cifra 1,2,3,4,5,6,7,8,9
            else 
               if (lcdsegs % CNTRBASE != 0){itoa(lcdsegs%CNTRBASE,PlayBytes,10);lcd.setCursor(14,0);lcd.print(PlayBytes);} // es 10,20,30,40,50,60,70,80,90,110,120,..
               else 
                  if (lcdsegs % (CNTRBASE*10) != 0) {itoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);lcd.setCursor(13,0);lcd.print(PlayBytes);} // es 100,200,300,400,500,600,700,800,900,1100,..
                  else {lcd.setCursor(13,0);lcd.print("000");} // es 000,1000,2000,...
/*
          if (lcdsegs % 10 != 0) {lcd.setCursor(15,0);lcd.print(lcdsegs%10);} // ultima cifra 1,2,3,4,5,6,7,8,9
          else 
             if (lcdsegs % 100 != 0){lcd.setCursor(14,0);lcd.print(lcdsegs%100);}  // es 10,20,30,40,50,60,70,80,90,110,120,..
             else 
                if (lcdsegs %1000 != 0) {lcd.setCursor(13,0);lcd.print(lcdsegs%1000);}  // es 100,200,300,400,500,600,700,800,900,1100,..
                else {lcd.setCursor(13,0);lcd.print("000");}   // es 000,1000,2000,...
*/

           lcdsegs++;
          //if (lcdsegs == 1000) lcdsegs = 0; 
          //itoa(lcdsegs++,PlayBytes,10);lcd.setCursor(13,0);lcd.print(PlayBytes);          
    /*      switch(offset){
            case 0:
              break;
            case 1:
              //lcd.print("0");
              break;
            case 2:
              //lcd.print("00");
              break;          
          }  */
          //lcd.print(lcdsegs++);                 
          //sprintf(PlayBytes, "%03d",lcdsegs++);lcd.print(PlayBytes);
           //sprintf(PlayBytes,"Playing% 3d%%  %03d",newpct,lcdsegs++);lcd.setCursor(0,0);lcd.print(PlayBytes);        
        #endif
        #ifdef OLED1306
    
            //sprintf(PlayBytes,"%03d",lcdsegs);
            //sendStrXY(PlayBytes,13,0);

            if (lcdsegs % 10 != 0) {setXY(15,0);sendChar(48+lcdsegs%10);} // ultima cifra 1,2,3,4,5,6,7,8,9
            else 
               if (lcdsegs % CNTRBASE != 0){setXY(14,0);sendChar(48+(lcdsegs%CNTRBASE)/10);sendChar('0');} // es 10,20,30,40,50,60,70,80,90,110,120,..
               else 
                  if (lcdsegs % (CNTRBASE*10) != 0) {setXY(13,0);sendChar(48+(lcdsegs% (CNTRBASE*10))/CNTRBASE);sendChar('0');sendChar('0');} // es 100,200,300,400,500,600,700,800,900,1100,..
                  else {setXY(13,0);sendChar('0');sendChar('0');sendChar('0');} // es 000,1000,2000,...

           lcdsegs++;        
          //if (lcdsegs == 1000) lcdsegs = 0;
                     
 /*         PlayBytes[0]='\0';
          u8g.firstPage();
          do { 
            switch(offset){
              case 0:         
                utoa(lcdsegs,PlayBytes,10);            
                break;
              case 1:
                strcat_P(PlayBytes,PSTR("0"));utoa(lcdsegs,PlayBytes+1,10);            
                break;
              case 2:
                strcat_P(PlayBytes,PSTR("00"));utoa(lcdsegs,PlayBytes+2,10);
                break;          
            }
            //u8g.drawStr( 8*13, 15, PlayBytes);
            u8g.setPrintPos(8*13, 15);
            u8g.print(PlayBytes);    
          } while( u8g.nextPage() );
          lcdsegs++ ;  */
      
          //sprintf(PlayBytes,"Playing %d\% %03d",newpct,lcdsegs++);
  //        sprintf(PlayBytes,"Playing% 3d%%  %03d",newpct,lcdsegs++);
        /*  u8g.firstPage();
          do { 
             u8g.setPrintPos(0,15);
             //u8g.print(PlayBytes);
             u8g.print("Playing "+ String(newpct) + "%" + "  " + String(lcdsegs++));
             u8g.setPrintPos(0,30);
             u8g.print(line1);    
          } while( u8g.nextPage() ); */

   //        printtext(PlayBytes,0);         
        #endif

        #ifdef P8544
        if (lcdsegs % 10 != 0) {itoa(lcdsegs%10,PlayBytes,10);
        lcd.setCursor(13,3);
        lcd.print(PlayBytes);} // ultima cifra 1,2,3,4,5,6,7,8,9
            else 
               if (lcdsegs % CNTRBASE != 0){itoa(lcdsegs%CNTRBASE,PlayBytes,10);
               lcd.setCursor(12,3);
               lcd.print(PlayBytes);} // es 10,20,30,40,50,60,70,80,90,110,120,..
               else 
                  if (lcdsegs % (CNTRBASE*10) != 0) {itoa(lcdsegs%(CNTRBASE*10)/CNTRBASE*100,PlayBytes,10);
                  lcd.setCursor(11,3);
                  lcd.print(PlayBytes);} // es 100,200,300,400,500,600,700,800,900,1100,..
                  else {
                    lcd.setCursor(11,3);
                    lcd.print("000");} // es 000,1000,2000,...
/*
          if (lcdsegs % 10 != 0) {lcd.setCursor(15,0);lcd.print(lcdsegs%10);} // ultima cifra 1,2,3,4,5,6,7,8,9
          else 
             if (lcdsegs % 100 != 0){lcd.setCursor(14,0);lcd.print(lcdsegs%100);}  // es 10,20,30,40,50,60,70,80,90,110,120,..
             else 
                if (lcdsegs %1000 != 0) {lcd.setCursor(13,0);lcd.print(lcdsegs%1000);}  // es 100,200,300,400,500,600,700,800,900,1100,..
                else {lcd.setCursor(13,0);lcd.print("000");}   // es 000,1000,2000,...
*/

           lcdsegs++;
          //if (lcdsegs == 1000) lcdsegs = 0; 
          //itoa(lcdsegs++,PlayBytes,10);lcd.setCursor(13,0);lcd.print(PlayBytes);          
    /*      switch(offset){
            case 0:
              break;
            case 1:
              //lcd.print("0");
              break;
            case 2:
              //lcd.print("00");
              break;          
          }  */
          //lcd.print(lcdsegs++); 
          //sprintf(PlayBytes,"%03d",lcdsegs++); lcd.print(PlayBytes);                     
           //sprintf(PlayBytes,"Playing% 3d%%  %03d",newpct,lcdsegs++);lcd.setCursor(0,0);lcd.print(PlayBytes);        
        #endif
          
    }
}

void lcdPercent() {  
    newpct=(100 * bytesRead)/filesize;                   
    if (currpct ==100){
        currpct= 0;
        #ifdef LCDSCREEN16x2  
          lcd.setCursor(8,0);
          lcd.print(newpct);lcd.print("%");  
          //lcdSpinner(); 
        #endif
        #ifdef OLED1306
          //strcat_P(PlayBytes,PSTR("%"));
          
          if (newpct <10) {setXY(8,0);sendChar(48+newpct%10);}
          else
            if (newpct <100){setXY(8,0);sendChar(48+newpct/10); sendChar(48+newpct%10);}
            else {setXY(8,0);sendChar('1');sendChar('0');sendChar('0');}
            
          sendChar('%');
     
  /*            u8g.firstPage();
          do {  
              //u8g.drawStr( 8*8, 15, PlayBytes);
              u8g.setPrintPos(8*8, 15);
              u8g.print(String(newpct,DEC)+F(" %"));    
          } while( u8g.nextPage() ); */
        #endif
  
        #ifdef P8544
          lcd.setCursor(0,3);
          lcd.print(newpct);
          lcd.print("%");               
        #endif
                             
    }
    if ((newpct >currpct)&& (newpct % 1 == 0)) {
        //PlayBytes[0]='\0'; 
        //strcat_P(PlayBytes,PSTR("Playing "));itoa(newpct,PlayBytes+8,10);strcat_P(PlayBytes,PSTR("%"));
        //printtext(PlayBytes,0);
         #ifdef LCDSCREEN16x2            
            lcd.setCursor(8,0);
            lcd.print(newpct);lcd.print("%"); 
         #endif             
        #ifdef OLED1306
        
          if (newpct <10) {setXY(8,0);sendChar(48+newpct%10);}
          else
            if (newpct <100){setXY(8,0);sendChar(48+newpct/10); sendChar(48+newpct%10);}
            else {setXY(8,0);sendChar('1');sendChar('0');sendChar('0');}
            
          sendChar('%');
  /*           u8g.firstPage();
        do {  
          //u8g.drawStr( 8*8, 15, PlayBytes); 
          u8g.setPrintPos(8*8, 15);
          u8g.print(String(newpct,DEC)+F(" %"));                 
        } while( u8g.nextPage() ); */
        #endif
  
        #ifdef P8544
          lcd.setCursor(0,3);
          lcd.print(newpct);
          lcd.print("%");  
        #endif
        
        currpct = newpct;
        //lcdSpinner();   
    } 
}

