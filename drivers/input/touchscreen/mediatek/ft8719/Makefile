# WTK_FT8719_E977 -- FocalTech FT8719, Doogee S88 Pro (MT6771).
#
# `ccflags-y` from the parent Makefile does NOT propagate into subdirectories
# (only `subdir-ccflags-y` does), and this file includes the parent's "tpd.h":
# the -I flags have to be repeated here.
#
# The parent's `-Werror` does NOT reach this directory, for two independent
# reasons, and the file does stand it when imposed with KCFLAGS=-Werror.
#
# The working notes -- the measurements against the factory binary, the
# batch-by-batch record and the reasoning behind each choice -- are in
# docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_Makefile.md
# in the oracolo repository. They are kept in Italian, as the project's
# internal record.
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include

# ONE OBJECT FILE PER FACTORY TRANSLATION UNIT. It is not a matter of style:
# putting two units in the same .c would let clang inline into one another
# functions that in the factory build live in distinct object files (there is no
# LTO), and would change the size of functions already measured. The partition
# into six units is measured -- text, .data and .bss all give the same one.
#
#   focaltech_core.c   unit C  0xffffff8008a7eac0..0xa8020c + .init + .exit
#   focaltech_i2c.c    unit E  0xffffff8008a802ac..0xa807e0
#   focaltech_ex_mode.c unit D 0xffffff8008a8020c..0xa802ac
#   focaltech_ft8719_upgrade.c unit F 0xffffff8008a807e0..0xa80b54
#   focaltech_sysfs.c  unit A  0xffffff8008a7b600..0xa7cc4c  (22 fn out of 22)
#   focaltech_flash.c  unit B  0xffffff8008a7cc4c..0xa7eac0  (17 fn out of 17)
#
# THE DRIVER IS COMPLETE: 67 functions out of 67, and no FT8719 function is
# left unresolved in built-in.o.
#
# The working notes -- the measurements against the factory binary, the
# batch-by-batch record and the reasoning behind each choice -- are in
# docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_Makefile.md
# in the oracolo repository. They are kept in Italian, as the project's
# internal record.
obj-y := focaltech_core.o
obj-y += focaltech_i2c.o
obj-y += focaltech_ex_mode.o
obj-y += focaltech_ft8719_upgrade.o
obj-y += focaltech_sysfs.o
obj-y += focaltech_flash.o
