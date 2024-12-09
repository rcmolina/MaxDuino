#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

typedef void (*timerCallback) ();

class TimerCounter
{
  public:
    TimerCounter();
    void initialize(unsigned long microseconds);
    void setPeriod(unsigned long microseconds);
    void stop();
    void attachInterrupt(timerCallback isr);
};
extern TimerCounter& Timer;

#endif // TIMER_H_INCLUDED