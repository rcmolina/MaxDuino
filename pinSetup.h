#ifndef PINSETUP_H_INCLUDED
#define PINSETUP_H_INCLUDED

#ifdef __AVR_ATmega2560__
#define outputPin           23 
#define INIT_OUTPORT        DDRA |=  _BV(1)         // El pin23 es el bit1 del PORTA
#define WRITE_LOW           PORTA &= ~_BV(1)         // El pin23 es el bit1 del PORTA
#define WRITE_HIGH          PORTA |=  _BV(1)         // El pin23 es el bit1 del PORTA

#elif defined(__AVR_ATmega4809__)
  #define outputPin           9
  //#define INIT_OUTPORT         DDRB |=  _BV(1)         // El pin9 es el bit1 del PORTB
  //#define INIT_OUTPORT          pinMode(outputPin,OUTPUT)  
  #define INIT_OUTPORT         VPORTB.DIR |=  _BV(0)         // El pin9 es PB0
  //#define WRITE_LOW           PORTB &= ~_BV(1)         // El pin9 es el bit1 del PORTB
  //#define WRITE_LOW             digitalWrite(outputPin,LOW)
  #define WRITE_LOW           VPORTB.OUT &= ~_BV(0)         // El pin9 es PB0
  //#define WRITE_HIGH          PORTB |=  _BV(1)         // El pin9 es el bit1 del PORTB
  //#define WRITE_HIGH            digitalWrite(outputPin,HIGH)
  #define WRITE_HIGH          VPORTB.OUT |=  _BV(0)         // El pin9 es PB0

#elif defined(__AVR_ATmega4808__)
  #define outputPin           9
  //#define INIT_OUTPORT          pinMode(outputPin,OUTPUT)  
  #define INIT_OUTPORT         VPORTA.DIR |=  PIN7_bm         // El pin9 es PA7
  //#define WRITE_LOW             digitalWrite(outputPin,LOW)
  #define WRITE_LOW            VPORTA.OUT &= ~PIN7_bm         // El pin9 es PA7
  //#define WRITE_HIGH            digitalWrite(outputPin,HIGH)
  #define WRITE_HIGH           VPORTA.OUT |=  PIN7_bm         // El pin9 es PA7

#elif defined(__arm__) && defined(__STM32F1__)
  #define outputPin           PA9    // this pin is 5V tolerant and PWM output capable
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  //#define INIT_OUTPORT            pinMode(outputPin,OUTPUT); GPIOA->regs->CRH |=  0x00000030  
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  //#define WRITE_LOW               GPIOA->regs->ODR &= ~0b0000001000000000
  //#define WRITE_LOW               gpio_write_bit(GPIOA, 9, LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)
  //#define WRITE_HIGH              GPIOA->regs->ODR |=  0b0000001000000000
  //#define WRITE_HIGH              gpio_write_bit(GPIOA, 9, HIGH)

#elif defined(__AVR_ATmega32U4__) 
#define outputPin           7    // this pin is 5V tolerant and PWM output capable
//#define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  #define INIT_OUTPORT         DDRE |=  _BV(6)         // El pin PE6 es el bit6 del PORTE
//#define WRITE_LOW               digitalWrite(outputPin,LOW)
  #define WRITE_LOW           PORTE &= ~_BV(6)         // El pin PE6 es el bit6 del PORTE
//#define WRITE_HIGH              digitalWrite(outputPin,HIGH)
  #define WRITE_HIGH          PORTE |=  _BV(6)         // El pin PE6 es el bit6 del PORTE
  
#elif defined(SEEED_XIAO_M0)
  #define outputPin           0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  //#define INIT_OUTPORT            pinMode(outputPin,OUTPUT); GPIOA->regs->CRH |=  0x00000030  
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  //#define WRITE_LOW               GPIOA->regs->ODR &= ~0b0000001000000000
  //#define WRITE_LOW               gpio_write_bit(GPIOA, 9, LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)
  //#define WRITE_HIGH              GPIOA->regs->ODR |=  0b0000001000000000
  //#define WRITE_HIGH              gpio_write_bit(GPIOA, 9, HIGH)

#else  //__AVR_ATmega328P__
  //#define MINIDUINO_AMPLI     // For A.Villena's Miniduino new design
  #define outputPin           9
  #ifdef MINIDUINO_AMPLI
    #define INIT_OUTPORT         DDRB |= B00000011                              // pin8+ pin9 es el bit0-bit1 del PORTB 
    #define WRITE_LOW           (PORTB &= B11111101) |= B00000001               // pin8+ pin9 , bit0- bit1 del PORTB
    #define WRITE_HIGH          (PORTB |= B00000010) &= B11111110               // pin8+ pin9 , bit0- bit1 del PORTB  
  //  #define WRITE_LOW           PORTB = (PORTB & B11111101) | B00000001         // pin8+ pin9 , bit0- bit1 del PORTB
  //  #define WRITE_HIGH          PORTB = (PORTB | B00000010) & B11111110         // pin8+ pin9 , bit0- bit1 del PORTB 
  #else
    #define INIT_OUTPORT         DDRB |=  _BV(1)         // El pin9 es el bit1 del PORTB
    #define WRITE_LOW           PORTB &= ~_BV(1)         // El pin9 es el bit1 del PORTB
    #define WRITE_HIGH          PORTB |=  _BV(1)         // El pin9 es el bit1 del PORTB
  #endif

// pin 0-7 PortD0-7, pin 8-13 PortB0-5, pin 14-19 PortC0-5

/*
#ifdef rpolarity 
  #define WRITE_LOW           PORTB &= ~_BV(1)        // El pin9 es el bit1 del PORTB
  #define WRITE_HIGH          PORTB |= _BV(1)         // El pin9 es el bit1 del PORTB
  // pin 0-7 PortD0-7, pin 8-13 PortB0-5, pin 14-19 PortC0-5
#endif

#ifndef rpolarity 
  #define WRITE_HIGH           PORTB &= ~_BV(1)        // El pin9 es el bit1 del PORTB
  #define WRITE_LOW          PORTB |= _BV(1)         // El pin9 es el bit1 del PORTB
  // pin 0-7 PortD0-7, pin 8-13 PortB0-5, pin 14-19 PortC0-5
#endif
*/

#endif 
/////////////////////////////////////////////////////////////////////////////////////////////
  //General Pin settings
  //Setup buttons with internal pullup

#ifdef __AVR_ATmega2560__

  const byte chipSelect = 53;          //Sd card chip select pin
  
  #define btnUp         A0            //Up button
  #define btnDown       A1            //Down button
  #define btnPlay       A2            //Play Button
  #define btnStop       A3            //Stop Button
  #define btnRoot       A4            //Return to SD card root
  // #define btnDelete     A5         //Not implemented this button is for an optional function
  #define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)

#elif defined(__arm__) && defined(__STM32F1__)
//
// Pin definition for Blue Pill boards
//

#define chipSelect    PB12            //Sd card chip select pin

#define btnPlay       PA0           //Play Button
#define btnStop       PA1           //Stop Button
#define btnUp         PA2           //Up button
#define btnDown       PA3           //Down button
#define btnMotor      PA8     //Motor Sense (connect pin to gnd to play, NC for pause)
#define btnRoot       PA4           //Return to SD card root

#elif defined(__AVR_ATmega32U4__) 
  #define NO_MOTOR  
  const byte chipSelect = SS;          //Sd card chip select pin

  #define btnPlay       4            //Play Button
  #define btnStop       30            //Stop Button
  #define btnUp         6            //Up button
  #define btnDown       12            //Down button
  #define btnMotor      0             //Motor Sense (connect pin to gnd to play, NC for pause)
  #define btnRoot       1             //Return to SD card root
#elif defined(SEEED_XIAO_M0)
//
// Pin definition for Seeeduino Xiao M0 boards
//

#define chipSelect    12            //Sd card chip select pin - map to LED (on assumption that SD CS is actually just tied directly to GND)
#define BUTTON_ADC
#define btnADC        A3 // analog input pin for ADC buttons
#define NO_MOTOR    // because no spare gpio

#else
  const byte chipSelect = 10;          //Sd card chip select pin
  
  #define btnPlay       17            //Play Button
  #define btnStop       16            //Stop Button
  #define btnUp         15            //Up button
  #define btnDown       14            //Down button
  #define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)
  #define btnRoot       7             //Return to SD card root
#endif


#ifdef __AVR_ATmega2560__

  // Analog pin A0
  PORTF |= _BV(0);

  // Analog pin A1
  PORTF |= _BV(1);
  
  // Analog pin A2
  PORTF |= _BV(2);
  
  // Analog pin A3
  PORTF |= _BV(3);

  // Analog pin A4
  PORTF |= _BV(4);

  // Analog pin A5
  //PORTF |= _BV(5);

  // Digital pin 6
  PORTH |= _BV(3);

#elif defined(__AVR_ATmega4809__)
  //pinMode(btnPlay,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnPlay,HIGH); // 17 PD0
  //PORTD.PIN0CTRL |=0B00001000;
  PORTD.PIN0CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */

  //VPORTC.DIR |= ~PIN3_bm;
  //PORTC.DIR |= ~PIN3_bm; /* Configure PC3 as digital input */
  //PORTC.PIN3CTRL = PORT_PULLUPEN_bm; /* Enable the internal pullup */
  
    
  //digitalWrite(btnPlay,HIGH);
  //  VPORTC.OUT |= _BV(3);
  //PORTC |= _BV(3);
  
  //pinMode(btnStop,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnStop,HIGH); // 16 PD1
  PORTD.PIN1CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */  
  //VPORTC.OUT |= _BV(2);
  //PORTC |= _BV(2);

  //pinMode(btnUp,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnUp,HIGH); // 15 PD2
  PORTD.PIN2CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */    
  //VPORTC.OUT |= _BV(1);
  //PORTC |= _BV(1);

  //pinMode(btnDown,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnDown,HIGH); // 14 PD3 also to enbale PULLUP if PINMODE is INPUT
  PORTD.PIN3CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */    
  //VPORTC.OUT |= _BV(0);
  //PORTC |= _BV(0);

  //pinMode(btnMotor, INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnMotor,HIGH); // 6 PF4
  PORTF.PIN4CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */  
  //VPORTD.OUT |= _BV(btnMotor);
  //PORTD |= _BV(btnMotor);
  
  //pinMode(btnRoot, INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnRoot, HIGH); // 7 PA1 
  PORTA.PIN1CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  //VPORTD.OUT |= _BV(btnRoot); 
  //PORTD |= _BV(btnRoot);

#elif defined(__AVR_ATmega4808__)
  //pinMode(btnPlay,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnPlay,HIGH); // 17 PD3
  VPORTD.DIR |= ~PIN3_bm;
  PORTD.PIN3CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTD.OUT |=  PIN3_bm;

  //pinMode(btnStop,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnStop,HIGH); // 16 PD2
  VPORTD.DIR |= ~PIN2_bm;  
  PORTD.PIN2CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTD.OUT |=  PIN2_bm;    

  //pinMode(btnUp,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnUp,HIGH); // 15 PD1
  VPORTD.DIR |= ~PIN1_bm;
  PORTD.PIN1CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTD.OUT |=  PIN1_bm;    

  //pinMode(btnDown,INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnDown,HIGH); // 14 PD0 also to enbale PULLUP if PINMODE is INPUT
  VPORTD.DIR |= ~PIN0_bm;
  PORTD.PIN0CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */ 
  VPORTD.OUT |=  PIN0_bm;   

  //pinMode(btnMotor, INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnMotor,HIGH); // 6 PA4
  VPORTA.DIR |= ~PIN4_bm;
  PORTA.PIN4CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTA.OUT |=  PIN4_bm;  
  
  //pinMode(btnRoot, INPUT_PULLUP);  // Not needed, default is INPUT (0)
  //digitalWrite(btnRoot, HIGH); // 7 PA5
  VPORTA.DIR |= ~PIN5_bm; 
  PORTA.PIN5CTRL |=PORT_PULLUPEN_bm; /* Enable the internal pullup */
  VPORTA.OUT |=  PIN5_bm;
  
#elif defined(__arm__) && defined(__STM32F1__)

  //General Pin settings
  //Setup buttons with internal pullup 
  pinMode(btnPlay,INPUT_PULLUP);
  digitalWrite(btnPlay,HIGH);
  pinMode(btnStop,INPUT_PULLUP);
  digitalWrite(btnStop,HIGH);
  pinMode(btnUp,INPUT_PULLUP);
  digitalWrite(btnUp,HIGH);
  pinMode(btnDown,INPUT_PULLUP);
  digitalWrite(btnDown,HIGH);
  pinMode(btnMotor, INPUT_PULLUP);
  digitalWrite(btnMotor,HIGH);
  pinMode(btnRoot, INPUT_PULLUP);
  digitalWrite(btnRoot, HIGH); 

#elif defined(__AVR_ATmega32U4__) 
  
//  pinMode(btnPlay,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnPlay,HIGH); // Wrte for INPUT_PULLUP if input type is only INPUT
  PORTD |= _BV(4);
  
//  pinMode(btnStop,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnStop,HIGH);
  PORTD |= _BV(5);

//  pinMode(btnUp,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnUp,HIGH);
  PORTD |= _BV(7);

//  pinMode(btnDown,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnDown,HIGH);
  PORTD |= _BV(6);

//  pinMode(btnMotor, INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnMotor,HIGH);
  PORTD |= _BV(2);
  
//  pinMode(btnRoot, INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnRoot, HIGH); 
  PORTD |= _BV(3);

   
#elif defined (SEEED_XIAO_M0)

  // BUTTON PIN CONFIGURATION
  // n.a.
  
#else  //__AVR_ATmega328P__
  //pinMode(btnPlay,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnPlay,HIGH); // Wrte for INPUT_PULLUP if input type is only INPUT
  PORTC |= _BV(3);
  
  //pinMode(btnStop,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnStop,HIGH);
  PORTC |= _BV(2);

  //pinMode(btnUp,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnUp,HIGH);
  PORTC |= _BV(1);

  //pinMode(btnDown,INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnDown,HIGH);
  PORTC |= _BV(0);

  //pinMode(btnMotor, INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnMotor,HIGH);
  PORTD |= _BV(btnMotor);
  
  //pinMode(btnRoot, INPUT_PULLUP);  // Not needed, default is INPUT (0)
//  digitalWrite(btnRoot, HIGH); 
  PORTD |= _BV(btnRoot);
#endif

#ifdef BUTTON_ADC
// for a 10-bit ADC, each button is calibrated to the band between this value and the next value above (or 1023 for upper limit)
// The bands are intentionally set very wide, and far apart
// Each button acts as a voltage divider between 10k and the following resistors:
#define btnADCPlayLow 980 // 0 ohm
#define btnADCStopLow 900 // 1k ohm
#define btnADCRootLow 700 // 2.4k ohm
#define btnADCDownLow 400 // 10k ohm
#define btnADCUpLow 200 // 20k ohm
#endif

#endif // #define PINSETUP_H_INCLUDED
