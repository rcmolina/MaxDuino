#ifndef RECORD_H_INCLUDED
#define RECORD_H_INCLUDED

#include <Arduino.h>

// Phase 1: create "test.tzx" and record a Direct Recording (TZX block 0x15)
// by sampling the audio input on A7 at 44100 Hz.

// NOTE:
// The rest of MaxDuino can be built with Use_Rec disabled.
// Provide stubs in that case to avoid undefined references at link time.

#ifdef Use_Rec

// Returns true when recording is active.
bool is_recording();

// Start recording into /test.tzx (root of SD).
// Returns true if recording was started successfully.
bool start_recording();

// Service routine to be called frequently from loop() while recording.
// Handles SD writes for filled buffers.
void recording_loop();

// Recording pause state helpers used by motor-control integration.
bool is_recording_paused();
void pause_recording();
void resume_recording();

// Stop recording, finalize the TZX header/block and close the file.
void stop_recording();

#else

static inline bool is_recording() { return false; }
static inline bool start_recording() { return false; }
static inline void recording_loop() {}
static inline bool is_recording_paused() { return false; }
static inline void pause_recording() {}
static inline void resume_recording() {}
static inline void stop_recording() {}

#endif

#endif // RECORD_H_INCLUDED
