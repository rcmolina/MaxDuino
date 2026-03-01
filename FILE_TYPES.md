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

### .P

### .O

## ZX81

### .P

### .O

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

## DRAGON 32

### .CAS

## TANDY COLOR COMPUTER (COCO)

### .CAS

## AMSTRAD CPC

### .CDT
