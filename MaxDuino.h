#define SHORT_SILENCE       122
#define LONG_SILENCE        SHORT_SILENCE*2

#define SHORT_HEADER        200
#define LONG_HEADER         800

//#define buffsize            219
//#define dragonBuff          4
/* Buffer overflow detected by David Hooper, tzx buffer must be with even positions */
#define buffsize            175  // Impar para CoCo
#define dragonBuff          0     // Ajuste para que wbuffer sea divisible entre 8: (175+1-0)/8

/* Header Definitions */
PROGMEM const byte HEADER[8] = { 0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74 };
//PROGMEM const byte DRAGON[8] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
PROGMEM const byte ASCII[10] = { 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA };
PROGMEM const byte BINF[10]  =  { 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0 };
PROGMEM const byte BASIC[10] = { 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3, 0xD3 };
/*
#define lookNothing   0     //look for nothing
#define lookHeader    1     //looking for header/data
#define lookType      2     //looking for file type
#define wHeader       3     //Write Header
#define wSilence      4     //Write Silence
#define wData         5     //Write Data
#define wClose        6     //Write closing silence
*/
#define lookHeader    0     //looking for header/data
#define lookType      1     //looking for file type
#define wHeader       2     //Write Header
#define wSilence      3     //Write Silence
#define wData         4     //Write Data
#define wClose        5     //Write closing silence
#define wSync         6     //
#define wNameFileBlk  7     //
#define lookLeader    8
#define wNewLeader    9

#define typeNothing   0
#define typeAscii     1
#define typeBinf      2
#define typeBasic     3
#define typeUnknown   4
#define typeEOF       5

byte currentTask=lookHeader;
byte currentType=typeNothing;

byte bits[11];

//ISR Variables
volatile byte pass = 0;
volatile byte pos = 0;
volatile byte wbuffer[buffsize+1][2];
volatile byte morebuff = HIGH;
volatile byte working=0;
volatile byte isStopped=false;

//Main Variables
volatile long count = 0;
byte btemppos = 0;
byte copybuff = LOW;
byte input[11];
unsigned long bytesRead=0;
byte fileStage=0;
byte dragonMode=0;
byte out=LOW;
byte lastByte;
byte currpct = 100;
byte newpct = 0;
//uint8_t SpecialChar [8]= { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 };
byte spinpos = 0;
unsigned long timeDiff2 = 0;
unsigned int lcdsegs = 0;
unsigned int offset =2;

volatile byte currentBit=0;
//Keep track of which ID, Task, and Block Task we're dealing with
byte currentID = 0;
byte currentBlockTask = 0;
//Temporarily store for a pulse period before loading it into the buffer.
word currentPeriod=1;
//TZX block list - uncomment as supported
#define ID10                0x10    //Standard speed data block
#define ID11                0x11    //Turbo speed data block
#define ID12                0x12    //Pure tone
#define ID13                0x13    //Sequence of pulses of various lengths
#define ID14                0x14    //Pure data block
#define ID15                0x15    //Direct recording block -- TBD - curious to load OTLA files using direct recording (22KHz)
//#define ID18                0x18    //CSW recording block
#define ID19                0x19    //Generalized data block hacked for zx81
#define ID20                0x20    //Pause (silence) or 'Stop the tape' command
#define ID21                0x21    //Group start
#define ID22                0x22    //Group end
#define ID23                0x23    //Jump to block
#define ID24                0x24    //Loop start
#define ID25                0x25    //Loop end
#define ID26                0x26    //Call sequence
#define ID27                0x27    //Return from sequence
#define ID28                0x28    //Select block
#define ID2A                0x2A    //Stop the tape if in 48K mode
#define ID2B                0x2B    //Set signal level
#define ID30                0x30    //Text description
#define ID31                0x31    //Message block
#define ID32                0x32    //Archive info
#define ID33                0x33    //Hardware type
#define ID35                0x35    //Custom info block
#define ID4B                0x4B    //Kansas City block (MSX/BBC/Acorn/...)
#define IDPAUSE             0x59    //Custom Pause processing
#define ID5A                0x5A    //Glue block (90 dec, ASCII Letter 'Z')
#define ORIC                0xFA    //Oric Tap File
#define AYO                 0xFB    //AY file
#define ZXO                 0xFC    //ZX80 O file
#define ZXP                 0xFD    //ZX81 P File
#define TAP                 0xFE    //Tap File Mode
#define IDEOF               0xFF    //End of file

//TZX File Tasks
#define GETFILEHEADER         0
#define GETID                 1
#define PROCESSID             2
#define GETAYHEADER           3

//TZX ID Tasks
#define READPARAM             0
#define PILOT                 1
#define SYNC1                 2
#define SYNC2                 3
#define DATA                  4
#define PAUSE                 5
#define NEWPARAM              6
#define NAME                  7
#define GAP                   8
#define SYNCLAST              9
#define NAME00                10


//Spectrum Standards
#define PILOTLENGTH           619
#define SYNCFIRST             191
#define SYNCSECOND            210
#define ZEROPULSE             244
#define ONEPULSE              489
#define PILOTNUMBERL          8063
#define PILOTNUMBERH          3223
#define PAUSELENGTH           1000   

//ZX81 Standards
#define ZX80PULSE                 160
//#define ZX80TURBOPULSE            150
#define ZX80TURBOPULSE            120

#define ZX80BITGAP                1442
//#define ZX80TURBOBITGAP           1340
#define ZX80TURBOBITGAP           500

//ZX81 Pulse Patterns - Zero Bit  - HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, GAP
//                    - One Bit   - HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, GAP

// AY Header offset start
#define HDRSTART              0
PROGMEM const byte TZXTape[7] = {'Z','X','T','a','p','e','!'};
PROGMEM const byte TAPcheck[7] = {'T','A','P','t','a','p','.'};
PROGMEM const byte ZX81Filename[9] = {'T','Z','X','D','U','I','N','O',0x9D};
PROGMEM const byte AYFile[8] = {'Z','X','A','Y','E','M','U','L'};           // added additional AY file header check
PROGMEM const byte TAPHdr[20] = {0x0,0x0,0x3,'Z','X','A','Y','F','i','l','e',' ',' ',0x1A,0xB,0x0,0xC0,0x0,0x80,0x6E}; // 
//const char TAPHdr[24] = {0x13,0x0,0x0,0x3,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x1A,0xB,0x0,0xC0,0x0,0x80,0x52,0x1C,0xB,0xFF};
//Main Variables
byte AYPASS = 0;
byte hdrptr = 0;
byte blkchksum = 0;
byte EndOfFile=false;
word ayblklen = 0;
byte casduino = 0;
#ifdef ID11CDTspeedup
byte AMScdt = 0;
#endif

volatile byte pinState=LOW;
volatile byte isPauseBlock = false;
volatile byte wasPauseBlock = false;
volatile byte intError = false;
volatile byte workingBuffer=0;
byte outByte=0;
word pauseLength=0;
word outWord=0;
unsigned long outLong=0;
unsigned long bytesToRead=0;
word pilotPulses=0;
word pilotLength=0;
word sync1Length=0;
word sync2Length=0;
word zeroPulse=0;
word onePulse=0;
byte passforZero=2;
byte passforOne=4;

byte oneBitPulses = 4;
byte zeroBitPulses = 2;
//byte startBits = 1;
byte startBitPulses = 2;
byte startBitValue = 0;
//byte stopBits = 2;
byte stopBitPulses = 8;
byte stopBitValue = 1;
byte endianness = 0;      //0:LSb 1:MSb (default:0)
byte parity = 0 ;        //0:NoParity 1:ParityOdd 2:ParityEven (default:0)
byte UEFSWITCHPARITY = 1;
byte bitChecksum = 0;     // 0:Even 1:Odd number of one bits

word SampleLength=0;
byte usedBitsInLastByte=8;
word loopCount=0;
byte seqPulses=0;
word temppause=0;
byte forcePause0=0;
byte firstBlockPause = false;
unsigned long loopStart=0;
volatile byte currentChar=0;
volatile byte currentByte=0;

#ifdef BLKBIGSIZE
  word block = 0;
#else
  byte block = 0;
#endif

byte jblks = 1;
byte oldMinBlock = 0;
#ifdef BLOCK_EEPROM_PUT
  byte oldMaxBlock = 99;
#else
  byte oldMaxBlock = 19;
#endif

PROGMEM const char UEFFile[9] = {'U','E','F',' ','F','i','l','e','!'};
#define UEF                 0xFA    //UEF file for ID list
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

//TZX File Tasks for UEF
#define GETUEFHEADER          4
#define GETCHUNKID            5
#define PROCESSCHUNKID        6

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

#ifdef TURBOBAUD1500
#define UEFTURBOPILOTPULSES       outWord<<2
#define UEFTURBOPILOTLENGTH       156
#define UEFTURBOZEROPULSE         332
#define UEFTURBOONEPULSE          166
#endif

#ifdef TURBOBAUD1550
#define UEFTURBOPILOTPULSES       320
#define UEFTURBOPILOTLENGTH       161
#define UEFTURBOZEROPULSE         322
#define UEFTURBOONEPULSE          161
#endif

#ifdef TURBOBAUD1600
#define UEFTURBOPILOTPULSES       320
#define UEFTURBOPILOTLENGTH       156
#define UEFTURBOZEROPULSE         313
#define UEFTURBOONEPULSE          156
#endif

#define DEBUG 0
unsigned long debugCount=0;

word chunkID = 0;
// Set uefTurboMode to 0 if the default is 1200 baud. Set to 1 if the default is turbo speed. Holding doown 'ROOT' button on poweron, toggles this
byte uefTurboMode=0;
float outFloat;
byte UEFPASS = 0;

//#define ORICZEROPULSE     416
#define ORICZEROLOWPULSE  208
#define ORICZEROHIGHPULSE 416
#define ORICONEPULSE      208
//#define ORICONELOWPULSE   208
//#define ORICONEHIGHPULSE  208

//#define ORICTURBOZEROPULSE     416
#define ORICTURBOZEROLOWPULSE  60
#define ORICTURBOZEROHIGHPULSE 470
#define ORICTURBOONEPULSE      60
//#define ORICTURBOONELOWPULSE   208
//#define ORICTURBOONEHIGHPULSE  208


