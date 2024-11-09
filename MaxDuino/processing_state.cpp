#include "processing_state.h"

byte currentID=BLOCKID::UNKNOWN;
TASK currentTask=TASK::GETFILEHEADER;
BLOCKTASK currentBlockTask = BLOCKTASK::READPARAM;
byte pass=0;
long count_r = 0;
byte currentBit=0;
