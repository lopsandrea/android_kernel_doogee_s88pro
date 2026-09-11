#
# Makefile for the Awinic AW22xxx case LEDs (Doogee S88 Pro).
#
# A subdirectory because that is how the factory has it: the .ddebug of the factory
# kernel (tools/ddebug.py, D5 of revisione-aw22127-completo.md) names the
# source file "drivers/misc/mediatek/leds/mt6771/aw22127/leds-aw22xxx.c" and
# the KBUILD_MODNAME "leds_aw22xxx". KBUILD_MODNAME depends only on the base
# name of the object file (scripts/Makefile.build: modname = $(basetarget)),
# not on the folder -- but the folder name is the factory one all the same,
# for the same reason that moved qmcX983.c into its own.
#
obj-y := leds-aw22xxx.o
