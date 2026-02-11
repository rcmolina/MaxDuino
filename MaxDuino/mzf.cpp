#include "configs.h"
#include "mzf.h"
#include "file_utils.h"
#include "MaxProcessing.h"

// Sharp MZ tape PWM timings (MZ-700/K/A defaults).
// A "pulse" consists of an up (mark/high) time then a down (space/low) time.
// MaxDuino output toggles the output pin each "period", so we emit the up time and the down time as two consecutive periods.
static const word MZF_LONG_UP_US   = 464;
static const word MZF_LONG_DOWN_US = 494;
static const word MZF_SHORT_UP_US  = 240;
static const word MZF_SHORT_DOWN_US= 264;

// Conventional mode structure: LGAP(22000 short pulses) + LTM(40L 40S 1L) + HDR + CHK + HDRC + CHK + SGAP(11000 short pulses) + STM(20L 20S 1L) + FILE + CHK + FILEC + CHK
static const uint16_t MZF_LGAP_PULSES = 22000;
static const uint16_t MZF_SGAP_PULSES = 11000;

static const uint8_t MZF_LTM_LONGS = 40;
static const uint8_t MZF_LTM_SHORTS = 40;
static const uint8_t MZF_STM_LONGS = 20;
static const uint8_t MZF_STM_SHORTS = 20;

enum class MZF_STAGE : uint8_t {
  LGAP1,
  LTM_LONG,
  LTM_SHORT,
  LTM_ENDLONG,
  HDR1,
  CHKH1,
  HDR2,
  CHKH2,
  SGAP,
  STM_LONG,
  STM_SHORT,
  STM_ENDLONG,
  FILE1,
  CHKF1,
  FILE2,
  CHKF2,
  DONE
};

static MZF_STAGE mzf_stage = MZF_STAGE::DONE;

// header cached in RAM (128 bytes)
static byte mzf_hdr[128];
static uint16_t mzf_file_len = 0;

// Per-block checksums are "number of logical 1 bits" modulo 2^16 (big-endian on tape)
static uint16_t mzf_hdr_cksum = 0;
static uint16_t mzf_file_cksum = 0;

// counters for current stage
static uint32_t mzf_pulses_left = 0;       // for gaps
static uint16_t mzf_file_left = 0;         // for file body bytes remaining
static uint8_t mzf_tm_left = 0;            // for tapemark pulse counts

// byte writer state
enum class BYTE_SRC : uint8_t { HDR, FILE, CHK };
static BYTE_SRC mzf_src = BYTE_SRC::HDR;
static uint16_t mzf_hdr_idx = 0;
static byte mzf_cur_byte = 0;
static bool mzf_have_byte = false;
static bool mzf_leader_done = false;
static uint8_t mzf_bit_mask = 0x80;

// checksum byte emission state
static uint8_t mzf_chk_byte_idx = 0; // 0..1, big-endian

// half-wave state for emitting asymmetric pulses
static uint8_t mzf_half = 0; // 0=UP, 1=DOWN

static inline uint8_t popcount8(byte v) {
  v = v - ((v >> 1) & 0x55);
  v = (v & 0x33) + ((v >> 2) & 0x33);
  return (uint8_t)((((v + (v >> 4)) & 0x0F) * 0x01) & 0x1F);
}

static inline uint16_t mzf_cksum_add(uint16_t acc, byte v) {
  return (uint16_t)(acc + popcount8(v));
}

// Emit one half-period of a "pulse".
// Returns true when the full pulse (UP+DOWN) has been emitted.
static inline bool mzf_emit_pulse(bool isLong) {
  if (mzf_half == 0) {
    currentPeriod = isLong ? MZF_LONG_UP_US : MZF_SHORT_UP_US;
    mzf_half = 1;
    return false;
  } else {
    currentPeriod = isLong ? MZF_LONG_DOWN_US : MZF_SHORT_DOWN_US;
    mzf_half = 0;
    return true;
  }
}

static inline void mzf_next_stage(MZF_STAGE s) {
  mzf_stage = s;
  mzf_half = 0;
  mzf_have_byte = false;
  mzf_leader_done = false;
  mzf_bit_mask = 0x80;
  mzf_chk_byte_idx = 0;
}

void mzf_init() {
  // Read and cache 128-byte tape header from file
  mzf_stage = MZF_STAGE::DONE;
  mzf_half = 0;

  if (!entry.seekSet(0)) {
    // fall back to EOF handling
    currentID = BLOCKID::IDEOF;
    return;
  }
  int r = entry.read(mzf_hdr, 128);
  if (r != 128) {
    currentID = BLOCKID::IDEOF;
    return;
  }

  // File length is 2 bytes at offset 18 (little-endian) in the standard tape header.
  mzf_file_len = (uint16_t)mzf_hdr[18] | ((uint16_t)mzf_hdr[19] << 8);

  // Pre-compute header checksum.
  mzf_hdr_cksum = 0;
  for (uint16_t i = 0; i < 128; ++i) {
    mzf_hdr_cksum = mzf_cksum_add(mzf_hdr_cksum, mzf_hdr[i]);
  }

  // Start streaming file body at offset 128.
  bytesRead = 128;

  // Initialise the common TZX state machine to use our BLOCKID.
  currentTask = TASK::PROCESSID;
  currentID = BLOCKID::MZF;

  // Stage setup: LGAP first
  mzf_pulses_left = MZF_LGAP_PULSES;
  mzf_next_stage(MZF_STAGE::LGAP1);
}

static inline bool mzf_load_next_byte() {
  if (mzf_src == BYTE_SRC::HDR) {
    if (mzf_hdr_idx >= 128) return false;
    mzf_cur_byte = mzf_hdr[mzf_hdr_idx++];
    mzf_have_byte = true;
    return true;
  } else if (mzf_src == BYTE_SRC::FILE) {
    if (mzf_file_left == 0) return false;
    if (!ReadByte()) return false;
    mzf_cur_byte = outByte;
    mzf_have_byte = true;
    mzf_file_left--;
    // accumulate checksum as we stream
    mzf_file_cksum = mzf_cksum_add(mzf_file_cksum, mzf_cur_byte);
    return true;
  } else { // CHK
    if (mzf_chk_byte_idx >= 2) return false;
    uint16_t v = (mzf_stage == MZF_STAGE::CHKH1 || mzf_stage == MZF_STAGE::CHKH2) ? mzf_hdr_cksum : mzf_file_cksum;
    byte b = (mzf_chk_byte_idx == 0) ? (byte)(v >> 8) : (byte)(v & 0xFF);
    mzf_cur_byte = b;
    mzf_have_byte = true;
    mzf_chk_byte_idx++;
    return true;
  }
}

static inline void mzf_reset_byte_writer_for_src(BYTE_SRC src) {
  mzf_src = src;
  mzf_have_byte = false;
  mzf_leader_done = false;
  mzf_bit_mask = 0x80;
  mzf_chk_byte_idx = 0;
}

static void mzf_process_bytes(uint16_t totalBytesForHDR /*ignored for FILE/CHK*/) {
  (void)totalBytesForHDR;

  // Ensure a current byte is loaded, unless we're finished for this stage.
  if (!mzf_have_byte) {
    if (!mzf_load_next_byte()) {
      // done with this stage
      currentPeriod = 0;
      return;
    }
    mzf_leader_done = false;
    mzf_bit_mask = 0x80;
  }

  // Each byte is preceded by 1 long pulse
  if (!mzf_leader_done) {
    if (mzf_emit_pulse(true)) {
      mzf_leader_done = true;
    }
    return;
  }

  // Then 8 PWM pulses, MSB first
  bool bitIsOne = (mzf_cur_byte & mzf_bit_mask) != 0;
  if (mzf_emit_pulse(bitIsOne)) {
    mzf_bit_mask >>= 1;
    if (mzf_bit_mask == 0) {
      mzf_have_byte = false; // move to next byte
    }
  }
}

void mzf_process() {
  switch (mzf_stage) {
    case MZF_STAGE::LGAP1:
      // LGAP: short pulses
      if (mzf_emit_pulse(false)) {
        if (--mzf_pulses_left == 0) {
          mzf_tm_left = MZF_LTM_LONGS;
          mzf_next_stage(MZF_STAGE::LTM_LONG);
        }
      }
      return;

    case MZF_STAGE::LTM_LONG:
      if (mzf_emit_pulse(true)) {
        if (--mzf_tm_left == 0) {
          mzf_tm_left = MZF_LTM_SHORTS;
          mzf_next_stage(MZF_STAGE::LTM_SHORT);
        }
      }
      return;

    case MZF_STAGE::LTM_SHORT:
      if (mzf_emit_pulse(false)) {
        if (--mzf_tm_left == 0) {
          mzf_next_stage(MZF_STAGE::LTM_ENDLONG);
        }
      }
      return;

    case MZF_STAGE::LTM_ENDLONG:
      if (mzf_emit_pulse(true)) {
        // header 1
        mzf_hdr_idx = 0;
        mzf_reset_byte_writer_for_src(BYTE_SRC::HDR);
        mzf_next_stage(MZF_STAGE::HDR1);
      }
      return;

    case MZF_STAGE::HDR1:
      mzf_process_bytes(128);
      if (currentPeriod == 0) {
        // checksum of header
        mzf_reset_byte_writer_for_src(BYTE_SRC::CHK);
        mzf_next_stage(MZF_STAGE::CHKH1);
      }
      return;

    case MZF_STAGE::CHKH1:
      mzf_process_bytes(2);
      if (currentPeriod == 0) {
        // header copy
        mzf_hdr_idx = 0;
        mzf_reset_byte_writer_for_src(BYTE_SRC::HDR);
        mzf_next_stage(MZF_STAGE::HDR2);
      }
      return;

    case MZF_STAGE::HDR2:
      mzf_process_bytes(128);
      if (currentPeriod == 0) {
        mzf_reset_byte_writer_for_src(BYTE_SRC::CHK);
        mzf_next_stage(MZF_STAGE::CHKH2);
      }
      return;

    case MZF_STAGE::CHKH2:
      mzf_process_bytes(2);
      if (currentPeriod == 0) {
        // SGAP
        mzf_pulses_left = MZF_SGAP_PULSES;
        mzf_next_stage(MZF_STAGE::SGAP);
      }
      return;

    case MZF_STAGE::SGAP:
      if (mzf_emit_pulse(false)) {
        if (--mzf_pulses_left == 0) {
          mzf_tm_left = MZF_STM_LONGS;
          mzf_next_stage(MZF_STAGE::STM_LONG);
        }
      }
      return;

    case MZF_STAGE::STM_LONG:
      if (mzf_emit_pulse(true)) {
        if (--mzf_tm_left == 0) {
          mzf_tm_left = MZF_STM_SHORTS;
          mzf_next_stage(MZF_STAGE::STM_SHORT);
        }
      }
      return;

    case MZF_STAGE::STM_SHORT:
      if (mzf_emit_pulse(false)) {
        if (--mzf_tm_left == 0) {
          mzf_next_stage(MZF_STAGE::STM_ENDLONG);
        }
      }
      return;

    case MZF_STAGE::STM_ENDLONG:
      if (mzf_emit_pulse(true)) {
        // file body (copy 1)
        bytesRead = 128;
        mzf_file_left = mzf_file_len;
        mzf_file_cksum = 0;
        mzf_reset_byte_writer_for_src(BYTE_SRC::FILE);
        mzf_next_stage(MZF_STAGE::FILE1);
      }
      return;

    case MZF_STAGE::FILE1:
      mzf_process_bytes(0);
      if (currentPeriod == 0) {
        mzf_reset_byte_writer_for_src(BYTE_SRC::CHK);
        mzf_next_stage(MZF_STAGE::CHKF1);
      }
      return;

    case MZF_STAGE::CHKF1:
      mzf_process_bytes(2);
      if (currentPeriod == 0) {
        // Many Sharp MZ loaders will successfully load from the first payload copy.
        // MaxDuino's progress indicator is based on file length, so emitting the
        // conventional second copy makes playback appear to "start again".
        // For better UX, stop after the first FILE+CHK block and return to menu.
        mzf_next_stage(MZF_STAGE::DONE);
      }
      return;

    case MZF_STAGE::FILE2:
      mzf_process_bytes(0);
      if (currentPeriod == 0) {
        mzf_reset_byte_writer_for_src(BYTE_SRC::CHK);
        mzf_next_stage(MZF_STAGE::CHKF2);
      }
      return;

    case MZF_STAGE::CHKF2:
      mzf_process_bytes(2);
      if (currentPeriod == 0) {
        mzf_next_stage(MZF_STAGE::DONE);
      }
      return;

    case MZF_STAGE::DONE:
    default:
      // End of file: hand off to existing EOF handler
      currentID = BLOCKID::IDEOF;
      currentTask = TASK::PROCESSID;
      count_r = 255;
      currentPeriod = 0;
      return;
  }
}
