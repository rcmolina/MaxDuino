#include "configs.h"
#include "i2c_config.h"
#include "i2c.h"

#if (I2C_Library_Used == _I2C_Impl_SoftI2CMaster)
  #undef USE_SOFT_I2C_MASTER_H_AS_PLAIN_INCLUDE
  #undef _SOFTI2C_HPP
  #include <SoftI2CMaster.h>
#elif (I2C_Library_Used == _I2C_Impl_SoftWire)
  #include <SoftWire.h>
  SoftWire Wire = SoftWire();  
#else
  #include <Wire.h>
#endif
