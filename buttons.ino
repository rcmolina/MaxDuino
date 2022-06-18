#include "buttons.h"
#include "pinSetup.h"

#if defined(BUTTON_ADC)

void setup_buttons(void)
{
  pinMode(btnADC, INPUT_PULLUP);
}

  // todo - use isr to capture buttons?

bool button_any() {
  int sensorValue = analogRead(btnADC);
  return(sensorValue>=btnADCUpLow);
}

bool button_play()
{
  int sensorValue = analogRead(btnADC);
  return(sensorValue>=btnADCPlayLow);
}

bool button_stop()
{
  int sensorValue = analogRead(btnADC);
  return(sensorValue>=btnADCStopLow && sensorValue<btnADCPlayLow);
}

bool button_root()
{
  int sensorValue = analogRead(btnADC);
  return(sensorValue>=btnADCRootLow && sensorValue<btnADCStopLow);
}

bool button_down()
{
  int sensorValue = analogRead(btnADC);
  return(sensorValue>=btnADCDownLow && sensorValue<btnADCRootLow);
}

bool button_up()
{
  int sensorValue = analogRead(btnADC);
  return(sensorValue>=btnADCUpLow && sensorValue<btnADCDownLow);
}

#else // !defined (BUTTON_ADC)

void setup_buttons(void)
{
  // 
}

bool button_any() {
  return (digitalRead(btnPlay) == LOW || 
  digitalRead(btnStop) == LOW ||
  digitalRead(btnUp)   == LOW ||
  digitalRead(btnDown) == LOW ||
  digitalRead(btnRoot) == LOW);
}

bool button_play() {
  return(digitalRead(btnPlay) == LOW);
}

bool button_stop() {
  return(digitalRead(btnStop) == LOW);
}

bool button_root() {
  return(digitalRead(btnRoot) == LOW);
}

bool button_down() {
  return(digitalRead(btnDown) == LOW);
}

bool button_up() {
  return(digitalRead(btnUp) == LOW);
}

#endif // defined(BUTTON_ADC)


// common:
void debounce(bool (*button_fn)()) {
  while(button_fn()) {
    //prevent button repeats by waiting until the button is released.
    delay(50);
  }
}

void debouncemax(bool (*button_fn)())
{
  //prevent button repeats by waiting until the button is released.
  // return true or false, depending whether we hit the timeout (true) or key was released (false) before timeout occurred
  for(byte i=4; i>0; i--)
  {
    if (!button_fn()) break;
    delay(50);
  }
}
