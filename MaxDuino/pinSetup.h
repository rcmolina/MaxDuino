#ifndef PINSETUP_H_INCLUDED
#define PINSETUP_H_INCLUDED

#include "Arduino.h"

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
  #define outputPin           A0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  //#define INIT_OUTPORT            pinMode(outputPin,OUTPUT); GPIOA->regs->CRH |=  0x00000030  
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  //#define WRITE_LOW               GPIOA->regs->ODR &= ~0b0000001000000000
  //#define WRITE_LOW               gpio_write_bit(GPIOA, 9, LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)
  //#define WRITE_HIGH              GPIOA->regs->ODR |=  0b0000001000000000
  //#define WRITE_HIGH              gpio_write_bit(GPIOA, 9, HIGH)

#elif defined(ARDUINO_XIAO_ESP32C3)
  #define outputPin         D0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
  #define outputPin           16 // D0
  #define INIT_OUTPORT            pinMode(outputPin,OUTPUT)
  #define WRITE_LOW               digitalWrite(outputPin,LOW)
  #define WRITE_HIGH              digitalWrite(outputPin,HIGH)

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
#define btnADC        A2 // analog input pin for ADC buttons
#define NO_MOTOR    // because no spare gpio

#elif defined(ARDUINO_XIAO_ESP32C3)
//
// Pin definition for Seeeduino Xiao ESP32C3 boards
//

#define chipSelect    D7
#define BUTTON_ADC
#define btnADC        A2 // analog input pin for ADC buttons // CHANGED!!!
#define NO_MOTOR    // because no spare gpio

#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI)
//
// Pin definition for Wemos D1 Mini (ESP8266) boards
//
#define chipSelect    15
#define BUTTON_ADC
#define btnADC        A0 
#define btnMotor      2

#else
  const byte chipSelect = 10;          //Sd card chip select pin
  
  #define btnPlay       17            //Play Button
  #define btnStop       16            //Stop Button
  #define btnUp         15            //Up button
  #define btnDown       14            //Down button
  #define btnMotor      6             //Motor Sense (connect pin to gnd to play, NC for pause)
  #define btnRoot       7             //Return to SD card root
#endif

void pinsetup();

#ifdef BUTTON_ADC
// Each button acts as a voltage divider between 10k and the following resistors:
// 0 Ohm  i.e. 100%
// 2.2k Ohm i.e. 82% (10 : 12.2)
// 4.7k Ohm i.e. 68% (10 : 14.7)
// 10k Ohm i.e. 50% (10 : 20)
// 20k Ohm i.e. 33% (10 : 30)

// For a 10-bit ADC, each button is calibrated to the band between this value and the next value above
// (or 1023 for upper limit).
// The bands are intentionally set very wide, and far apart
// However note that ESP ADC is nonlinear and not full-scale, so the resistor
// values must be chosen to avoid ranges at the extreme top (100%) end.
// The resistor values and bands chosen here are compatible with ESP devices

#if defined(ESP32) || defined(ESP8266)
// ESP ADC is nonlinear, and also not full scale, so the values are different!
// because not full scale, a 1k:10k voltage divider (i.e. 90%) is undetectable
// and reads as 1023 still, so resistor values have been altered to create better spacing
#define btnADCPlayLow 1020 // 0 ohm reading 1023 due to saturation
#define btnADCStopLow 900 // 2.2k ohm reading around 960
#define btnADCRootLow 700 // 4.7k ohm reading around 800
#define btnADCDownLow 500 // 10k ohm reading around 590
#define btnADCUpLow 200 // 20k ohm reading around 390
#else
#define btnADCPlayLow 950 // 0 ohm reading around 1000, ideally 1023
#define btnADCStopLow 800 // 2.2k ohm reading around 840
#define btnADCRootLow 600 // 4.7k ohm reading around 695
#define btnADCDownLow 420 // 10k ohm reading around 510
#define btnADCUpLow 200 // 20k ohm reading around 340
#endif

#endif // BUTTON_ADC

#endif // #define PINSETUP_H_INCLUDED
