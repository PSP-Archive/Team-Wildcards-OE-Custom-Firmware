Preliminary media player that loads any OGG file or mikmod-compatible module via args passed to the program.

To build it as an IRshell plugin:
	make clean && make plugin

then copy the new EBOOT.PBP to ms0:/IRSHELL/EXTAPP15/APP(#)/EBOOT.PBP
and assign the extensions (OGG, MOD, IT, S3M, XM, etc.) to APP(#) in the Configurator (hold R and press Start in IRshell).

The EBOOT.PBP file here is the IRshell plugin.
