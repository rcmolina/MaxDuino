#include "configs.h"
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

enum CAQ_STAGE : uint8_t {
  START_BIT,
  DATA_BITS,
  STOP_BIT1,
  STOP_BIT2,
  DONE
};

static CAQ_STAGE caq_stage = DONE;

// Ensure CAQ timing is always 600 baud, regardless of the global MaxDuino baud setting.
static word caq_saved_baudrate = 0;
static bool caq_baudrate_overridden = false;

static byte caq_cur_byte = 0;
static bool caq_have_byte = false;

// bit index within data byte (MSB first; matches CAQ2WAV and Aquarius loaders)
static int8_t caq_bit_idx = 7;

// half-period toggles remaining for current bit (2 cycles = 4 half-periods)
static uint8_t caq_halves_left = 0;
static word caq_half_period = 0;

static inline void caq_begin_bit(bool bit_val) {
  caq_half_period = bit_val ? CAQ_MARK_HALF_US : CAQ_SPACE_HALF_US;
  caq_halves_left = 4;
}

// Pull next byte from file using core streaming helper.
// Returns false if no more bytes.
static bool caq_get_next_byte(byte &out) {
  // Use core file_utils streaming reader (advances bytesRead)
  if (!ReadByte()) return false;
  out = outByte;
  return true;
}

void caq_init() {
  caq_stage = START_BIT;
  caq_have_byte = false;
  caq_bit_idx = 7;
  caq_halves_left = 0;
  caq_half_period = CAQ_SPACE_HALF_US;

  // Force 600 baud for the duration of CAQ playback.
  // This keeps CAQ independent from the menu-configured BAUDRATE.
  if (!caq_baudrate_overridden) {
    caq_saved_baudrate = BAUDRATE;
    BAUDRATE = 600;
    caq_baudrate_overridden = true;
  }

  // Ensure we return to the main processing loop in CAQ mode
  currentID = BLOCKID::CAQ;
  currentTask = TASK::PROCESSID;
  count_r = 255;
}

void caq_process() {
  currentPeriod = 0;

  // If we're mid-bit, just emit the next half period
  if (caq_halves_left) {
    currentPeriod = caq_half_period;
    caq_halves_left--;
    return;
  }

  // Need a byte to send?
  if (!caq_have_byte) {
    if (!caq_get_next_byte(caq_cur_byte)) {
      caq_stage = DONE;
    } else {
      caq_have_byte = true;
      caq_stage = START_BIT;
      caq_bit_idx = 7;
    }
  }

  switch (caq_stage) {
    case START_BIT:
      // start bit = 0 (space)
      caq_begin_bit(false);
      caq_stage = DATA_BITS;
      currentPeriod = caq_half_period;
      caq_halves_left--;
      return;

    case DATA_BITS: {
      // MSB first (bit 7 down to bit 0), matching CAQ2WAV and Aquarius cassette loaders.
      bool bit_val = (caq_cur_byte >> caq_bit_idx) & 0x01;
      caq_begin_bit(bit_val);
      caq_bit_idx--;
      if (caq_bit_idx < 0)
        caq_stage = STOP_BIT1;
      currentPeriod = caq_half_period;
      caq_halves_left--;
      return;
    }

    case STOP_BIT1:
      // stop bits = 1 (mark)
      caq_begin_bit(true);
      caq_stage = STOP_BIT2;
      currentPeriod = caq_half_period;
      caq_halves_left--;
      return;

    case STOP_BIT2:
      caq_begin_bit(true);
      // after this, move to next byte
      caq_have_byte = false;
      caq_stage = START_BIT;
      currentPeriod = caq_half_period;
      caq_halves_left--;
      return;

    case DONE:
    default:
      // Restore previous baudrate setting (we force 600 only for CAQ playback).
      if (caq_baudrate_overridden) {
        BAUDRATE = caq_saved_baudrate;
        caq_baudrate_overridden = false;
      }
      // End of file: hand off to existing EOF handler (menu return)
      currentID = BLOCKID::IDEOF;
      currentTask = TASK::PROCESSID;
      count_r = 255;
      currentPeriod = 0;
      return;
  }
}
