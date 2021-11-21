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
#if  defined(__arm__) && defined(__STM32F1__)
//clase derivada
class TimerCounter:public HardwareTimer
{
  public:
    TimerCounter(uint8 timerNum) : HardwareTimer(timerNum) {};
    void setSTM32Period(unsigned long microseconds) __attribute__((always_inline)) {
      
/*    if (microseconds < 65536/36) {
      this->setPrescaleFactor(F_CPU/1000000/36);
      this->setOverflow(microseconds*36);
    }else
    if (microseconds < 65536/18) {
      this->setPrescaleFactor(F_CPU/1000000/18);
      this->setOverflow(microseconds*18);
    }else */
    if (microseconds < 65536/24) {
      this->setPrescaleFactor(F_CPU/1000000/24);
      this->setOverflow(microseconds*24);
    }else    
/*    if (microseconds < 65536/16) {
      this->setPrescaleFactor(F_CPU/1000000/16);
      this->setOverflow(microseconds*16);
    }else */
    if (microseconds < 65536/8) {
      this->setPrescaleFactor(F_CPU/1000000/8);
      this->setOverflow(microseconds*8);
    }else
    if (microseconds < 65536/4) {
      this->setPrescaleFactor(F_CPU/1000000/4);
      this->setOverflow(microseconds*4);
    }else
    if (microseconds < 65536/2) {
      this->setPrescaleFactor(F_CPU/1000000/2);
      this->setOverflow(microseconds*2);
    }else
    if (microseconds < 65536) {
      this->setPrescaleFactor(F_CPU/1000000);
      this->setOverflow(microseconds);
    }else
    if (microseconds < 65536*2) {
      this->setPrescaleFactor(F_CPU/1000000*2);
      this->setOverflow(microseconds/2);
    }else
    if (microseconds < 65536*4) {
      this->setPrescaleFactor(F_CPU/1000000*4);
      this->setOverflow(microseconds/4);
    }else
    if (microseconds < 65536*8) {
      this->setPrescaleFactor(F_CPU/1000000*8);
      this->setOverflow(microseconds/8);
    }else
/*    if (microseconds < 65536*16) {
      this->setPrescaleFactor(F_CPU/1000000*16);
      this->setOverflow(microseconds/16);
    }else
    if (microseconds < 65536*32) {
      this->setPrescaleFactor(F_CPU/1000000*32);
      this->setOverflow(microseconds/32);
    }else */
    if (microseconds < 65536*64) {
      this->setPrescaleFactor(F_CPU/1000000*64);
      this->setOverflow(microseconds/64);
    }else
/*    if (microseconds < 65536*128) {
      this->setPrescaleFactor(F_CPU/1000000*128);
      this->setOverflow(microseconds/128);
    }else
    if (microseconds < 65536*256) {
      this->setPrescaleFactor(F_CPU/1000000*256);
      this->setOverflow(microseconds/256);
    }else */
    if (microseconds < 65536*512) {                                    
      this->setPrescaleFactor(F_CPU/1000000*512);
      this->setOverflow(microseconds/512);
    }else {                           
      this->setPrescaleFactor(F_CPU/1000000*512);
      this->setOverflow(65535);      
    }
    this->refresh();
    }

};
#else

#if defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__)
class TimerCounter
{
  public:
    //****************************
    //  Configuration
    //****************************
    void initialize(unsigned long microseconds=1000000) __attribute__((always_inline)) {
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_DSBOTTOM_gc;        // set mode as DSBOTTOM, stop the timer
    //TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;        // set mode as SINGLESLOPE, stop the timer
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
   if (cycles < TIMER1_RESOLUTION * 1) {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV1_gc;    
    pwmPeriod = cycles / 1;
 } else {
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV64_gc;    
    pwmPeriod = TIMER1_RESOLUTION - 1;
  }
*/  
 
  if (cycles < TIMER1_RESOLUTION) {
    //clockSelectBits = _BV(CS10);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV1_gc;
    pwmPeriod = cycles;
  } else
  if (cycles < TIMER1_RESOLUTION * 2) {
    //clockSelectBits = _BV(CS10);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV2_gc;
    pwmPeriod = cycles / 2;
  } else 
  if (cycles < TIMER1_RESOLUTION * 4) {
    //clockSelectBits = _BV(CS10);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV4_gc;
    pwmPeriod = cycles / 4;
  } else  
  if (cycles < TIMER1_RESOLUTION * 8) {
    //clockSelectBits = _BV(CS11);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV8_gc;
    pwmPeriod = cycles / 8;
  } else
  if (cycles < TIMER1_RESOLUTION * 16) {
    //clockSelectBits = _BV(CS10);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV16_gc;
    pwmPeriod = cycles / 16;
  } else
  if (cycles < TIMER1_RESOLUTION * 64) {
    //clockSelectBits = _BV(CS11) | _BV(CS10);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV64_gc;    
    pwmPeriod = cycles / 64;
  } else
  if (cycles < TIMER1_RESOLUTION * 256) {
    //clockSelectBits = _BV(CS12);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV256_gc;   
    pwmPeriod = cycles / 256;
  } else
  if (cycles < TIMER1_RESOLUTION * 1024) {
    //clockSelectBits = _BV(CS12) | _BV(CS10);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV1024_gc;
    pwmPeriod = cycles / 1024;
  } else {
    //clockSelectBits = _BV(CS12) | _BV(CS10);
    clockSelectBits = TCA_SINGLE_CLKSEL_DIV1024_gc;    
    pwmPeriod = TIMER1_RESOLUTION - 1;
  }
 
  //ICR1 = pwmPeriod;
    TCA0.SINGLE.PER = pwmPeriod;
    //TCA0.SINGLE.PER = cycles / 64;
                                          // set clock source and start timer
    TCA0.SINGLE.CTRLA = clockSelectBits | TCA_SINGLE_ENABLE_bm;
  //TCA0.SINGLE.INTCTRL |= (TCA_SINGLE_OVF_bm); // Enable timer interrupts on overflow on timer A   
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
};    
#else  //__AVR_ATmega328P__
class TimerCounter
{
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
};    
#endif

#endif
//extern TimerCounter Timer1;



