// wrappers around different i2c implementations to expose the same interface to library code

#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include "configs.h"
#include "Arduino.h"
#include "i2c_config.h"

#if defined(I2CFAST)
  #define I2C_FASTMODE  1
  #define I2CCLOCK  400000L   //100000L for StandarMode, 400000L for FastMode and 1000000L for FastModePlus
#else
  #define I2C_FASTMODE  0
  #define I2CCLOCK  100000L   //100000L for StandarMode, 400000L for FastMode and 1000000L for FastModePlus
#endif

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega328P__)
  #if I2C_Library_Preference == _I2C_Impl_SoftI2CMaster
    #define I2C_Library_Used (_I2C_Impl_SoftI2CMaster)
  #elif I2C_Library_Preference == _I2C_Impl_SoftWire
    #define I2C_Library_Used (_I2C_Impl_SoftWire)
  #else
    #warning Wire library uses a lot of firmware space, you would be better off using SoftI2CMaster
    #define I2C_Library_Used (_I2C_Impl_Wire)
  #endif
#else
  #if I2C_Library_Preference == _I2C_Impl_SoftI2CMaster
    #warning This chip does not support SoftI2CMaster. Using default.
  #endif
  #if I2C_Library_Preference == _I2C_Impl_SoftWire
    #warning This chip does not support SoftWire. Using default.
  #endif
  #define I2C_Library_Used (_I2C_Impl_Wire)
#endif

#if (defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__))
  #define SDA_PORT PORTD
  #define SDA_PIN 1 
  #define SCL_PORT PORTD
  #define SCL_PIN 0 
#else
  #define SDA_PORT PORTC
  #define SDA_PIN 4 
  #define SCL_PORT PORTC
  #define SCL_PIN 5 
#endif

#if (I2C_Library_Used == _I2C_Impl_SoftI2CMaster)
  // ideally I'd do this:
  //     #define USE_SOFT_I2C_MASTER_H_AS_PLAIN_INCLUDE
  //     #include <SoftI2CMaster.h>
  // but as of 2.1.9 it appears it still doesn't work properly (at least not with platform io)
  // so I'll forward-declare everything myself:
  #ifndef I2C_WRITE
  #define I2C_WRITE 0
  #endif
  bool __attribute__ ((noinline)) i2c_init(void) __attribute__ ((used));
  bool __attribute__ ((noinline)) i2c_start(uint8_t addr) __attribute__ ((used));
  void __attribute__ ((noinline)) i2c_stop(void) asm("ass_i2c_stop") __attribute__ ((used));
  bool __attribute__ ((noinline)) i2c_write(uint8_t value) asm("ass_i2c_write") __attribute__ ((used));

  #define mx_i2c_init() i2c_init()
  #define mx_i2c_start(address) i2c_start((address<<1)|I2C_WRITE)
  #define mx_i2c_write(byte) i2c_write(byte)
  #define mx_i2c_end() i2c_stop()

#else
  // Wire or SoftWire
  #if (I2C_Library_Used == _I2C_Impl_SoftWire)
    extern SoftWire Wire;
  #elif (I2C_Library_Used == _I2C_Impl_Wire)
    #include <Wire.h>
  #else
    #error Unknown I2C library configuration
  #endif

  #define mx_i2c_init() Wire.begin();\
                        Wire.setClock(I2CCLOCK)
  #define mx_i2c_start(address) Wire.beginTransmission(address)
  #define mx_i2c_write(byte) Wire.write(byte)
  #define mx_i2c_end() Wire.endTransmission()

#endif

#if defined(P8544)
  #include <pcd8544.h>
  #define ADMAX 1023
  #define ADPIN 0
  #include <avr/pgmspace.h>
  byte dc_pin = 5;
  byte reset_pin = 3;
  byte cs_pin = 4;
  #define backlight_pin 2
#endif

#endif // I2C_H_INCLUDED
