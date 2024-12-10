#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#include "Arduino.h"

#define _I2C_Impl_SoftI2CMaster (0)
#define _I2C_Impl_SoftWire (1)
#define _I2C_Impl_Wire (2)


enum class CASDUINO_FILETYPE : byte {
  NONE = 0,
  CASDUINO = 11, // number of bits
  DRAGONMODE = 8, // number of bits
};


enum class CAS_TYPE : byte {
  Nothing=0,
  Ascii,
  Binf,
  Basic,
  Unknown,
  typeEOF,
};

#endif // CONSTANTS_H_INCLUDED
