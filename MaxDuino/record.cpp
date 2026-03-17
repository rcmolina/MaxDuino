#include "configs.h"

#include "record.h"

#ifdef Use_Rec

#include <Arduino.h>
#include <SdFat.h>

#include "Display.h"

// MaxDuino exports the current working directory pointer.
#include "file_utils.h" // currentDir

// SdFat instance is owned by MaxDuino.ino
extern SdFat sd;

// -----------------
// TZX constants
// -----------------
static constexpr uint16_t kSampleRate = 44100;
// ZX Spectrum nominal CPU clock is 3.5 MHz, so 3_500_000 / 44100 ≈ 79.36 T-states per sample.
// The TZX spec recommends 79 for 44100 Hz.
static constexpr uint16_t kTStatesPerSample = 79;
static constexpr uint16_t kPauseAfterMs = 1000;

static const uint8_t kTzxHeader[10] = {
  'Z','X','T','a','p','e','!',
  0x1A,
  0x01, 0x20 // v1.20
};

// -----------------
// Recording buffers
// -----------------
// Double-buffered 512-byte pages for SD writes.
static uint8_t pageA[512];
static uint8_t pageB[512];
static volatile uint16_t pagePos = 0;
static volatile uint8_t activePage = 0; // 0=A, 1=B
static volatile bool pageReadyA = false;
static volatile bool pageReadyB = false;
static volatile uint32_t droppedBytes = 0;

// -----------------
// Recording UI
// -----------------
// MaxDuino's OLED XY2 path prints strings using sendStrXY() which expects a
// NUL-terminated buffer. Several existing helper functions build a 16-char
// buffer without terminating it, which can overrun into adjacent memory and
// visually corrupt the previous line.
//
// During recording we only update the screen ONCE (to reduce ADC noise), so we
// construct two fixed-width, NUL-terminated lines and render them atomically.
static void drawRecordingScreenOnce(const char* filename)
{
  char l1[17];
  char l2[17];
  memset(l1, ' ', 16);
  memset(l2, ' ', 16);
  l1[16] = '\0';
  l2[16] = '\0';

  const char* msg = "Recording";
  for (uint8_t i = 0; msg[i] && i < 16; ++i) l1[i] = msg[i];
  for (uint8_t i = 0; filename && filename[i] && i < 16; ++i) l2[i] = filename[i];

  #if defined(OLED1306) && defined(XY2)
    // XY2 uses sendStrXY() which requires NUL termination.
    sendStrXY(l1, 0, 0);
    // MaxDuino's second logical line is "lineaxy" (often 2 on XY2 builds).
    sendStrXY(l2, 0, lineaxy);
  #else
    // Other display backends are safe with fixed-width printing.
    // IMPORTANT: MaxDuino uses "lineaxy" for the second display line.
    // On OLED builds with XY2 font, each logical text line consumes two
    // rows (Y and Y+1). Using "1" here overlaps/clips line 0.
    printtext(l1, 0);
    printtext(l2, lineaxy);
  #endif
}

// -----------------
// Filename helpers
// -----------------
static bool has_ext_ci(const char *name, const char *ext3) {
  // ext3 must be 3 chars without dot, e.g. "tzx".
  if (!name || !ext3) return false;
  const char *dot = strrchr(name, '.');
  if (!dot) return false;
  // Require exactly 3 chars after the dot.
  if (!dot[1] || !dot[2] || !dot[3] || dot[4]) return false;

  auto up = [](char c) -> char {
    if (c >= 'a' && c <= 'z') return (char)(c - 'a' + 'A');
    return c;
  };

  return (up(dot[1]) == up(ext3[0]) && up(dot[2]) == up(ext3[1]) && up(dot[3]) == up(ext3[2]));
}

static uint16_t count_files_with_ext_in_current_dir(const char *ext3) {
  if (!currentDir) return 0;

  // Preserve directory stream position so the browser UI isn't disturbed.
  const uint32_t savedPos = currentDir->curPosition();
  currentDir->rewind();

  uint16_t count = 0;
  SdBaseFile tmp;
  char name[64];

  while (tmp.openNext(currentDir, O_RDONLY)) {
    if (tmp.isFile()) {
      name[0] = 0;
      tmp.getName(name, sizeof(name));
      if (has_ext_ci(name, ext3)) {
        count++;
      }
    }
    tmp.close();
  }

  currentDir->seekSet(savedPos);
  return count;
}

// Bit packing (MSb first as per TZX spec)
static volatile uint8_t bitByte = 0;
static volatile uint8_t bitCount = 0; // 0..7 bits already filled in bitByte

static volatile bool gRecording = false;
static volatile bool gRecordPaused = false;
static char gRecName[32] = {0};

// File + positions for patching header fields on stop
static SdBaseFile recFile;
static uint32_t filePos_usedBits = 0;
static uint32_t filePos_len3 = 0;
static uint32_t dataBytesWritten = 0; // bytes already written to file (payload only)

// -----------------
// MCU sampling (megaAVR 0/1-series)
// -----------------
#if defined(__AVR_ATmega4808__) || defined(__AVR_ATmega4809__)

static void adc_start_freerun_record_pin() {
  // Configure ADC0 for free running conversions on the recording input.
  // ATmega4808 Nano mapping: A7 = AIN15 = PF5.
  // ATmega4809 Nano Every mapping: A7 = AIN5 = PD5.
  // We let the ADC run faster than 44.1k and downsample via a timer ISR.

  // Select VDD as reference (default), right-adjusted.
  ADC0.CTRLA = 0; // disable while configuring
  ADC0.CTRLB = 0;
  ADC0.CTRLC = ADC_PRESC_DIV16_gc; // 16MHz/16 = 1MHz ADC clock
  ADC0.CTRLD = 0;
  ADC0.SAMPCTRL = 0;

  #if defined(__AVR_ATmega4808__)
    // Nano 4808: A7 = PF5 = AIN15
    ADC0.MUXPOS = ADC_MUXPOS_AIN15_gc;
  #elif defined(__AVR_ATmega4809__)
    // Nano Every / ATmega4809: A7 = PD5 = AIN5
    ADC0.MUXPOS = ADC_MUXPOS_AIN5_gc;
  #endif

  // Free-run, enable
  ADC0.CTRLA = ADC_ENABLE_bm | ADC_FREERUN_bm;
  ADC0.COMMAND = ADC_STCONV_bm;
}

static void adc_stop() {
  ADC0.CTRLA &= ~(ADC_ENABLE_bm);
}

static void timer_start_44100() {
  // IMPORTANT: Do NOT use TCA0 here.
  // MaxDuino already uses TCA0 via TimerCounter.cpp on megaAVR, which would
  // create a duplicate ISR ("multiple definition of __vector_x").
  // Use TCB1 when available (keeps TCB0 free for millis() on some cores).

#if defined(TCB1)
#  define REC_TCB TCB1
#  define REC_TCB_INT_vect TCB1_INT_vect
#else
#  define REC_TCB TCB0
#  define REC_TCB_INT_vect TCB0_INT_vect
#endif

  // Periodic interrupt using TCB in "INT" mode.
  // CCMP is the period in timer clocks.
  // With DIV1: CCMP ~= F_CPU / sampleRate
  REC_TCB.CTRLA = 0;
  REC_TCB.CTRLB = TCB_CNTMODE_INT_gc;
  REC_TCB.CCMP  = (uint16_t)(F_CPU / kSampleRate);
  REC_TCB.CNT   = 0;
  REC_TCB.INTFLAGS = TCB_CAPT_bm;
  REC_TCB.INTCTRL  = TCB_CAPT_bm;
  REC_TCB.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
}

static void timer_stop() {
  REC_TCB.INTCTRL = 0;
  REC_TCB.CTRLA = 0;
  REC_TCB.INTFLAGS = TCB_CAPT_bm;
}

static inline uint8_t* active_page_ptr() {
  return (activePage == 0) ? pageA : pageB;
}

static inline bool other_page_ready() {
  return (activePage == 0) ? pageReadyB : pageReadyA;
}

static inline void mark_active_ready_and_swap() {
  if (activePage == 0) pageReadyA = true;
  else pageReadyB = true;
  activePage ^= 1;
  pagePos = 0;
}

ISR(REC_TCB_INT_vect) {
  // Clear interrupt flag
  REC_TCB.INTFLAGS = TCB_CAPT_bm;

  if (!gRecording) return;

  // Read latest ADC sample (10-bit) and threshold to 1-bit.
  // For phase 1 we use a mid-scale threshold; later phases can auto-calibrate.
  const uint16_t sample = ADC0.RES;
  const uint8_t bit = (sample >= 512) ? 1 : 0;

  uint8_t bb = bitByte;
  uint8_t bc = bitCount;
  if (bit) bb |= (uint8_t)(0x80 >> bc);
  bc++;

  if (bc >= 8) {
    // push completed byte into current page
    uint16_t pos = pagePos;
    uint8_t* p = active_page_ptr();
    p[pos] = bb;
    pos++;

    // reset bit packer
    bb = 0;
    bc = 0;

    if (pos >= 512) {
      // Page full. If the other page is still waiting to be written, drop data.
      if (other_page_ready()) {
        droppedBytes++;
        pos = 511; // keep overwriting last byte to avoid bounds issues
      } else {
        pagePos = pos;
        bitByte = bb;
        bitCount = bc;
        mark_active_ready_and_swap();
        return;
      }
    }
    pagePos = pos;
  }

  bitByte = bb;
  bitCount = bc;
}

#else

// Non-megaAVR targets: recording is not implemented in this phase.
static void adc_start_freerun_ain15_pf5() {}
static void adc_stop() {}
static void timer_start_44100() {}
static void timer_stop() {}

#endif

// -----------------
// Public API
// -----------------

bool is_recording() {
  return gRecording;
}

bool is_recording_paused() {
  return gRecordPaused;
}

void pause_recording() {
  if (!gRecording || gRecordPaused) return;
  timer_stop();
  gRecordPaused = true;
  printtext2F(PSTR("Paused       "),0);
}

void resume_recording() {
  if (!gRecording || !gRecordPaused) return;
  gRecordPaused = false;
  timer_start_44100();
  printtext2F(PSTR("Recording    "),0);
}

static void tzx_write_u16_le(SdBaseFile &f, uint16_t v) {
  uint8_t b[2] = { (uint8_t)(v & 0xFF), (uint8_t)(v >> 8) };
  f.write(b, 2);
}

static void tzx_write_u24_le(SdBaseFile &f, uint32_t v) {
  uint8_t b[3] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF), (uint8_t)((v >> 16) & 0xFF) };
  f.write(b, 3);
}

bool start_recording() {
  if (gRecording) return true;

  // Create a unique filename in the *current directory* where SdFat is positioned.
  // filecount = number of existing .tzx files in the folder.
  const uint16_t filecount = count_files_with_ext_in_current_dir("tzx");

  char recName[32];
  // Matches requested pattern: MaxSave<i>.tzx
  snprintf(recName, sizeof(recName), "MaxSave%u.tzx", (unsigned)filecount);
  strncpy(gRecName, recName, sizeof(gRecName) - 1);
  gRecName[sizeof(gRecName) - 1] = 0;

  // Open in current directory. Overwrite if exists.
  recFile.close();
  if (!recFile.open(currentDir, recName, O_RDWR | O_CREAT | O_TRUNC)) {
    printtextF(PSTR("SD open fail"), 0);
    return false;
  }

  // Reset buffers
  noInterrupts();
  pagePos = 0;
  activePage = 0;
  pageReadyA = false;
  pageReadyB = false;
  droppedBytes = 0;
  bitByte = 0;
  bitCount = 0;
  interrupts();

  dataBytesWritten = 0;
  gRecordPaused = false;

  // UI: draw once, then keep screen quiet until stop.
  // Use a NUL-terminated, fixed-width render so line 2 never corrupts line 1.
  drawRecordingScreenOnce(recName);

  // Write TZX header
  recFile.write(kTzxHeader, sizeof(kTzxHeader));

  // Write Direct Recording block (0x15) with placeholder length fields.
  recFile.write((uint8_t)0x15);
  tzx_write_u16_le(recFile, kTStatesPerSample);
  tzx_write_u16_le(recFile, kPauseAfterMs);

  // used bits in last byte (patched on stop)
  filePos_usedBits = recFile.curPosition();
  recFile.write((uint8_t)8);

  // data length (3 bytes, patched on stop)
  filePos_len3 = recFile.curPosition();
  tzx_write_u24_le(recFile, 0);

  recFile.flush();

  // Start ADC + timer sampling
  adc_start_freerun_record_pin();
  gRecording = true;
  timer_start_44100();
  return true;
}

static void write_ready_page(uint8_t which) {
  if (!recFile.isOpen()) return;
  if (which == 0) {
    recFile.write(pageA, 512);
  } else {
    recFile.write(pageB, 512);
  }
  dataBytesWritten += 512;
}

void recording_loop() {
  if (!gRecording) return;

  // Write any full pages to SD.
  if (pageReadyA) {
    noInterrupts();
    pageReadyA = false;
    interrupts();
    write_ready_page(0);
  }

  if (pageReadyB) {
    noInterrupts();
    pageReadyB = false;
    interrupts();
    write_ready_page(1);
  }
}

void stop_recording() {
  if (!gRecording) return;

  // Stop sampling first.
  timer_stop();
  gRecording = false;
  gRecordPaused = false;
  adc_stop();

  // Flush any ready pages.
  recording_loop();

  // Capture remaining partial page and bit pack state.
  uint16_t pos;
  uint8_t bb;
  uint8_t bc;
  uint8_t which;
  noInterrupts();
  pos = pagePos;
  bb = bitByte;
  bc = bitCount;
  which = activePage;
  // prevent ISR from touching buffers
  pagePos = 0;
  interrupts();

  // If we have a partial byte, write it into the page.
  uint8_t usedBitsLast = 8;
  if (bc != 0) {
    usedBitsLast = bc;
    uint8_t* p = (which == 0) ? pageA : pageB;
    if (pos < 512) {
      p[pos] = bb; // remaining bits are already MSb aligned
      pos++;
    }
  }

  // Write remaining payload bytes (pos may be 0..512).
  if (pos) {
    uint8_t* p = (which == 0) ? pageA : pageB;
    recFile.write(p, pos);
    dataBytesWritten += pos;
  }

  // Patch used bits + length.
  recFile.seekSet(filePos_usedBits);
  recFile.write(usedBitsLast);

  recFile.seekSet(filePos_len3);
  tzx_write_u24_le(recFile, dataBytesWritten);

  recFile.flush();
  recFile.close();

  // Show completion (one-time).
  printtextF(PSTR("Saved"), 0);
  printtext(gRecName, lineaxy);
}

#endif // Use_Rec
