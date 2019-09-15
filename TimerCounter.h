/*
 *  Interrupt and PWM utilities for 16 bit Timer1 on ATmega168/328
 *  Original code by Jesse Tane for http://labs.ideo.com August 2008
 */

#define TIMER1_A_PIN   9
#define TIMER1_B_PIN   10
#define TIMER1_ICP_PIN 8
#define TIMER1_CLK_PIN 5

#define TIMER1_RESOLUTION 65536UL  // Timer1 is 16 bit

// Placing nearly all the code in this .h file allows the functions to be
// inlined by the compiler.  In the very common case with constant values
// the compiler will perform all calculations and simply write constants
// to the hardware registers (for example, setPeriod).


class TimerCounter
{
#ifdef __AVR_ATmega4809__
  public:
    //****************************
    //  Configuration
    //****************************
    void initialize(unsigned long microseconds=1000000) __attribute__((always_inline)) {
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_DSBOTTOM_gc;        // set mode as DSBOTTOM, stop the timer
    //TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV64_gc) | (TCA_SINGLE_ENABLE_bm);
    //TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_ENABLE_bm);     //stop the timer   
    /* disable event counting */
    //TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    setPeriod(microseconds);
    }
    void setPeriod(unsigned long microseconds) __attribute__((always_inline)) {
      //const unsigned long cycles = 16 * microseconds;  //WGMODE_NORMAL
   //DSBOTTOM: the counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2
  const unsigned long cycles = (F_CPU / 2000000) * microseconds;
/*
  if (cycles < TIMER1_RESOLUTION * 64) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV64_gc;    
    pwmPeriod = cycles / 64;
  } else {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV64_gc;    
    pwmPeriod = TIMER1_RESOLUTION - 1;
  }  
*/
/*
  if (cycles < TIMER1_RESOLUTION) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV1_gc;
    pwmPeriod = cycles;
  } else
  if (cycles < TIMER1_RESOLUTION * 2) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV2_gc;
    pwmPeriod = cycles / 2;
  } else 
  if (cycles < TIMER1_RESOLUTION * 4) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV4_gc;
    pwmPeriod = cycles / 4;
  } else
  if (cycles < TIMER1_RESOLUTION * 8) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV8_gc;
    pwmPeriod = cycles / 8;
  } else
  if (cycles < TIMER1_RESOLUTION * 16) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV16_gc;
    pwmPeriod = cycles / 16;
  } else */
  if (cycles < TIMER1_RESOLUTION * 64) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV64_gc;    
    pwmPeriod = cycles / 64;
  } else
  if (cycles < TIMER1_RESOLUTION * 256) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV256_gc;   
    pwmPeriod = cycles / 256;
  } else
  if (cycles < TIMER1_RESOLUTION * 1024) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV1024_gc;
    pwmPeriod = cycles / 1024;
  } else {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV1024_gc;    
    pwmPeriod = TIMER1_RESOLUTION - 1;
  }

  //ICR1 = pwmPeriod;
    TCA0.SINGLE.PER = pwmPeriod;
    //TCA0.SINGLE.PER = cycles / 64;
                                          // set clock source and start timer
    TCA0.SINGLE.CTRLA = clockSelectBits | TCA_SINGLE_ENABLE_bm;  
    }

    //****************************
    //  Run Control
    //****************************
    void start() __attribute__((always_inline)) {
  //TCCR1B = 0;
  //TCA0.SINGLE.CTRLB=TCA_SINGLE_WGMODE_NORMAL_gc;
  //TCA0.SINGLE.CTRLA = ~(TCA_SINGLE_ENABLE_bm);
  //TCNT1 = 0;    // TODO: does this cause an undesired interrupt?
  //TCA0.SINGLE.CNT = 0;
  resume();
    }
    void stop() __attribute__((always_inline)) {
  //TCCR1B = _BV(WGM13);
  //TCA0.SINGLE.CTRLB=TCA_SINGLE_WGMODE_NORMAL_gc;
  //TCA0.SINGLE.CTRLA =0;
  TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_ENABLE_bm);
    }
    void restart() __attribute__((always_inline)) {
  start();
    }
    void resume() __attribute__((always_inline)) {
  //TCCR1B = _BV(WGM13) | clockSelectBits;
    //TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    //TCA0.SINGLE.CTRLA = clockSelectBits                    /* set clock source (sys_clk/256) */
                //      | TCA_SINGLE_ENABLE_bm;                /* start timer */
   TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;                 
    }

    //****************************
    //  Interrupt Function
    //****************************
    void attachInterrupt(void (*isr)()) __attribute__((always_inline)) {
  isrCallback = isr;
  //TIMSK1 = _BV(TOIE1);
     /* enable overflow interrupt */
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm;
    }
    void attachInterrupt(void (*isr)(), unsigned long microseconds) __attribute__((always_inline)) {
  if(microseconds > 0) setPeriod(microseconds);
  attachInterrupt(isr);
    }
    void detachInterrupt() __attribute__((always_inline)) {
  //TIMSK1 = 0;
     /* disable overflow interrupt */
    TCA0.SINGLE.INTCTRL &= ~(TCA_SINGLE_OVF_bm);
    }
    static void (*isrCallback)();

  private:
    // properties
    static unsigned short pwmPeriod;
    static unsigned char clockSelectBits;
    
#else  //__AVR_ATmega328P__

  public:
    //****************************
    //  Configuration
    //****************************
    void initialize(unsigned long microseconds=1000000) __attribute__((always_inline)) {
  TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer
  TCCR1A = 0;                 // clear control register A 
  setPeriod(microseconds);
    }
    void setPeriod(unsigned long microseconds) __attribute__((always_inline)) {
  const unsigned long cycles = (F_CPU / 2000000) * microseconds;
  if (cycles < TIMER1_RESOLUTION) {
    clockSelectBits = _BV(CS10);
    pwmPeriod = cycles;
  } else
  if (cycles < TIMER1_RESOLUTION * 8) {
    clockSelectBits = _BV(CS11);
    pwmPeriod = cycles / 8;
  } else
  if (cycles < TIMER1_RESOLUTION * 64) {
    clockSelectBits = _BV(CS11) | _BV(CS10);
    pwmPeriod = cycles / 64;
  } else
  if (cycles < TIMER1_RESOLUTION * 256) {
    clockSelectBits = _BV(CS12);
    pwmPeriod = cycles / 256;
  } else
  if (cycles < TIMER1_RESOLUTION * 1024) {
    clockSelectBits = _BV(CS12) | _BV(CS10);
    pwmPeriod = cycles / 1024;
  } else {
    clockSelectBits = _BV(CS12) | _BV(CS10);
    pwmPeriod = TIMER1_RESOLUTION - 1;
  }
  ICR1 = pwmPeriod;
  TCCR1B = _BV(WGM13) | clockSelectBits;
    }

    //****************************
    //  Run Control
    //****************************
    void start() __attribute__((always_inline)) {
  TCCR1B = 0;
  TCNT1 = 0;    // TODO: does this cause an undesired interrupt?
  resume();
    }
    void stop() __attribute__((always_inline)) {
  TCCR1B = _BV(WGM13);
    }
    void restart() __attribute__((always_inline)) {
  start();
    }
    void resume() __attribute__((always_inline)) {
  TCCR1B = _BV(WGM13) | clockSelectBits;
    }

    //****************************
    //  Interrupt Function
    //****************************
    void attachInterrupt(void (*isr)()) __attribute__((always_inline)) {
  isrCallback = isr;
  TIMSK1 = _BV(TOIE1);
    }
    void attachInterrupt(void (*isr)(), unsigned long microseconds) __attribute__((always_inline)) {
  if(microseconds > 0) setPeriod(microseconds);
  attachInterrupt(isr);
    }
    void detachInterrupt() __attribute__((always_inline)) {
  TIMSK1 = 0;
    }
    static void (*isrCallback)();

  private:
    // properties
    static unsigned short pwmPeriod;
    static unsigned char clockSelectBits;
#endif

};

//extern TimerCounter Timer1;

#ifdef __AVR_ATmega4809__

/////////////////////////////////////////////////////////////////
/////   millis() and delay() reimplementation with TimerB   /////
/////////////////////////////////////////////////////////////////

// the whole number of milliseconds per timer overflow
uint16_t millis_inc;

// the fractional number of milliseconds per timer overflow
uint16_t fract_inc;
#define FRACT_MAX (1000)

volatile uint32_t mytimer_overflow_count = 0;
volatile uint32_t mytimer_millis = 0;
static uint16_t mytimer_fract = 0;
// the prescaler is set so that timer ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
volatile uint16_t microseconds_per_timer_overflow;
volatile uint16_t microseconds_per_timer_tick;
static volatile TCB_t* _timer = &TCB0;

// timer.h

#define TIME_TRACKING_TIMER_PERIOD    0xFF
#define TIME_TRACKING_TICKS_PER_OVF   (TIME_TRACKING_TIMER_PERIOD + 1)  // Timer ticks per overflow of TCB3
#define TIME_TRACKING_TIMER_DIVIDER   2    // Clock divider for TCB0
#define TIME_TRACKING_CYCLES_PER_OVF  (TIME_TRACKING_TICKS_PER_OVF * TIME_TRACKING_TIMER_DIVIDER)

void timerBinit() {   // needed for things like clocks, timers and uart
                // Para complementar la funcion init() en setup()
                //
                //After any reset, CLK_MAIN is provided by the 16/20 MHz Oscillator (OSC20M) 
                //and with a prescaler division factor of 6. The actual frequency of the OSC20M is determined 
                //by the Frequency Select bits (FREQSEL) of the Oscillator Configuration fuse (FUSE.OSCCFG). 
                //Refer to the description of FUSE.OSCCFG for details of the possible frequencies after reset. 

  /* Enable writing to protected register */
  //CPU_CCP = CCP_IOREG_gc;
  /* Disable CLK_PER Prescaler */
  //CLKCTRL.MCLKCTRLB = 0 << CLKCTRL_PEN_bp;
  /* Enable Prescaler and set Prescaler Division to 64 */
      _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0 << CLKCTRL_PEN_bp );  // Prescaler Disabled
  //CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_64X_gc | CLKCTRL_PEN_bm;
      //_PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);  // OSC20M
  /* Enable writing to protected register */
  //CPU_CCP = CCP_IOREG_gc;
  /* Select 32KHz Internal Ultra Low Power Oscillator (OSCULP32K) */
  //CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSCULP32K_gc;
  /* Wait for system oscillator changing to finish */
  //while(CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm){;}
 
  // Calculate relevant time tracking values
  microseconds_per_timer_overflow = clockCyclesToMicroseconds(TIME_TRACKING_CYCLES_PER_OVF);
  microseconds_per_timer_tick = microseconds_per_timer_overflow/TIME_TRACKING_TIMER_PERIOD;

  millis_inc = microseconds_per_timer_overflow / 1000;
  fract_inc = ((microseconds_per_timer_overflow % 1000));

  // Default Periodic Interrupt Mode
  // TOP value for overflow every 1024 clock cycles
  _timer->CCMP = TIME_TRACKING_TIMER_PERIOD;

  // Enable timer interrupt
  _timer->INTCTRL |= TCB_CAPT_bm;

  // Clock selection -> same as TCA (F_CPU/64 -- 250kHz)
 // _timer->CTRLA = TCB_CLKSEL_CLKTCA_gc;   
  _timer->CTRLA = TCB_CLKSEL_CLKDIV2_gc;

  // Enable & start
  _timer->CTRLA |= TCB_ENABLE_bm; // Keep this last before enabling interrupts to ensure tracking as accurate as possible 

             
} 

ISR(TCB0_INT_vect)
{
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access)
  uint32_t m = mytimer_millis;
  uint16_t f = mytimer_fract;

  m += millis_inc;
  f += fract_inc;
  if (f >= FRACT_MAX) {

    f -= FRACT_MAX;
    m += 1;
  }

  mytimer_fract = f;
  mytimer_millis = m;
  mytimer_overflow_count++;

  // Clear flag
  _timer->INTFLAGS = TCB_CAPT_bm;
}

unsigned long mymicros() {
  unsigned long overflows, microseconds;
  uint8_t ticks;

  // Save current state and disable interrupts 
  uint8_t mystatus = SREG;
  cli();

  // Get current number of overflows and timer count
  overflows = mytimer_overflow_count;
  ticks = _timer->CNTL;

  // If the timer overflow flag is raised, we just missed it,
  //increment to account for it, & read new ticks
  if(_timer->INTFLAGS & TCB_CAPT_bm){
    overflows++;
    ticks = _timer->CNTL;
  }

  // Restore state
  SREG = mystatus;

  // Return microseconds of up time  (resets every ~70mins)
  microseconds = ((overflows * microseconds_per_timer_overflow)
        + (ticks * microseconds_per_timer_tick));
  return microseconds;
}
unsigned long mymillis()
{
  unsigned long m;

  // disable interrupts while we read timer0_millis or we might get an
  // inconsistent value (e.g. in the middle of a write to timer0_millis)
  uint8_t mystatus = SREG;
  cli();
  m = mytimer_millis;

  SREG = mystatus;

  return m;
}

void mydelay(unsigned long ms)
{
  uint32_t start_time = mymicros(), delay_time = 1000*ms;

  // Calculate future time to return 
  uint32_t return_time = start_time + delay_time;

  // If return time overflows 
  if(return_time < delay_time){
    // Wait until micros overflows
    while(mymicros() > return_time);
  }

  // Wait until return time
  while(mymicros() < return_time);
}

inline uint16_t clockCyclesPerMicrosecondComp(uint32_t clk){
  return ( (clk) / 1000000L );
}

inline uint16_t clockCyclesPerMicrosecond(){
  return clockCyclesPerMicrosecondComp(16000000);
}

inline unsigned long clockCyclesToMicroseconds(unsigned long cycles){
  return ( cycles / clockCyclesPerMicrosecond() );
}

inline unsigned long microsecondsToClockCycles(unsigned long microseconds){
  return ( microseconds * clockCyclesPerMicrosecond() );
}
#endif

