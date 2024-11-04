#include "configs.h"
#include "pinSetup.h"  // for BUTTON_ADC but then really that should move to configs.h ?

#ifdef SOFT_POWER_OFF

#include "Arduino.h"
#include "Display.h"
#include "buttons.h"
#include "power.h"
#include "file_utils.h"

unsigned long soft_poweroff_timer = 0;
unsigned long poweroff_millis = 0;
bool b_poweroff_key_detected = false;

#ifndef POWER_OFF_LOGO
#define POWER_OFF_LOGO DEFAULT
//#define POWER_OFF_LOGO SQUARED
//#define POWER_OFF_LOGO BIGBUTTON
#endif

#ifdef BUTTON_ADC
#define WAKEUP_BUTTON_PIN_BITMASK (1<<btnADC)
#else
#define WAKEUP_BUTTON_PIN_BITMASK ((1<<btnPlay) | (1<<btnStop))
#endif

#include "power_logos.h"

static void device_power_off();

static void show_power_off_logo()
{
#ifdef OLED1306
  clear_display();

  #if defined(OLED1306_128_64) || defined(video64text32)
  for(int j=0;j<8;j++) {
  #else
  for(int j=0;j<4;j++) {
  #endif
    setXY(0,j);
    for(int i=0;i<128;i++)
    {
      SendByte(*(power_logo+j*128+i));
    }
  }
#endif
}

static void power_off_clear_display()
{
#ifdef OLED1306
  clear_display();
  sendcommand(0x8D); // charge pump
  sendcommand(0x10); // disable charge pump
  sendcommand(0x81); // contrast
  sendcommand(0x01); // lowest contrast
  displayOff();
#endif
}

void check_power_off_key()
{
  if(button_stop())
  {
    if(!b_poweroff_key_detected)
    {
      b_poweroff_key_detected = true;
      poweroff_millis = millis();
    }
    else
    {
      unsigned long tdiff = millis() - poweroff_millis;
      poweroff_millis = millis();
      soft_poweroff_timer += tdiff;
      if (soft_poweroff_timer >= SOFT_POWER_OFF)
      {
        power_off();
      }
    }
  }
  else
  {
    clear_power_off();
  }
}

void clear_power_off()
{
  soft_poweroff_timer = 0;
  poweroff_millis = 0;
  b_poweroff_key_detected = false;
}

void power_off()
{
  show_power_off_logo();
 
  // small delay to show poweroff logo, and wait for button to be released
  delay(1000);
  while(button_any()) delay(100);

  power_off_clear_display();

  // ensure SD is finished before we stop everything (common)
  entry.close();
  currentDir->close();
  _tmpdirs[0].close();
  _tmpdirs[1].close();
  sd.end();

  device_power_off();
}

// device-specific poweroff actions

#if defined(ESP32)
#include "esp_sleep.h"
#include "driver/periph_ctrl.h"

static void device_power_off()
{
  // in theory, reduce power consumption more by disabling the I2C pullups
  // see https://arduino.stackexchange.com/questions/81759/cant-turn-off-internal-i2c-pull-ups-on-esp32
  //rtc_gpio_isolate(gpio_num_t(SDA));
  //rtc_gpio_isolate(gpio_num_t(SCL));
  esp_sleep_config_gpio_isolate();
  periph_module_disable(PERIPH_I2C0_MODULE);  
  digitalWrite(SDA, 0);
  digitalWrite(SCL, 0);

  // set up triggers for wakeup
  #ifdef BUTTON_ADC
  pinMode(btnADC, INPUT); // treat buttons now as digital input instead of analog
  #endif
  esp_deep_sleep_enable_gpio_wakeup(WAKEUP_BUTTON_PIN_BITMASK, ESP_GPIO_WAKEUP_GPIO_HIGH);
  esp_deep_sleep_start();
  // wakeup will reboot
}

#endif

#endif
