# WTK_ILITEK_E977 -- ILITEK touchscreen, Doogee S88 Pro (MT6771).
#
# It goes into `drivers/input/touchscreen/mediatek/ilitek_e977/`, and it is a NEW
# directory: the factory turns on `CONFIG_WTK_ILITEK_E977` and the ALPS tree does
# not have that symbol. Hooking it up -- the `Kconfig` next door, plus a `source`
# line and an `obj-$(CONFIG_WTK_ILITEK_E977) += ilitek_e977/` in the parent -- was
# done on 2026-08-23.
#
# THIS FILE LIVED FOR DAYS ONLY ON THE BUILD MACHINE, and that has to be said:
# the repo had the sources and not the recipe to compile them. It is the same
# defect the project already met with `pytest.ini` -- something that
# exists only where it runs is not something the repository owns.
#
# `ccflags-y` from the parent does NOT propagate into subdirectories (only
# `subdir-ccflags-y` does), and these files include the parent's "tpd.h": the -I
# flags have to be repeated here.
#
# THE `-Werror` DOES ARRIVE, and it is the parent's (`subdir-ccflags-y += -Werror` in
# drivers/input/touchscreen/mediatek/Makefile). It arrives ONLY when kbuild
# descends from the parent, that is, since the `obj-$(CONFIG_...)` line exists:
# with the `make ... <dir>/` command this project uses to measure, the
# KBUILD_SUBDIR_CCFLAGS variable is never exported and the -Werror is absent.
# GT917S's Makefile documents the mechanism in full.

ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include

obj-y := ilitek_main.o ilitek_bus.o ilitek_plat.o ilitek_ic.o ilitek_touch.o ilitek_fw.o ilitek_node.o ilitek_mp.o
