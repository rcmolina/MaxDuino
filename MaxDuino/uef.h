#ifndef UEF_H_INCLUDED
#define UEF_H_INCLUDED

#include "configs.h"
#include "current_settings.h"

#ifdef Use_UEF
void UEFCarrierToneBlock();
void writeUEFData();
void ReadUEFHeader();

void tzx_process_taskid_uef_getchunkid();
void tzx_process_taskid_uef_processchunkid();

// UEF chunks
#define ID0000              0x0000 // origin information chunk
#define ID0100              0x0100 // implicit start/stop bit tape data block
#define ID0104              0x0104 // defined tape format data block: data bits per packet/parity/stop bits
#define ID0110              0x0110 // carrier tone (previously high tone) 
#define ID0111              0x0111 // carrier tone (previously high tone) with dummy byte at byte
#define ID0112              0x0112 // Integer gap: cycles = (this.baud/1000)*2*n
#define ID0114              0x0114 // Security Cycles replaced with carrier tone
#define ID0116              0x0116 // floating point gap: cycles = floatGap * this.baud
#define ID0117              0x0117 // data encoding format change for 300 bauds
#define IDCHUNKEOF          0xffff

// UEF stuff
// For 1200 baud zero is 416us, one is 208us
// For 1500 baud zero is 333us, one is 166us
// For 1550 baud zero is 322us, one is 161us
// For 1600 baud zero is 313us, one is 156us

// STANDARD 1200 baud UEF
#define UEFPILOTPULSES           outWord<<2
#define UEFPILOTLENGTH           208
#define UEFZEROPULSE             416
#define UEFONEPULSE              208

#if defined(TURBOBAUD1500)
  #define UEFTURBOPILOTPULSES       outWord<<2
  #define UEFTURBOPILOTLENGTH       156
  #define UEFTURBOZEROPULSE         332
  #define UEFTURBOONEPULSE          166
#elif defined(TURBOBAUD1550)
  #define UEFTURBOPILOTPULSES       320
  #define UEFTURBOPILOTLENGTH       161
  #define UEFTURBOZEROPULSE         322
  #define UEFTURBOONEPULSE          161
#elif defined(TURBOBAUD1600)
  #define UEFTURBOPILOTPULSES       320
  #define UEFTURBOPILOTLENGTH       156
  #define UEFTURBOZEROPULSE         313
  #define UEFTURBOONEPULSE          156
#endif

#endif

#endif // UEF_H_INCLUDED
