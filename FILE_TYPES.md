# MaxDuino Filetypes support

## ZX Spectrum

### .TAP

We support standard .TAP files with standard timings

### .TZX

We support most block types for standard .TZX files . Due to differences in interpretations of the .TZX spec, we support some additional flags to change polarity behaviours.

### .AY

.AY files containing z80 music player routines are supported.  We generate .TAP headers on-the-fly when loading a .AY file , so that these can simply be loaded like any other file.  These can then be loaded into an AY module player application running on the target device (your ZX Spectrum).

Support is enabled via configuration flag: `AYPLAY`.  This is enabled in most configurations by default, and there should be very little reason to disable it.

For more information about this file type, please see:
* https://worldofspectrum.net/projectay/tech.htm
* https://worldofspectrum.net/projectay/ayplayers.htm
* https://www.timf-tinkering.co.uk/spectrum/specay/

## ZX80

### .O

.O (O80 format) is supported. 

### .P

.P (P81 format) is supported for ZX80 (using 8K ROM conversions).

## ZX81

### .P

.P (P81 format) is supported for ZX81 files.

## ORIC-1

### .TAP

We support standard ORIC-1 .TAP files, and can playback at standard 300 BAUD or fast 2400 BAUD.

Support is enabled via configuration flag: `tapORIC`.  This is enabled in most configurations by default, and there should be very little reason to disable it.

Fast playback support is enabled via configuration flag: `ORICSPEEDUP`.  This is enabled everywhere by default.

## BBC MICRO / ACORN ATOM / ACORN ELECRON

### .UEF

We support many of the .UEF features, but not all are enabled by default (increases firmware size so you may wish to customize your build to choose which options you really need).
* `Use_c112` - enableds integer gap chunk for .uef
* `Use_hqUEF` - support `.hq.uef` files playback (see note)
* `Use_c104` - support defined tape format data block: data bits per packet/parity/stop bits    
* `Use_c114` - security cycles replaced with carrier tone
* `Use_c116` - floating point gap chunk for .hq.uef
* `Use_c117` - data encoding format change for 300 bauds
        
We also support a customized default baud rate for .UEF files.  The default is 1200, but you can customize it by setting one of the following configuration flags: `TURBOBAUD1500`, `TURBOBAUD1550`, `TURBOBAUD1600`

## JUPITER ACE

### .TAP

## MATTEL AQUARIUS

### .CAQ

We handle .CAQ encoded at 600 baud (playback is fixed to this baud rate, meaning that baud rate customizations are ignored for .CAQ files)

## MSX

### .CAS

We support .CAS files for MSX computers.  CAS support is enabled via configuration flag: `Use_CAS`.  Due to several recent optimisations, it should be possible to enable `Use_CAS` for all devices now without running out of firmware space, so all devices should be able to include CAS support by default now.

### .TSX

.TSX format is a variant of .TZX format with a specific additional block type.  We support .TSX files.

In addition, we have included support for enabling the user to choose their own baud rate, as an alternative to using the timing parameters from the .TSX file itself.  The custom baud rates we support here are 1200 , 2400, 3150, 3600, and 3850 .  To turn on the custom baud rate when playing a .TSX file, toggle the `TSXCzxpUEFSW` option in the menu to 'ON'. (Note that this option has a different meaning for certain other file types).  Turning this option to OFF uses the standard parameters from the .TSX file .

For more information about .TSX format, take a look at https://github.com/nataliapc/makeTSX/wiki

### .TZX

Because of how MaxDuino operates, MSX files are supported regardless of extension (we don't check that the extension is specifically .TSX and can handle MSX file saved as .TZX too)

## DRAGON 32/64

### .CAS

We support .CAS files for DRAGON computers.  Support is enabled via configuration flag: `Use_DRAGON` **in addition to** `Use_CAS`.  Due to several recent optimisations, it should be possible to enable `Use_CAS` and `Use_DRAGON` for all devices now without running out of firmware space, so all devices should be able to include CAS support by default now.

There are several additional customizations for .CAS support for DRAGON computers:

* `Use_Dragon_sLeader` - a short Leader of 0x55 allowed for loading TOSEC files
* `Expand_All` - expand short Leaders in ALL file header blocks. 

## TANDY COLOR COMPUTER (COCO)

### .CAS

## AMSTRAD CPC

### .CDT

.CDT images are essentially the same as .TZX images, for Amstrad CPC computers.
.CDT support is enabled via configuration flag: `ID11CDTspeedup` .  Without this flag enabled, files with a .cdt extension will not be recognised.  This is enabled in most configurations by default, and there should be very little reason to disable it.
In addition, we have included support for enabling the user to choose their own baud rate, as an alternative to using the timing parameters  from the .CDT file itself.  The custom baud rates we support here are 1000 (same as ROM default), 2000, 3500, and 4000 .  The menu options currently don't match exactly, so you will choose `1200`, `2400`, `3150` (or `3600`), and `3850`, respectively.  To turn on the custom baud rate when playing a .CDT file, toggle the `TSXCzxpUEFSW` option in the menu to 'ON'. (Note that this option has a different meaning for certain other file types).  Turning this option to OFF uses the standard parameters from the .CDT file .

## SHARP MZ (MZ-700, MZ-800)

### .MZF

We support .MZF files for these computers.
