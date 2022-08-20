# IBM5150

JohnElliott: https://forum.vcfed.org/index.php?threads/ibm-pc-cassette-interface.8765/

I've done a bit of research into how Cassette BASIC on the 5150 stores its files, and written up what I've found at http://fileformats.archiveteam.org/wiki/IBM_PC_data_cassette .

What I also discovered is that the timings are close enough to those used by the ZX Spectrum that the .TZX archiving format designed for Spectrum tapes can also be used to store 5150 BASIC programs. This is obviously far more convenient than .WAV; a 9k BASIC program occupies 1293k in WAV format, but 9k in TZX format.

If you've got a cassette-equipped 5150 or PCjr to hand, you can find a sample .TZX here . PlayTZX can be used to play it through your soundcard.

Creating the .TZX isn't quite as straightforward; I had to add a trailer block by hand to avoid load errors. The process I used (having captured the output from the 5150's MIC connector as a .WAV) was:

sox haunt.wav -b 8 haunt.voc
voc2tzx haunt.voc haunt.tzx
Then create the 20-byte trailer block in a hex editor:

11 9E 07 C1  46 6E A8 4F  00 9E 00 8B  0C 01 00 00
01 00 00 00
Append it to the file:

copy /b haunt.tzx+trailer.bin hauntx.tzx
And check that it will load on the 5150:

LOAD "CAS1:" (on the 5150)
playtzx hauntx.tzx (on the PC)

==================

mkibmtap -n orient.tzx -a orient.bas
playtzx orient.tzx
or turn it into a .WAV with

mkibmtap -fpzxi -n orient.pzx -a orient.bas
pzx2wav orient.pzx > orient.wav

https://www.atariarchives.org/bca/Chapter04_TheOrientExpress.php