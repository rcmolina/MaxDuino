# MaxDuino Devices

Various MaxDuino and TZXDuino devices have been manufactured over the last 15 years or so, and these have been constructed with different combinations of hardware components (microcontroller type, LCD type, etc).  We provide MaxDuino software intended to be compatible with all of them and more.

Our configuration-based build process enables you to customise a configuration that matches your particular hardware.  The below sections outline the components for which we include support.


## Commercial devices

Many TZXDuino and MaxDuino have been produced by a variety of individual suppliers and businesses.  The MaxDuino firmware project is not directly affiliated with any of the manufacturers of these products.
MaxDuino firmware is itself an open-source project and remains free for everybody.

* TZXDuino
  * typically, Arduino Nano 328p, with either 128x64 GRAPHICAL OLED or 16x2 TEXT LCD
* MaxDuino Ultimate
  * Thinary Nano Every + 128x64 OLED
* MaxDuino Next
  * ???


## Microcontroller type

*  Arduino Nano 328p
*  Arduino Nano Every
*  Thinary Nano Every
*  Arduino Mega 2560
*  Seeeduino Seeed Xiao M0 (SAMD21) (experimental)
*  Espressif ESP8266 (experimental)
*  Espressif ESP32C3 (experimental)
*  STMicroelectonics STM32 (experimental, needs testing)

## LCD type

*  128x64 GRAPHICAL OLED (SPI 1306-compatible)
*  16x2 TEXT LCD (12C PCF8574T-compatible)
*  LCD (Nokia P8544-compatible)  (see [Note 1][#notes])

## Output type

*  Audio jack
*  (Optional) Motor control line

## Inputs type

*  5 buttons (Up, Down, Left, Play, Pause, Stop)  (See [Note 2][#notes])
**  Most devices support multi-press combinations
**  Some devices only support one-button-at-a-time due to lack of indepdent GPIOs e.g. ESP32C3, Xiao M0
*  Power button, aka "soft-off" (for supported devices only e.g. ESP32C3) (See [Note 3][#notes]])

## Prebuilt configurations

Several combinations are available 'prebuilt' and included in GitHub releases: https://github.com/rcmolina/MaxDuino/releases
You can create your own combinations by editing the configuration files and building your own firmware - see [BUILDING](./BUILDING.md)

# Notes
1. Support for P8544 has been removed due to lack of devices for testing and development purposes
2. Devices may have a reset button but this directly resets the device via a hardware reset signal, and is not handled by the firmware
3. Currently, soft-off is implemented as a long-press on the Stop button, but future hardware iterations might include a separate GPIO power button
