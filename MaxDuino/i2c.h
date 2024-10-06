// wrappers around different i2c implementations to expose the same interface to library code

#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#if defined(Use_SoftI2CMaster)
  #define mx_i2c_init() i2c_init()
  #define mx_i2c_start(address) i2c_start((address<<1)|I2C_WRITE)
  #define mx_i2c_write(byte) i2c_write(byte)
  #define mx_i2c_end() i2c_stop()
#else
  #define mx_i2c_init() Wire.begin();\
                        Wire.setClock(I2CCLOCK)
  #define mx_i2c_start(address) Wire.beginTransmission(address)
  #define mx_i2c_write(byte) Wire.write(byte)
  #define mx_i2c_end() Wire.endTransmission()
#endif

#endif // I2C_H_INCLUDED