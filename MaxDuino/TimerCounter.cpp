#include "configs.h"
#include "Arduino.h"
#include "TimerCounter.h"

/*
 *  Interrupt and PWM utilities for 16 bit Timer1 on ATmega168/328
 *  Original code by Jesse Tane for http://labs.ideo.com August 2008
 */

timerCallback isrCallback = NULL;

#if defined(__arm__) && defined(__STM32F1__)
//clase derivada
class HwTimerCounter:public HardwareTimer
{
  public:
    HwTimerCounter(uint8 timerNum) : HardwareTimer(timerNum) {};
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

#define TIMER_CHANNEL 2 // channel 2
HwTimerCounter timer_instance(TIMER_CHANNEL);

TimerCounter::TimerCounter() {};
void TimerCounter::stop()
{
  timer_instance.pause();
}

void TimerCounter::initialize(unsigned long period)
{
  // behaviour of other timers is to set period
  setPeriod(period);  
}

void TimerCounter::setPeriod(unsigned long period)
{
  timer_instance.setSTM32Period(period);
}

void TimerCounter::attachInterrupt(timerCallback isr)
{
  // behaviour of other timers is to attach interrupt and resume
  timer_instance.attachInterrupt(TIMER_CHANNEL, isr);
  timer_instance.resume();
}

#elif defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__)

#define TIMER1_RESOLUTION 65536UL  // Timer1 is 16 bit

unsigned long _current_microseconds;
TimerCounter::TimerCounter()
{
    _current_microseconds = 0;
}

void TimerCounter::initialize(unsigned long microseconds) {
    // turn off split mode (enabled at startup on TCA0 for MegaCoreX).
    // Ensure timer is stopped for this:
    TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_ENABLE_bm);     //stop the timer   
    TCA0.SPLIT.CTRLD &= ~(TCA_SINGLE_SPLITM_bm);
    TCA0.SINGLE.INTCTRL = 0; // Disable all interrupts on timer A
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm); // disable event counting
    TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESTART_gc;

    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc; // set mode as NORMAL, stop the timer
    //TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_DSBOTTOM_gc;        // set mode as DSBOTTOM, stop the timer

    //TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;        // set mode as SINGLESLOPE, stop the timer
    //TCA0.SINGLE.CTRLA |= (TCA_SINGLE_CLKSEL_DIV64_gc) | (TCA_SINGLE_ENABLE_bm);
    //TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_ENABLE_bm);     //stop the timer   
    /* disable event counting */
    //TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    setPeriod(microseconds);
}

void TimerCounter::setPeriod(unsigned long microseconds) {
    unsigned short pwmPeriod;
    unsigned char clockSelectBits;

    if (_current_microseconds == microseconds)
    {
        // nothing to do - timer is already set for the correct
        // period, and it will just wrap and repeat and retrigger
        // the interrupt anyway
        return;
    }

    _current_microseconds = microseconds;

    //DSBOTTOM: the counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2
    //const unsigned long cycles = (F_CPU / 1000000) * microseconds;

    //NORMAL: the counter runs to TOP, interrupt is at TOP
    const unsigned long cycles = (F_CPU / 1000000) * microseconds;

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
}

void TimerCounter::stop() {
    //TCCR1B = _BV(WGM13);
    //TCA0.SINGLE.CTRLB=TCA_SINGLE_WGMODE_NORMAL_gc;
    //TCA0.SINGLE.CTRLA =0;
    TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_ENABLE_bm);
}

void TimerCounter::attachInterrupt(timerCallback isr) {
    isrCallback = isr;
    //TIMSK1 = _BV(TOIE1);
    /* enable overflow interrupt */
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm;
}

ISR(TCA0_OVF_vect)
{
  if (isrCallback)
    (*isrCallback)();
  /* The interrupt flag has to be cleared manually */
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}  

#elif defined(__AVR_ATmega328P__) || defined ( __AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)

#define TIMER1_RESOLUTION 65536UL  // Timer1 is 16 bit

TimerCounter::TimerCounter(){};

void TimerCounter::initialize(unsigned long microseconds) {
    TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer
    TCCR1A = 0;                 // clear control register A 
    setPeriod(microseconds);
}

void TimerCounter::setPeriod(unsigned long microseconds) {
    const unsigned long cycles = (F_CPU / 2000000) * microseconds;
    unsigned short pwmPeriod;
    unsigned char clockSelectBits;

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
    TCCR1B = _BV(WGM13) | clockSelectBits; // starts the timer
}

void TimerCounter::stop() {
    TCCR1B = _BV(WGM13);
}

void TimerCounter::attachInterrupt(timerCallback isr) {
    isrCallback = isr;
    TIMSK1 = _BV(TOIE1);
}

ISR(TIMER1_OVF_vect)
{
  if (isrCallback)
    (*isrCallback)();
}

#elif defined(__SAMD21__)
  // The following, including the MyTC3Timer class, is a modified version of SAMDTimer (from SAMD_TimerInterrupt)
  // incorporating significant fixes to period accuracy and reducing instruction count of setInterval function
  // see https://github.com/khoih-prog/SAMD_TimerInterrupt/issues/18
  // and https://github.com/khoih-prog/SAMD_TimerInterrupt/issues/17
  // (see https://github.com/stripwax/SAMD_TimerInterrupt )
  // The author of SAMD_TimerInterrupt has made it clear that they have no intention to address these behaviours,
  // so we must resort to not using SAMD_TimerInterrupt...
  // As an aside, the TimerTC3 library also has the same bugs (as well as one or two others), as it is derived from
  // the same logic as SAMD_TimerInterrupt, and therefore also cannot be used by this project...
  
  static timerCallback TC3_callback;
  
  void TC3_Handler()
  {
    // get timer struct
    TcCount16* TC = (TcCount16*) TC3;
    
    // If the compare register matching the timer count, trigger this interrupt
    if (TC->INTFLAG.bit.MC0 == 1) 
    {
      if (TC3_callback)
        (*TC3_callback)();
      TC->INTFLAG.bit.MC0 = 1; // write 1 here, to clear the interrupt tr
    }
  }
  
#define SAMD_TC3        ((TcCount16*) TC3)
#include "Arduino.h"
  
unsigned long _current_microseconds;
TimerCounter::TimerCounter()
{
  _current_microseconds = 0;
  TC3_callback = NULL;
}
    
void TimerCounter::initialize(unsigned long microseconds=1000)
{
  TcCount16* _Timer = SAMD_TC3;

  noInterrupts();
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID (GCM_TCC2_TC3));
  
  while ( GCLK->STATUS.bit.SYNCBUSY);
          
  _Timer->CTRLA.reg &= ~TC_CTRLA_ENABLE; // ensure clock is disabled.  necessary, in order to configure clock, anyway.

  // Use the 16-bit timer
  _Timer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  
  while (_Timer->STATUS.bit.SYNCBUSY);

  // Use match mode so that the timer counter resets when the count matches the compare register
  _Timer->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  
  while (_Timer->STATUS.bit.SYNCBUSY);

  TC3_callback = NULL;

  setPeriod(microseconds); // this does not enable the timer, just configures it
  interrupts();
}

void TimerCounter::setPeriod(unsigned long microseconds)
{
  TcCount16* _Timer = SAMD_TC3;
  if (_current_microseconds == microseconds)
  {
    // nothing to do - timer is already set for the correct
    // period, and it will just wrap and repeat and retrigger
    // the interrupt anyway
    return;
  }

  // adjust microseconds if out of bounds:
  // 1. impose some kind of minimum cycle time, to avoid deadlock
  if (microseconds < 20)
  {
    microseconds = 20;
  }
  // 2. avoid wraparound for periods longer than the maximum permitted with the widest prescaler
  if (microseconds > 1398080)
  {
    microseconds = 1398080;
  }
  // if the adjusted microseconds matches what we previously configured
  // then again nothing to do, timer will repeat as planned
  if (_current_microseconds == microseconds)
    return;

  // otherwise, set new timer registers
  
  _current_microseconds = microseconds;
  
  bool ctrla_enabled = _Timer->CTRLA.reg & TC_CTRLA_ENABLE;
  
  _Timer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (_Timer->STATUS.bit.SYNCBUSY);

  _Timer->CTRLA.reg &= ~(TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_PRESCALER_DIV256 | TC_CTRLA_PRESCALER_DIV64 | TC_CTRLA_PRESCALER_DIV16 | TC_CTRLA_PRESCALER_DIV8 | TC_CTRLA_PRESCALER_DIV4 | TC_CTRLA_PRESCALER_DIV2 | TC_CTRLA_PRESCALER_DIV1);
  while (_Timer->STATUS.bit.SYNCBUSY);

  uint32_t cycles = (F_CPU/1000000) * microseconds;
  if (cycles > (65535*256)) 
  {
    // Set prescaler to 1024
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
    cycles >>= 10;
  } 
  else if (cycles > (65535*64))
  {
    // Set prescaler to 256
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV256;
    cycles >>= 8;
  } 
  else if (cycles > (65535*16))
  {
    // Set prescaler to 64
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV64;
    cycles >>= 6;
  } 
  else if (cycles > (65535*8))
  {
    // Set prescaler to 16
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV16;
    cycles >>= 4;
  } 
  else if (cycles > (65535*4))
  {
    // Set prescaler to 8
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV8;
    cycles >>= 3;
  } 
  else if (cycles > (65535*2))
  {
    // Set prescaler to 4
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV4;
    cycles >>= 2;
  } 
  else if (cycles > 65535)
  {
    // Set prescaler to 2
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV2;
    cycles >>= 1;
  } 
  else
  {
    // Set prescaler to 1, cycles is unchanged
    _Timer->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1;
  }
  while (_Timer->STATUS.bit.SYNCBUSY);

  _Timer->CC[0].reg = cycles;
  while (_Timer->STATUS.bit.SYNCBUSY);

  if (ctrla_enabled)
  {
    // so re-enable the clock
    _Timer->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (_Timer->STATUS.bit.SYNCBUSY); // per datasheet, sync is not required when just setting the enabled bit
  }

//    TODO: make this only disable/enable the timer (via CTRLA) if the prescaler actually changed
//    (if prescaler didn't change, you could just set the new CC[0].reg , which should be super quick to do)

//    TODO use the fancy code instead
//      // Make sure the count is in a proportional position to where it was
//      // to prevent any jitter or disconnect when changing the compare value.
//      _Timer->READREQ.reg = TC_READREQ_RREQ | TC_READREQ_ADDR(0x10); // 0x10 is the offset of the 16-bit count register
//      while (_Timer->STATUS.bit.SYNCBUSY);
//      uint16_t prev_counter = _Timer->COUNT.reg;
//
//      // cannot update ctrla at the same time as the enabled bit is set
//      _Timer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
//      while (_Timer->STATUS.bit.SYNCBUSY);
//
//      // need to synchronise the count and cc values
//      _Timer->READREQ.reg = TC_READREQ_RREQ | TC_READREQ_ADDR(0x18); // 0x18 is the offset of the 16-bit CC0 register
//      while (_Timer->STATUS.bit.SYNCBUSY);
//
//      uint16_t old_compare_value = _Timer->CC[0].reg;
//      _Timer->CC[0].reg = (uint16_t)(-1); // max, so that changing Count doesn't end up wrapping it back to zero again due to the continuous comparison
//
//      _Timer->COUNT.reg = map(prev_counter, 0, old_compare_value, 0, _compareValue);
//      while (_Timer->STATUS.bit.SYNCBUSY);
//      
//      _Timer->CC[0].reg = _compareValue;
//      while (_Timer->STATUS.bit.SYNCBUSY);
//
//      _Timer->CTRLA.reg = ctrla | TC_CTRLA_ENABLE;
//      while (_Timer->STATUS.bit.SYNCBUSY);
}
    
void TimerCounter::stop()
{
  noInterrupts();
  TcCount16* _Timer = SAMD_TC3;

  // disable the timer
  _Timer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (_Timer->STATUS.bit.SYNCBUSY);

  // Disable the compare interrupt
  SAMD_TC3->INTENCLR.reg = 0;
  SAMD_TC3->INTENCLR.bit.MC0 = 1;
  NVIC_DisableIRQ(TC3_IRQn);

  interrupts();
}

void TimerCounter::attachInterrupt(timerCallback isr)
{
  noInterrupts();

  TcCount16* _Timer = SAMD_TC3;

  // disable the timer (this might not be necessary)
  _Timer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (_Timer->STATUS.bit.SYNCBUSY);
  
  TC3_callback = isr;
  
  // Enable the compare interrupt
  SAMD_TC3->INTENSET.reg = 0;
  SAMD_TC3->INTENSET.bit.MC0 = 1;
  NVIC_EnableIRQ(TC3_IRQn);

  // enable the timer:
  _Timer->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (_Timer->STATUS.bit.SYNCBUSY);
  
  interrupts();
}

#elif defined(ESP32)

timerCallback ESPTimerCallback;

void ARDUINO_ISR_ATTR onTimer(){
  // just call the callback
  if (ESPTimerCallback)
    (*ESPTimerCallback)();
}

hw_timer_t * timer = NULL;

TimerCounter::TimerCounter()
{
  ESPTimerCallback = NULL;
}

void TimerCounter::initialize(unsigned long microseconds=1000000)
{
  ESPTimerCallback = NULL;
  if (timer==NULL)
  {
    // count microseconds - so divide CPU freq in Hz by 1e6
    timer = timerBegin(0, F_CPU/1000000, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, microseconds, true);
  }
}

void TimerCounter::setPeriod(unsigned long microseconds)
{
  timerAlarmWrite(timer, microseconds, true);
}

void TimerCounter::stop()
{
  if(timer!=NULL)
    timerAlarmDisable(timer);
}

void TimerCounter::attachInterrupt(timerCallback isr)
{
  isrCallback = isr;
  timerAlarmEnable(timer);
}

#elif defined(ESP8266)

timerCallback ESPTimerCallback;

void IRAM_ATTR onTimer(){
  // just call the callback
  if (isrCallback)
    (*isrCallback)();
}

TimerCounter::TimerCounter()
{
  ESPTimerCallback = NULL;
}

void TimerCounter::initialize(unsigned long microseconds=1000000)
{
  ESPTimerCallback = NULL;
  // Divide CPU freq in Hz (e.g. 80000000) by 1e6 (=> 80) to determine how many ticks per microsecond
  // DIV16 to reduce this by a factor of 16
  // ESP8266 timer1 is only 23 bits
  // So 1000000 us (=1 second) would need 1000000 * (80/16) ticks = 5000000 ticks
  // (which is less than 2^23 i.e. 8338608)
  timer1_isr_init();
  timer1_attachInterrupt(onTimer);
  timer1_write(microseconds*((F_CPU/1000000)/16));
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
}

void TimerCounter::setPeriod(unsigned long microseconds)
{
  timer1_write(microseconds*((F_CPU/1000000)/16));
  // timer1_write also (re)enables edge interrupts
}

void TimerCounter::stop()
{
  timer1_disable();
}

void TimerCounter::attachInterrupt(void (*isr)())
{
  ESPTimerCallback = isr;
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
}

#else
#error Missing definition of TimerCounter / unsupported device
#endif

static class TimerCounter _TimerInstance;
class TimerCounter &Timer = _TimerInstance;
