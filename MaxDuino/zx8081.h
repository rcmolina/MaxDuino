#ifndef ZX8081_H_INCLUDED
#define ZX8081_H_INCLUDED

#include "Arduino.h"

//ZX81 Standards
#define ZX80PULSE                 160
#define ZX80TURBOPULSE            120

#define ZX80BITGAP                1442
#define ZX80TURBOBITGAP           500

void ZX8081DataBlock();

void tzx_process_blockid_zx8081_zxp();
void tzx_process_blockid_zx8081_zxo();

#endif //  ZX8081_H_INCLUDED