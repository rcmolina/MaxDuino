#include "c64tap.h"

#ifdef Use_c64

#include <string.h>

#include "file_utils.h"
#include "processing_state.h"
#include "MaxDuino.h"
#include "MaxProcessing.h"

namespace {

constexpr byte C64TAP_HEADER_SIZE = 20;
constexpr byte C64TAP_VERSION_V0 = 0;
constexpr byte C64TAP_VERSION_V2 = 2;
constexpr word C64TAP_MAX_INLINE_US = 0x3FFF;
constexpr word C64TAP_LONG_PULSE_OPCODE = 0xC000;

constexpr byte C64TAP_MACHINE_C64 = 0;
constexpr byte C64TAP_MACHINE_VIC = 1;
constexpr byte C64TAP_MACHINE_C16 = 2;

constexpr byte C64TAP_VIDEO_PAL = 0;

const char C64TapMagicC64[] PROGMEM = "C64-TAPE-RAW";
const char C64TapMagicC16[] PROGMEM = "C16-TAPE-RAW";

byte c64tapVersion = 0;
byte c64tapMachine = C64TAP_MACHINE_C64;
byte c64tapVideo = C64TAP_VIDEO_PAL;
unsigned long c64tapEndPos = C64TAP_HEADER_SIZE;
unsigned long c64tapSavedPeriod = 0;
bool c64tapEmitSavedPeriod = false;
unsigned long c64tapPendingLongPeriodUs = 0;
byte c64tapPendingLongStage = 0;

bool header_matches(const byte *header, const char *magic) {
  for (byte i = 0; i < 12; ++i) {
    if (header[i] != pgm_read_byte(magic + i)) {
      return false;
    }
  }
  return true;
}

unsigned long read_le32(const byte *ptr) {
  return ((unsigned long)ptr[0])
    | ((unsigned long)ptr[1] << 8)
    | ((unsigned long)ptr[2] << 16)
    | ((unsigned long)ptr[3] << 24);
}

unsigned long cycles_per_second() {
  switch (c64tapMachine) {
    case C64TAP_MACHINE_C16:
      return (c64tapVideo == C64TAP_VIDEO_PAL) ? 886724UL : 894886UL;
    case C64TAP_MACHINE_VIC:
      return (c64tapVideo == C64TAP_VIDEO_PAL) ? 1108404UL : 1022727UL;
    case C64TAP_MACHINE_C64:
    default:
      return (c64tapVideo == C64TAP_VIDEO_PAL) ? 985248UL : 1022272UL;
  }
}

unsigned long cycles_to_us(const unsigned long cycles) {
  const unsigned long cps = cycles_per_second();
  const unsigned long long numerator = (unsigned long long)cycles * 1000000ULL + (cps / 2);
  const unsigned long periodUs = (unsigned long)(numerator / cps);
  return (periodUs == 0) ? 1UL : periodUs;
}

unsigned long clamp_half_period(const unsigned long fullPeriodUs) {
  unsigned long halfPeriodUs = fullPeriodUs / 2;
  if (halfPeriodUs == 0) {
    halfPeriodUs = 1UL;
  }
  return halfPeriodUs;
}

bool read_next_period(unsigned long &periodUs) {
  if (bytesRead >= c64tapEndPos) {
    return false;
  }

  if (!ReadByte()) {
    return false;
  }

  unsigned long cycles = 0;
  if (outByte != 0) {
    cycles = (unsigned long)outByte * 8UL;
  } else if (c64tapVersion == C64TAP_VERSION_V0) {
    cycles = 256UL * 8UL;
  } else {
    if (!ReadLong()) {
      return false;
    }
    cycles = outLong;
  }

  periodUs = cycles_to_us(cycles);
  return true;
}

void set_header_state(const byte *header, unsigned long fileSize) {
  c64tapVersion = header[12];
  c64tapMachine = header_matches(header, C64TapMagicC16) ? C64TAP_MACHINE_C16 : header[13];
  if (c64tapMachine > C64TAP_MACHINE_C16) {
    c64tapMachine = header_matches(header, C64TapMagicC16) ? C64TAP_MACHINE_C16 : C64TAP_MACHINE_C64;
  }
  c64tapVideo = (header[14] == 1) ? 1 : C64TAP_VIDEO_PAL;
  const unsigned long declaredLength = read_le32(header + 16);
  const unsigned long availableLength = (fileSize >= C64TAP_HEADER_SIZE) ? (fileSize - C64TAP_HEADER_SIZE) : 0;
  const unsigned long dataLength = (declaredLength <= availableLength) ? declaredLength : availableLength;
  c64tapEndPos = C64TAP_HEADER_SIZE + dataLength;
  c64tapSavedPeriod = 0;
  c64tapEmitSavedPeriod = false;
  c64tapPendingLongPeriodUs = 0;
  c64tapPendingLongStage = 0;
}

void begin_long_pulse_output(const unsigned long periodUs) {
  c64tapPendingLongPeriodUs = periodUs;
  c64tapPendingLongStage = 2;
  currentPeriod = C64TAP_LONG_PULSE_OPCODE;
}

void emit_period(const unsigned long periodUs) {
  if (periodUs <= C64TAP_MAX_INLINE_US) {
    currentPeriod = (word)periodUs;
  } else {
    begin_long_pulse_output(periodUs);
  }
}

bool emit_pending_long_pulse_word() {
  if (c64tapPendingLongStage == 0) {
    return false;
  }

  if (c64tapPendingLongStage == 2) {
    currentPeriod = (word)(c64tapPendingLongPeriodUs & 0xFFFFUL);
    c64tapPendingLongStage = 1;
  } else {
    currentPeriod = (word)((c64tapPendingLongPeriodUs >> 16) & 0xFFFFUL);
    c64tapPendingLongPeriodUs = 0;
    c64tapPendingLongStage = 0;
  }

  return true;
}

} // namespace

bool c64tap_is_header(const byte *header, unsigned long fileSize) {
  if (fileSize < C64TAP_HEADER_SIZE) {
    return false;
  }

  const bool isC64 = header_matches(header, C64TapMagicC64);
  const bool isC16 = header_matches(header, C64TapMagicC16);
  if (!isC64 && !isC16) {
    return false;
  }

  if (header[12] > C64TAP_VERSION_V2) {
    return false;
  }

  set_header_state(header, fileSize);
  return true;
}

void c64tap_init() {
  bytesRead = C64TAP_HEADER_SIZE;
  currentTask = TASK::PROCESSID;
  currentID = BLOCKID::C64TAP;
  currentBlockTask = BLOCKTASK::READPARAM;
}

void c64tap_process() {
  if (emit_pending_long_pulse_word()) {
    return;
  }

  if (c64tapEmitSavedPeriod) {
    emit_period(c64tapSavedPeriod);
    c64tapEmitSavedPeriod = false;
    return;
  }

  unsigned long periodUs = 0;
  if (!read_next_period(periodUs)) {
    currentID = BLOCKID::IDEOF;
    return;
  }

  // TAP v0/v1 stores the time between successive edges, i.e. a full pulse period.
  // MaxDuino emits one timer interval per edge transition, so those formats must
  // be split into two equal half-waves. TAP v2 already stores half-wave lengths.
  if (c64tapVersion != C64TAP_VERSION_V2) {
    const unsigned long halfPeriodUs = clamp_half_period(periodUs);
    emit_period(halfPeriodUs);
    c64tapSavedPeriod = halfPeriodUs;
    c64tapEmitSavedPeriod = true;
  } else {
    emit_period(periodUs);
  }
}

#endif
