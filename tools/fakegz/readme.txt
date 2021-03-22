make install
will put the binary in your psp dev bin path. Not sure if endianess will survive properly on certain machines.

Similarto bin2c and based on raw2c, can be used from a makefile (extra target) or separately on command line to
create reboot.h/rebootex.h files in DA's style with the filename and size
included in the first 16bytes of the array.

0.1 - option "-a" added to add "__attribute__((aligned(16)))" to the declaration
