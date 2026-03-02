#include "configs.h"

#ifdef Use_CAQ

#include "caq.h"
#include "MaxProcessing.h"
#include "processing_state.h"
#include "file_utils.h"
#include "current_settings.h"

// Aquarius cassette encoding (from Aquarius I/O map documentation):
// One byte: 1 start bit (0), 8 data bits, 2 stop bits (1).
// Each bit is encoded as 2 full cycles of square wave.
// "Mark" (logical 1) has period 0.6ms; "Space" (logical 0) has period 1.2ms.
// Since MaxDuino toggles the output on each stored "period", we store half-periods.
// Mark half-period = 0.3ms; Space half-period = 0.6ms.

// Hard timing for Aquarius CAQ playback.
// CAQ is a ~600 baud cassette format for the Mattel Aquarius.
// It uses two full waves per bit:
//   - Bit 1 (mark): shorter period (~0.6ms)  => half-period ~300us
//   - Bit 0 (space): longer period (~1.2ms)  => half-period ~600us
// Keep these constant and independent of MaxDuino's BAUDRATE/menu settings.
static const word CAQ_MARK_HALF_US  = 272;// ~0.544ms full wave (matches CAQ2WAV 22.05k settings)
static const word CAQ_SPACE_HALF_US = 544;// ~1.088ms full wave (matches CAQ2WAV 22.05k settings)
static word caq_half_period = 0;

static void caq_begin_bit(bool bit_val) {
  caq_half_period = bit_val ? CAQ_MARK_HALF_US : CAQ_SPACE_HALF_US;
  currentPeriod = caq_half_period;
  // half-period toggles remaining for current bit (2 cycles = 4 half-periods)
  // we (re)use pass variable and count down from 4 to 0
  pass = 4;
}

void caq_init() {
  // bit index within data byte (MSB first; matches CAQ2WAV and Aquarius loaders)
  // 0 means 'we need to read another byte now' (replaces caq_have_byte)
  currentBit = 0;
  pass = 0; // to indicate we are not mid-bit

  // Ensure we return to the main processing loop in CAQ mode
  currentID = BLOCKID::CAQ;
  currentTask = TASK::PROCESSID;
  //currentBlockTask = BLOCKTASK::READPARAM;  // unnecessary since first blocktask is ALWAYS readparam (see fundctions UniPlay() and SetPlayBlock() )
  count_r = 255;
}

void caq_process() {
  currentPeriod = caq_half_period;

  // If we've finished emitting 4-half-periods (pass==0), get the next bits ready
  // If we're mid-bit (pass!=0), just emit the next half period

  if (pass == 0)
  {
    // Need a byte to send?
    if (currentBlockTask == BLOCKTASK::CAQ_START_BIT && currentBit == 0) {
      // Pull next byte from file using core streaming helper.
      // Returns false if no more bytes.
      // Use core file_utils streaming reader (advances bytesRead)
      if (!ReadByte()) {
        currentBlockTask = BLOCKTASK::CAQ_DONE;
      } else {
        currentByte = outByte;
        currentBit = 8;
      }
    }

    switch (currentBlockTask) {
      case BLOCKTASK::READPARAM: // no-op , straight into START_BIT
      case BLOCKTASK::CAQ_START_BIT:
        // start bit = 0 (space)
        caq_begin_bit(false);
        currentBlockTask = BLOCKTASK::CAQ_DATA_BITS;
        break;

      case BLOCKTASK::CAQ_DATA_BITS: {
        // MSB first (bit 7 down to bit 0), matching CAQ2WAV and Aquarius cassette loaders.
        caq_begin_bit(currentByte & 0x80);
        currentByte <<= 1;
        currentBit--;
        if (currentBit == 0)
          currentBlockTask = BLOCKTASK::CAQ_STOP_BIT1;
        break;
      }

      case BLOCKTASK::CAQ_STOP_BIT1:
        // stop bits = 1 (mark)
        caq_begin_bit(true);
        currentBlockTask = BLOCKTASK::CAQ_STOP_BIT2;
        break;

      case BLOCKTASK::CAQ_STOP_BIT2:
        caq_begin_bit(true);
        // after this, move to next byte
        currentBlockTask = BLOCKTASK::CAQ_START_BIT;
        break;

      case BLOCKTASK::CAQ_DONE:
      default:
        // End of file: hand off to existing EOF handler (menu return)
        currentID = BLOCKID::IDEOF;
        currentTask = TASK::PROCESSID;
        count_r = 255;
        currentPeriod = 0;
        break;
    }
  }

  pass -= 1;
}

#endif // Use_CAQ