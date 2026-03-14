#include "configs.h"
#ifdef Use_MTX

#include "mtx.h"
#include "file_utils.h"
#include "MaxDuino.h"
#include "MaxProcessing.h"
#include "processing_state.h"

// Memotech MTX tape waveform, based on the supplied mtx.c converter.
// At 44.1 kHz normal mode:
//   bit 0 =  9 low samples +  9 high samples  ~= 204 us + 204 us
//   bit 1 = 18 low samples + 18 high samples  ~= 408 us + 408 us
// Bits are written LSB-first.
// Leader structure before each block/chunk is:
//   9-sample extra half-cycle, 1500 x bit-0 leader, 9-sample half-cycle,
//   27-sample gap into data mode.
static const word MTX_SHORT_US = 204;
static const word MTX_LONG_US  = 408;
static const word MTX_GAP_US   = 612; // 27 samples at 44.1 kHz
static const uint16_t MTX_LEADER_BITS = 1500;
static const uint16_t MTX_CHUNK_BYTES = 16384;
static const uint16_t MTX_HEADER_PAUSE_MS = 372; // 0x4000 / 44100 ~= 372 ms
static const uint16_t MTX_EXTRA_PAUSE_MS  = 186; // 0x2000 / 44100 ~= 186 ms
static const uint16_t MTX_SYSVARS_TOP = 0xFB4B;

enum class MTX_STAGE : uint8_t {
  HEADER_LEADER,
  HEADER_BYTES,
  HEADER_PAUSE,
  SYS_LEADER,
  SYS_BYTES,
  PRG_LEADER,
  PRG_BYTES,
  VAR_LEADER,
  VAR_BYTES,
  EXTRA_PAUSE,
  EXTRA_LEADER,
  EXTRA_BYTES,
  DONE
};

enum class MTX_LEADER_PHASE : uint8_t {
  PRE_HALF,
  BITS,
  POST_HALF,
  GAP_HALF,
  COMPLETE
};

static MTX_STAGE mtx_stage = MTX_STAGE::DONE;
static MTX_LEADER_PHASE mtx_leader_phase = MTX_LEADER_PHASE::COMPLETE;

static byte mtx_header[18];
static uint8_t mtx_header_idx = 0;
static uint8_t mtx_header_tail_zeros = 0;

static uint32_t mtx_payload_start = 18;
static uint32_t mtx_sysvars_len = 0;
static uint32_t mtx_prg_len = 0;
static uint32_t mtx_var_len = 0;
static uint32_t mtx_extra_len = 0;

static uint32_t mtx_section_start = 0;
static uint32_t mtx_section_remaining = 0;
static uint16_t mtx_chunk_remaining = 0;
static uint16_t mtx_leader_remaining = 0;
static uint16_t mtx_pause_remaining = 0;

static uint8_t mtx_cur_byte = 0;
static bool mtx_have_byte = false;
static uint8_t mtx_bit_mask = 0x01; // LSB first
static uint8_t mtx_half = 0;        // 0 = first half of bit, 1 = second half

static bool mtx_emit_bit(bool isOne) {
  currentPeriod = isOne ? MTX_LONG_US : MTX_SHORT_US;
  mtx_half ^= 1;
  return (mtx_half == 0);
}

static bool mtx_emit_pause() {
  if (mtx_pause_remaining == 0) {
    currentPeriod = 0;
    return true;
  }
  word chunk = (mtx_pause_remaining > MAXPAUSE_PERIOD) ? MAXPAUSE_PERIOD : mtx_pause_remaining;
  currentPeriod = chunk;
  bitSet(currentPeriod, 15);
  mtx_pause_remaining -= chunk;
  return false;
}

static void mtx_start_leader(MTX_STAGE leaderStage) {
  mtx_stage = leaderStage;
  mtx_leader_phase = MTX_LEADER_PHASE::PRE_HALF;
  mtx_leader_remaining = MTX_LEADER_BITS;
  mtx_half = 0;
  mtx_have_byte = false;
  mtx_bit_mask = 0x01;
}

static bool mtx_process_leader() {
  switch (mtx_leader_phase) {
    case MTX_LEADER_PHASE::PRE_HALF:
      currentPeriod = MTX_SHORT_US;
      mtx_leader_phase = MTX_LEADER_PHASE::BITS;
      return false;

    case MTX_LEADER_PHASE::BITS:
      if (mtx_emit_bit(false)) {
        if (--mtx_leader_remaining == 0) {
          mtx_half = 0;
          mtx_leader_phase = MTX_LEADER_PHASE::POST_HALF;
        }
      }
      return false;

    case MTX_LEADER_PHASE::POST_HALF:
      currentPeriod = MTX_SHORT_US;
      mtx_leader_phase = MTX_LEADER_PHASE::GAP_HALF;
      return false;

    case MTX_LEADER_PHASE::GAP_HALF:
      currentPeriod = MTX_GAP_US;
      mtx_leader_phase = MTX_LEADER_PHASE::COMPLETE;
      mtx_half = 0;
      return true;

    case MTX_LEADER_PHASE::COMPLETE:
    default:
      currentPeriod = 0;
      return true;
  }
}

static void mtx_begin_section(uint32_t start, uint32_t len, MTX_STAGE leaderStage) {
  mtx_section_start = start;
  mtx_section_remaining = len;
  bytesRead = start;
  mtx_chunk_remaining = 0;
  mtx_start_leader(leaderStage);
}

static void mtx_advance_after_section();

static bool mtx_load_next_header_byte() {
  if (mtx_header_idx < sizeof(mtx_header)) {
    mtx_cur_byte = mtx_header[mtx_header_idx++];
    return true;
  }
  if (mtx_header_tail_zeros < 2) {
    ++mtx_header_tail_zeros;
    mtx_cur_byte = 0x00;
    return true;
  }
  return false;
}

static bool mtx_load_next_section_byte() {
  if (mtx_section_remaining == 0) {
    return false;
  }

  if (mtx_chunk_remaining == 0) {
    mtx_chunk_remaining = (mtx_section_remaining > MTX_CHUNK_BYTES) ? MTX_CHUNK_BYTES : (uint16_t)mtx_section_remaining;
  }

  if (!ReadByte()) {
    return false;
  }

  mtx_cur_byte = outByte;
  return true;
}

static void mtx_finish_section_byte() {
  if (mtx_chunk_remaining > 0) --mtx_chunk_remaining;
  if (mtx_section_remaining > 0) --mtx_section_remaining;
  mtx_have_byte = false;
  mtx_bit_mask = 0x01;

  if (mtx_chunk_remaining == 0 && mtx_section_remaining > 0) {
    switch (mtx_stage) {
      case MTX_STAGE::SYS_BYTES:   mtx_start_leader(MTX_STAGE::SYS_LEADER); break;
      case MTX_STAGE::PRG_BYTES:   mtx_start_leader(MTX_STAGE::PRG_LEADER); break;
      case MTX_STAGE::VAR_BYTES:   mtx_start_leader(MTX_STAGE::VAR_LEADER); break;
      case MTX_STAGE::EXTRA_BYTES: mtx_start_leader(MTX_STAGE::EXTRA_LEADER); break;
      default: break;
    }
  } else if (mtx_section_remaining == 0) {
    mtx_advance_after_section();
  }
}

static void mtx_process_header_bytes() {
  if (!mtx_have_byte) {
    if (!mtx_load_next_header_byte()) {
      mtx_pause_remaining = MTX_HEADER_PAUSE_MS;
      mtx_stage = MTX_STAGE::HEADER_PAUSE;
      currentPeriod = 0;
      return;
    }
    mtx_have_byte = true;
    mtx_bit_mask = 0x01;
    mtx_half = 0;
  }

  const bool bitIsOne = (mtx_cur_byte & mtx_bit_mask) != 0;
  if (mtx_emit_bit(bitIsOne)) {
    mtx_bit_mask <<= 1;
    if (mtx_bit_mask == 0) {
      mtx_have_byte = false;
      mtx_bit_mask = 0x01;
    }
  }
}

static void mtx_process_section_bytes() {
  if (!mtx_have_byte) {
    if (!mtx_load_next_section_byte()) {
      mtx_advance_after_section();
      currentPeriod = 0;
      return;
    }
    mtx_have_byte = true;
    mtx_bit_mask = 0x01;
    mtx_half = 0;
  }

  const bool bitIsOne = (mtx_cur_byte & mtx_bit_mask) != 0;
  if (mtx_emit_bit(bitIsOne)) {
    mtx_bit_mask <<= 1;
    if (mtx_bit_mask == 0) {
      mtx_finish_section_byte();
    }
  }
}

static void mtx_advance_after_section() {
  if (mtx_stage == MTX_STAGE::SYS_BYTES || mtx_stage == MTX_STAGE::SYS_LEADER) {
    if (mtx_prg_len > 0) {
      mtx_begin_section(mtx_payload_start + mtx_sysvars_len, mtx_prg_len, MTX_STAGE::PRG_LEADER);
    } else if (mtx_var_len > 0) {
      mtx_begin_section(mtx_payload_start + mtx_sysvars_len + mtx_prg_len, mtx_var_len, MTX_STAGE::VAR_LEADER);
    } else if (mtx_extra_len > 0) {
      mtx_pause_remaining = MTX_EXTRA_PAUSE_MS;
      mtx_stage = MTX_STAGE::EXTRA_PAUSE;
    } else {
      mtx_stage = MTX_STAGE::DONE;
    }
    return;
  }

  if (mtx_stage == MTX_STAGE::PRG_BYTES || mtx_stage == MTX_STAGE::PRG_LEADER) {
    if (mtx_var_len > 0) {
      mtx_begin_section(mtx_payload_start + mtx_sysvars_len + mtx_prg_len, mtx_var_len, MTX_STAGE::VAR_LEADER);
    } else if (mtx_extra_len > 0) {
      mtx_pause_remaining = MTX_EXTRA_PAUSE_MS;
      mtx_stage = MTX_STAGE::EXTRA_PAUSE;
    } else {
      mtx_stage = MTX_STAGE::DONE;
    }
    return;
  }

  if (mtx_stage == MTX_STAGE::VAR_BYTES || mtx_stage == MTX_STAGE::VAR_LEADER) {
    if (mtx_extra_len > 0) {
      mtx_pause_remaining = MTX_EXTRA_PAUSE_MS;
      mtx_stage = MTX_STAGE::EXTRA_PAUSE;
    } else {
      mtx_stage = MTX_STAGE::DONE;
    }
    return;
  }

  if (mtx_stage == MTX_STAGE::EXTRA_BYTES || mtx_stage == MTX_STAGE::EXTRA_LEADER) {
    mtx_stage = MTX_STAGE::DONE;
    return;
  }
}

void mtx_init() {
  mtx_stage = MTX_STAGE::DONE;
  mtx_leader_phase = MTX_LEADER_PHASE::COMPLETE;
  mtx_half = 0;
  mtx_have_byte = false;
  mtx_header_idx = 0;
  mtx_header_tail_zeros = 0;

  if (!entry.seekSet(0) || entry.read(mtx_header, sizeof(mtx_header)) != (int)sizeof(mtx_header)) {
    currentID = BLOCKID::IDEOF;
    return;
  }

  if (mtx_header[0] != 0xFF) {
    currentID = BLOCKID::IDEOF;
    return;
  }

  // Real-world MTX files exist with both 0xF2 0xF8 and 0xF8 0xF2 in bytes 16/17.
  // Accept either ordering, but always interpret the stack limit as little-endian.
  const bool validMtxMarker =
      ((mtx_header[16] == 0xF2) && (mtx_header[17] == 0xF8)) ||
      ((mtx_header[16] == 0xF8) && (mtx_header[17] == 0xF2));
  if (!validMtxMarker) {
    currentID = BLOCKID::IDEOF;
    return;
  }

  const uint16_t stklim = (uint16_t)mtx_header[16] | ((uint16_t)mtx_header[17] << 8);
  const uint32_t payload_len = (filesize > mtx_payload_start) ? (filesize - mtx_payload_start) : 0;

  if (stklim >= MTX_SYSVARS_TOP) {
    mtx_sysvars_len = 0;
  } else {
    mtx_sysvars_len = (uint32_t)MTX_SYSVARS_TOP - stklim;
    if (mtx_sysvars_len > payload_len) mtx_sysvars_len = payload_len;
  }

  auto read_le16_at = [&](uint16_t addr, uint16_t &value) -> bool {
    if (addr < stklim) return false;
    const uint32_t rel = (uint32_t)addr - stklim;
    if (rel + 1 >= mtx_sysvars_len) return false;
    if (readfile(2, mtx_payload_start + rel) != 2) return false;
    value = (uint16_t)filebuffer[0] | ((uint16_t)filebuffer[1] << 8);
    return true;
  };

  uint16_t calcst = 0;
  uint16_t varnam = 0;
  uint16_t prglen = 0;
  if (mtx_sysvars_len > 0) {
    (void)read_le16_at(0xFA81, calcst);
    (void)read_le16_at(0xFA7B, varnam);
    (void)read_le16_at(0xFACC, prglen);
  }

  const uint32_t remaining_after_sys = (payload_len > mtx_sysvars_len) ? (payload_len - mtx_sysvars_len) : 0;
  mtx_prg_len = prglen;
  if (mtx_prg_len > remaining_after_sys) mtx_prg_len = remaining_after_sys;

  if (calcst >= varnam) {
    mtx_var_len = (uint32_t)calcst - varnam;
  } else {
    mtx_var_len = 0;
  }
  const uint32_t remaining_after_prg = (remaining_after_sys > mtx_prg_len) ? (remaining_after_sys - mtx_prg_len) : 0;
  if (mtx_var_len > remaining_after_prg) mtx_var_len = remaining_after_prg;

  const uint32_t consumed = mtx_sysvars_len + mtx_prg_len + mtx_var_len;
  mtx_extra_len = (payload_len > consumed) ? (payload_len - consumed) : 0;

  currentTask = TASK::PROCESSID;
  currentID = BLOCKID::MTX;
  mtx_start_leader(MTX_STAGE::HEADER_LEADER);
}

void mtx_process() {
  switch (mtx_stage) {
    case MTX_STAGE::HEADER_LEADER:
      if (mtx_process_leader()) {
        mtx_stage = MTX_STAGE::HEADER_BYTES;
      }
      return;

    case MTX_STAGE::SYS_LEADER:
      if (mtx_process_leader()) {
        mtx_stage = MTX_STAGE::SYS_BYTES;
      }
      return;

    case MTX_STAGE::PRG_LEADER:
      if (mtx_process_leader()) {
        mtx_stage = MTX_STAGE::PRG_BYTES;
      }
      return;

    case MTX_STAGE::VAR_LEADER:
      if (mtx_process_leader()) {
        mtx_stage = MTX_STAGE::VAR_BYTES;
      }
      return;

    case MTX_STAGE::EXTRA_LEADER:
      if (mtx_process_leader()) {
        mtx_stage = MTX_STAGE::EXTRA_BYTES;
      }
      return;

    case MTX_STAGE::HEADER_BYTES:
      mtx_process_header_bytes();
      return;

    case MTX_STAGE::HEADER_PAUSE:
      if (mtx_emit_pause()) {
        if (mtx_sysvars_len > 0) {
          mtx_begin_section(mtx_payload_start, mtx_sysvars_len, MTX_STAGE::SYS_LEADER);
        } else if (mtx_prg_len > 0) {
          mtx_begin_section(mtx_payload_start + mtx_sysvars_len, mtx_prg_len, MTX_STAGE::PRG_LEADER);
        } else if (mtx_var_len > 0) {
          mtx_begin_section(mtx_payload_start + mtx_sysvars_len + mtx_prg_len, mtx_var_len, MTX_STAGE::VAR_LEADER);
        } else if (mtx_extra_len > 0) {
          mtx_pause_remaining = MTX_EXTRA_PAUSE_MS;
          mtx_stage = MTX_STAGE::EXTRA_PAUSE;
        } else {
          mtx_stage = MTX_STAGE::DONE;
        }
      }
      return;

    case MTX_STAGE::SYS_BYTES:
    case MTX_STAGE::PRG_BYTES:
    case MTX_STAGE::VAR_BYTES:
    case MTX_STAGE::EXTRA_BYTES:
      mtx_process_section_bytes();
      return;

    case MTX_STAGE::EXTRA_PAUSE:
      if (mtx_emit_pause()) {
        if (mtx_extra_len > 0) {
          mtx_begin_section(mtx_payload_start + mtx_sysvars_len + mtx_prg_len + mtx_var_len, mtx_extra_len, MTX_STAGE::EXTRA_LEADER);
        } else {
          mtx_stage = MTX_STAGE::DONE;
        }
      }
      return;

    case MTX_STAGE::DONE:
    default:
      currentID = BLOCKID::IDEOF;
      currentTask = TASK::PROCESSID;
      currentPeriod = 0;
      return;
  }
}

#endif // Use_MTX
