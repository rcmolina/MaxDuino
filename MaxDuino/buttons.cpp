#include "configs.h"
#include "Arduino.h"
#include "buttons.h"
#include "pinSetup.h"

bool lastbtn=true;

#if defined(BUTTON_ADC)

void setup_buttons(void)
{
  pinMode(btnADC, INPUT);
  #if !defined(ESP8266)
  // analogReadResolution is only defined on certain platforms (including SAMD21 and ESP32, but not including ESP8266)
  // For some devices, the resolution is 10 bits by default (which is why we set other platforms to also use 10 bits
  // so that all the same code works on all the devices)
  // ESP8266 defaults to 10 bits anyway
  analogReadResolution(10);
  #endif

  #if defined(ESP32)
  // ESP32 has additional options for setting ADC range
  analogSetAttenuation(ADC_11db);
  analogSetClockDiv(255);
  #endif
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

void checkLastButton()
{
  if(!button_down() && !button_up() && !button_play() && !button_stop()) lastbtn=false; 
  delay(50);
}
