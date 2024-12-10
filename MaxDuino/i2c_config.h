#ifndef I2C_CONFIG_H_INCLUDED
#define I2C_CONFIG_H_INCLUDED

#include "constants.h"

// preferred I2C implementation
#ifndef I2C_Library_Preference
#define I2C_Library_Preference _I2C_Impl_SoftI2CMaster
//#define I2C_Library_Preference _I2C_Impl_SoftWire
#endif

#define I2CFAST // if defined, I2C bus runs at 400kHz, instead of 100kHz default

#endif // I2C_CONFIG_H_INCLUDED
