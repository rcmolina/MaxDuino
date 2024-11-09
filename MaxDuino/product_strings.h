#ifndef PRODUCT_STRINGS_H_INCLUDED
#define PRODUCT_STRINGS_H_INCLUDED

#include "Arduino.h"
#include "version.h"
#include "utils.h"

const char P_VERSION[] PROGMEM = XXSTR(_VERSION);
const char P_PRODUCT_NAME[] PROGMEM = "MaxDuino OTLA";

#endif // PRODUCT_STRINGS_H_INCLUDED