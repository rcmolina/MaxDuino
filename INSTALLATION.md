# MaxDuino Installation

If you have downloaded the files from the 'Releases' section of GitHub, go to the [Installing Releases][#Installing Releases] section below.

If you have built your own variant using PlatformIO or Arduino IDE, the instructions for uploading to your device are on the [Building][BUILDING.md] page.

## Releases

The primary repository for MaxDuino is: https://github.com/rcmolina/MaxDuino.
We automatically generate new releases of firmware binaries for every commit on the "master" branch, provided that the automatic build scripts have succeeded with no error messages.

We also aim to produce release packages with tagged versions whenever there is a major or minor version bump.

## Installing A Release

You can download prebuilt releases from the github Releases page and upload them to your device
using the appropriate tools for your device

### ARDUINO NANO (ATMEGA328P)

On Arduino Nano you can follow these instructions: https://forum.arduino.cc/t/using-avrdude-to-push-sketch-to-arduino/525745

### ARDUINO MEGA

TBD

### ARDUINO EVERY

TBD

### THINARY EVERY

TBD

### SEEED XIAO M0 (SAMD21)

TBD

### ESP8266

On Windows, I use Platform IO with Visual Studio Code, which includes latest versions of esptool.  (For example, I find these in `%USERPROFILE%/.platformio/packages/tool-esptool` folder)
`esptool.exe -v -cp COM4 -cd nodemcu -cf .\ESP8266_firmware_ESP8266.bin`

You may need to adjust the com port and/or maybe also the reset method (-cd) ... use esptool -h for more help

You can also use a graphical tool: https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html .  Enter the downloaded .bin file into the first file entry using the ... button, set the address to 0x00000 , and check the checkbox for that row.  Clicking Start should then upload directly to the device.  You may then need to manually reset the device.

Please note that we do not support these tools, so if you are having trouble with the tools please contact the supplier of the tools.

### ESP32C3

TBD

### STM32

For more details, please see [STM32 INSTALLATION][STM32_INSTALLATION.md]
