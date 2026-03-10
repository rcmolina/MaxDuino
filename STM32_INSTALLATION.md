# Instructions to upload firmware to a STM32 Maple device

To upload firmware to a Maple or Maple Mini board via USB, the board must be put into DFU (Device Firmware Update) mode.

You can achieve this if you have the `maple_upload` (or `maple_upload.sh`) script along with `dfu-util` and `usb_reset` command line utilities.

The `maple_upload` script (often used in Arduino IDE or PlatformIO) attempts to trigger DFU mode by opening the serial port at 1200 baud and sending a DTR toggle and "1EAF" string with the `usb_reset` command, and then uses `dfu-util` to upload the firmware.

## PlatformIO
If you have PlatformIO installed for your local user, the script can be found at

`~/.platformio/packages/tool-stm32duino/maple_upload`
for Linux/MacOS, or at
`%USERPROFILE%/.platformio/packages/tool-stm32duino/maple_upload.bat`
for Windows.

In order to use this to upload a .bin file you need to first determine which serial port the device is using.

For example, in MacOS, you can list all the cu.* devices to find out:

```
> ls /dev/cu.*                                                                                 
/dev/cu.Bluetooth-Incoming-Port
/dev/cu.debug-console
/dev/cu.usbmodem83101
...
```

And use a command like this one:

`~/.platformio/packages/tool-stm32duino/maple_upload <serial_port_name> 1 1EAF:0003 <file_to_upload.bin>`

For example, with MacOS port `cu.usbmodem83101` and a file named `STM32_MapleMiniDuino_firmware_STM32_MapleMiniDuino.bin`, you would execute the command:

`~/.platformio/packages/tool-stm32duino/maple_upload cu.usbmodem83101 1 1EAF:0003 STM32_MapleMiniDuino_firmware_STM32_MapleMiniDuino.bin`

## Arduino Tools
Instead of PlatformIO, you can use the Arduino tools from the STM32duino open source community, available to download here:

https://github.com/stm32duino/Arduino_Tools/tree/main

The process is very similar to the one explained before. After determining the serial port, you issue a command like this:

`..../STM32Tools/maple_upload.sh <serial_port_name> 1 1EAF 0003 <file_to_upload.bin>`
Take note that this version of the script uses two arguments `1EAF` `0003` instead of the single one used with PlatformIO (`1EAF:0003`).

Again, for the previous examples with MacOS port `cu.usbmodem83101` and a file named `STM32_MapleMiniDuino_firmware_STM32_MapleMiniDuino.bin`, the command would be:

`..../STM32Tools/maple_upload.sh cu.usbmodem83101 1 1EAF 0003 STM32_MapleMiniDuino_firmware_STM32_MapleMiniDuino.bin`

## GUI Tools
You can use the official STM32CubeProgrammer to send the binary firmware file to your device.

> [!WARNING]  
> Theoretically possible and yet unconfirmed.  Somebody needs to try this and confirm exactly how this works.
