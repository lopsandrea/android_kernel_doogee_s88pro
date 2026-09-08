#
# Makefile per i LED della scocca Awinic AW22xxx (Doogee S88 Pro).
#
# Sottodirectory perche' cosi' sta di fabbrica: il .ddebug del kernel di
# fabbrica (tools/ddebug.py, D5 di revisione-aw22127-completo.md) nomina il
# file sorgente "drivers/misc/mediatek/leds/mt6771/aw22127/leds-aw22xxx.c" e
# il KBUILD_MODNAME "leds_aw22xxx". KBUILD_MODNAME dipende solo dal nome
# base del file oggetto (scripts/Makefile.build: modname = $(basetarget)),
# non dalla cartella -- ma il nome della cartella e' comunque quello di
# fabbrica, per la stessa ragione che ha spostato qmcX983.c nella sua.
#
obj-y := leds-aw22xxx.o
