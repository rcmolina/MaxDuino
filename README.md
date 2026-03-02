# MaxDuino

This project contains the MaxDuino firmware, compatible with most MaxDuino and TZXDuino devices.  These devices are 'tape emulators' that output audio for loading into retro computers typically manufactured in the 1980s and 1990s.
By downloading and installing the latest MaxDuino firmware onto your MaxDuino or TZXDuino devices, you can add support for the largest range of filetypes for different retro computers as well as adding new usability and configuration options for your device.
For more information about MaxDuino and TZXDuino, check out the [DOCUMENTATION](./Docs.md)

## LATEST NEWS

* V3.02 Adds Sharp MZ-700 .MZF and Mattel Aquarius .CAQ implementation.
* V3.01 Adds Native Jupiter tap (JTAP) implementation.
* V3.00 Adds 44.1kHz OTLA support and we now use "Maxduino OTLA" as reference name for this version. Check https://github.com/rcmolina/otla_tzx for some testing.

## FILETYPES SUPPORTED BY MAXDUINO

* ZX SPECTRUM: .TZX, .TAP, .AY
* ZX80: .P, .O
* ZX81: .P, .O
* BBC MICRO: .UEF
* JUPITER ACE: .TAP (JTAP)
* MATTEL AQUARIUS: .CAQ
* MSX: .TSZ, .CAS
* DRAGON 32: .CAS
* AMSTRAD CPC: .CDT
* SHARP MZ-700: .MZF

More information at [FILE TYPES](./FILE_TYPES.md)

## SUPPORTED MAXDUINO/TZXDUINO DEVICES

Various MaxDuino and TZXDuino devices have been manufactured over the last 15 years or so, and these have been constructed with different combinations of hardware components (microcontroller type, LCD type, etc).  We provide MaxDuino software intended to be compatible with all of them and more.

*  Arduino Nano 328p
*  Arduino Nano Every
*  Thinary Nano Every
*  Arduino Mega 2560
*  Seeeduino Seeed Xiao M0 (SAMD21) (experimental)
*  Espressif ESP8266 (experimental)
*  Espressif ESP32C3 (experimental)
*  STMicroelectonics STM32 (experimental, needs testing)
*  .. and others

The above are included in GitHub releases: https://github.com/rcmolina/MaxDuino/releases

More information at [MAXDUINO DEVICES](./MAXDUINO_DEVICES.md)

## BUILD INSTRUCTIONS

You can use PlatformIO along with the included environments in platformio.ini or create your own.

Or you can use Arduino IDE (via the MaxDuino/MaxDuino.ino file)

More information at [BUILDING](./BUILDING.md)

## INSTALLING PREBUILT RELEASES

You can download prebuilt releases from the github Releases page and upload them to your device
using the appropriate tools for your device (usually that's avrdude).  For example, on Arduino Nano you can
follow these instructions: https://forum.arduino.cc/t/using-avrdude-to-push-sketch-to-arduino/525745

The master branch will always have a release called "latest" which is automatically rebuilt on every commit.
We also aim to produce release packages with tagged versions whenever there is a major or minor version bump.

More information at [INSTALLATION](./INSTALLATION.md)

## PREVIOUS VERSIONS

More information at [HISTORY](./HISTORY.md)

## DOCS

See [DOCS](./DOCS.md)

## In memoriam

To my father R.I.P. who bought my first ZX Spectrum and forced me to learn with his casio programming calc.