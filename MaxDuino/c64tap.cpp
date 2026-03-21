#include "configs.h"
#include "compat.h"
#include "c64tap.h"
#include "MaxDuino.h"
#include "MaxProcessing.h"
#include "file_utils.h"
#include "isr.h"
#include "pinSetup.h"
#include "processing_state.h"

#ifdef Use_c64
namespace {

const uint32_t C64_PAL_CLOCK_HZ = 985248UL; // testing: *0,998=983248UL;
const uint32_t C64_NTSC_CLOCK_HZ = 1022727UL;
const uint32_t C64_UNIT_CYCLES = 8UL;
const uint16_t C64_HEADER_SIZE = 20;
const uint8_t C64_VERSION_OFFSET = 12;
const uint8_t C64_VIDEO_OFFSET = 13;
const uint16_t C64_DEFAULT_OVERFLOW_CYCLES = 256U * C64_UNIT_CYCLES;
PROGMEM const char C64TapSignature[] = "C64-TAPE-RAW";

uint8_t c64tapVersion = 0;
uint32_t c64tapClockHz = C64_PAL_CLOCK_HZ;
bool c64tapWholeWave = true;
uint32_t c64tapPendingPulseUs = 0;
uint32_t c64tapQueuedSecondHalfUs = 0;
bool c64tapPendingPulseNeedsEdge = false;

uint32_t cycles_to_us(uint32_t cycles) {
  if (cycles == 0) {
    return 1;
  }

  //uint32_t usec = ((cycles * 1000000UL) + (c64tapClockHz / 2UL)) / c64tapClockHz;
  uint32_t usec = ((cycles * 2000000UL +c64tapClockHz)>>1)/c64tapClockHz;
  if (usec == 0) {
    usec = 1;
  }
  return usec;
}

bool read_pulse_cycles(uint32_t &cycles) {
  if (!ReadByte()) {
    return false;
  }

  if (outByte != 0) {
    cycles = (uint32_t)outByte * C64_UNIT_CYCLES;
    return true;
  }

  if (c64tapVersion == 0) {
    cycles = C64_DEFAULT_OVERFLOW_CYCLES;
    return true;
  }

  if (!ReadLong()) {
    return false;
  }

  cycles = outLong;
  return true;
}

void queue_whole_wave(uint32_t totalUs) {
  uint32_t firstHalf = totalUs / 2U;
  uint32_t secondHalf = totalUs - firstHalf;

  if (firstHalf == 0) {
    firstHalf = 1;
    if (secondHalf > 1) {
      secondHalf -= 1;
    }
  }
  if (secondHalf == 0) {
    secondHalf = 1;
  }

  c64tapPendingPulseUs = firstHalf;
  c64tapQueuedSecondHalfUs = secondHalf;
/*
if (totalUs*c64tapClockHz/1000000 < C64_DEFAULT_OVERFLOW_CYCLES)
  c64tapPendingPulseNeedsEdge = true;
else
  c64tapPendingPulseNeedsEdge = false;
*/
  c64tapPendingPulseNeedsEdge = true;
}
void queue_half_wave(uint32_t totalUs) {
  c64tapPendingPulseUs = totalUs;
  c64tapPendingPulseNeedsEdge = true;
}

void emit_pending_pulse_chunk() {
  if (c64tapPendingPulseNeedsEdge) {
    uint16_t edgeUs = (c64tapPendingPulseUs > 0x3FFFUL) ? 0x3FFFU : (uint16_t)c64tapPendingPulseUs;
    currentPeriod = edgeUs;
    c64tapPendingPulseUs -= edgeUs;
    c64tapPendingPulseNeedsEdge = false;
    return;
  }

  uint16_t holdUs = (c64tapPendingPulseUs > HOLD_SIGNAL_MAX_US) ? HOLD_SIGNAL_MAX_US : (uint16_t)c64tapPendingPulseUs;
  currentPeriod = HOLD_SIGNAL_FLAG | holdUs;
  c64tapPendingPulseUs -= holdUs;
}

}

bool c64tap_is_valid() {
  return readfile(C64_HEADER_SIZE, 0) == C64_HEADER_SIZE &&
         memcmp_P(filebuffer, C64TapSignature, sizeof("C64-TAPE-RAW") - 1) == 0;
}

void c64tap_init() {
  readfile(C64_HEADER_SIZE, 0);
  c64tapVersion = filebuffer[C64_VERSION_OFFSET];
/*
#ifdef C64_FORCE_V1
  // Force legacy whole-wave playback with V1-style extended pulse handling.
  c64tapVersion = 1;
#endif
*/
  c64tapClockHz = (filebuffer[C64_VIDEO_OFFSET] == 1) ? C64_NTSC_CLOCK_HZ : C64_PAL_CLOCK_HZ;
  c64tapWholeWave = (c64tapVersion != 2);
  c64tapPendingPulseUs = 0;
  c64tapQueuedSecondHalfUs = 0;
  c64tapPendingPulseNeedsEdge = false;
  bytesRead = C64_HEADER_SIZE;
  currentTask = TASK::PROCESSID;
  currentID = BLOCKID::C64TAP;
  currentBlockTask = BLOCKTASK::TDATA;
  EndOfFile = false;

/*
#ifdef c64_invert
  pinState = HIGH;
  WRITE_HIGH;
#else
  pinState = LOW;
  WRITE_LOW;
#endif
*/
}

void tzx_process_blockid_c64tap() {
  if (c64tapPendingPulseUs > 0) {
    emit_pending_pulse_chunk();
    return;
  }

  if (c64tapQueuedSecondHalfUs > 0) {
    c64tapPendingPulseUs = c64tapQueuedSecondHalfUs;
    c64tapQueuedSecondHalfUs = 0;
    c64tapPendingPulseNeedsEdge = true;
    emit_pending_pulse_chunk();
    return;
  }

  uint32_t cycles = 0;
  if (!read_pulse_cycles(cycles)) {
    EndOfFile = true;
    currentID = BLOCKID::IDEOF;
    return;
  }

  uint32_t totalUs = cycles_to_us(cycles);
  if (c64tapWholeWave) {
    queue_whole_wave(totalUs);
  } else {
    queue_half_wave(totalUs);
  }
  emit_pending_pulse_chunk();
}
#endif
