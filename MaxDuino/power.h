#ifndef POWER_H_INCLUDED
#define POWER_H_INCLUDED

#include "configs.h"

#ifdef SOFT_POWER_OFF
void check_power_off_key();
void clear_power_off();
void power_off();
#endif

#endif // POWER_H_INCLUDED
