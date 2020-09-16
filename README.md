# maxduino

https://www.va-de-retro.com/foros/viewtopic.php?t=5541&start=9999

* V1.59 Oric native tap support.
* V1.58 STM32 testing support thanks to @Sildenafile.
* V1.57 Changed zx81 TSXControl to Speed to avoid polarity problems. Better zx81 file loading (@El Viejoven FX). Removed Amstrad tweaking to slower ID15, 
so Oricium game now can load in 4 minutes in Oric Atmos, file attached for testing.
* V1.56 aka "baby Fran". ID19 short implementation to load zx81 tzx files. Now you can use TSXControl to speedup loadings.
* V1.55 support for BBC chunk 0x117 (@vanekp)
* V1.54 Maintenance release. Final byte patched again for Dragon and Coco, got switched when testing in v1.51.
* V1.53 Some rework for Arduino nano every and timers TCA / TCB0 and using last SDFat for better speed.
* V1.52 New arduino nano EVERY micro supported. Need work for turbo loading. Compressed logo where only even columns are used for load/save with EEPROM. Different config files for new high capacity chips (Riccardo Repetto).
* V1.51 Dragon TOSEC files with short leader loading now.
* V1.50 aka "Maria". File auto-scrolling when holding down UP or DOWN buttons (Frank Schröder).
* V1.49 ID15 adjustment for slow Amstrad musical loaders.
* V1.48 New chunks for Acorn computers: Implemented parity handling when loading protected games (@acf76es). Remove warnings when compilation from IDE (@llopis).
* V1.47 Optional BLOCKID_INTO_MEM if loading many turbo short blocks. BLOCK_EEPROM_PUT must be disabled when loading Amstrad cpc Breaking Baud demo. Bug fixed: block counter should not be incremented when pausing. 
* V1.46 OLED 128x64 mode with 8 rows, logo can also be loaded with full resolution (@geloalex, @merlinkv, @acf76es)
* V1.45 New ID15 testing code, can be deactivated from userconfig.h to save space. Amstrad bugs solved: Deflektor and other cdts now loading.
* V1.44 New define btnRoot_AS_PIVOT in userconfig.h so it can be deactivated by user. New #define in Maxduino.h to support Antonio Villena's MINIDUINO new design with amplifier and new define OLED_SETCONTRAS in userconfig.h for contrast value override. New Miniduino logo. It's posible to select RECORD_EEPROM and LOAD_EEPROM both for better testing when selecting new logos in userconfig.h, pressing MENU simulates a reset to show logo again.
* V1.43 aka "Mar Menor". Implemented half-interval (logarithmic) search for dirs and block selection. Now ROOT button used as pivot.
Menu functions now activated with ROOT + STOP, new half-interval search with ROOT + UP/DOWN.
* V1.42 Bugs solved: Rewinding block not showing first digit correctly (@acf76es). LCD printing when block ID not recognized again in hex.
First support of 1,3" oled SH1106 but using 128x32 and not 128x64. Backported optimizations from V1.43.
* V1.41 New logos from zx-uno companion Sugarless (@spark2k06/@yoxxxoy). New SPLASH_SCREEN configuration (@spark2k06).  Better .tsx start/stopbits support: Now working with SVI-3x8 machines!
* V1.40 LCD sketch savings, tsx control/polarity/UEF turbo mode can be changed while paused with menu button. Reworked acorn chunk 116 floating point conversion to integer.
* V1.39 Optional hq.uef support for Acorn using define (implemented chunks 111 and 116). New userconfig.h with help for easier configuration.
* V1.38	Code reformatted, cas is optional with new #def Use_CAS. Solved a bug when back with stop button from 4th subdir.
* V1.37 Minor optimizations for counter format, acorn standard/turbo speed, simplication in polarity handling and now controlled from menu using the same option for tsx acceleration: tsxSPEEDzxPOL.
* V1.36 Polarity handling with conditional define.
* V1.35t Acorn speed for loading now 1500 baud, 25% faster (standard is 1200).
* V1.35 Uncompressed UEF support for Acorn computers.
* V1.34 alias "EEPROM version". Now blocks can be directly read/written from EEPROM.Also logo can be copied to eeprom using #define option.
	After that, you can select option for loading logo directly from EEPROM.. this saves 512 bytes in the sketch.
* V1.33 Blocks for manual REW/FF expanded from 10 upto 20, used as circular buffer. On Oled screen prints upto 99 blocks, 
	overflowing to 00. Selecting a block in pause mode traverse last 20 blocks.
* V1.32 Deprecated old pskipPause. New block2A pause control managed in menu.
* V1.31 Modified ID20,2A to support automatic pausing(@spirax). Also swapped REW-FW for block navigation(@acf76es). 
* V1.30 max TSX speed 3850 vs cas speed 3675. Also changed in Menu.
* V1.29 3-digits counter can be configured to display m:ss with #define CNTRBASE
* V1.28 Better speeed loading tzx files, more than 5000 baud using PORT instructions

### In memoriam

To my father R.I.P. who bought my first ZX spectrum and forced me to learn with his casio programming calc.