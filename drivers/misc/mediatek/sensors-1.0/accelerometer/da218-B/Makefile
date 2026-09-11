# MiraMEMS mir3da (DA218-B) accelerometer, Doogee S88 Pro.
#
# The directory name is the factory one, read from the __FILE__ that the
# ASSERT in mir3da_reg_data_store leaves in the binary:
#   .../drivers/misc/mediatek/sensors-1.0/accelerometer/da218-B/mir3da_cust.c
#
# TWO translation units, as in the factory build: mir3da_cust.c (37 functions out
# of 37, written) and the core (19 of the block's 29). The name "mir3da_core.c"
# is the convention of the upstream MiraMEMS driver, NOT a measurement: the string
# "mir3da_core" does not exist in stock.elf (independent review,
# docs/bringup/rapporti/revisione-mir3da.md, finding R6b).
#
# That they are two separate objects and not one is proven by the built-in
# MODULE_VERSION of mir3da_cust.c, whose .module_name field is "mir3da_cust":
# KBUILD_MODNAME is per object file.
#
# Since 2026-08-16 the driver is HOOKED UP: the Kconfig and the parent Makefile of
# accelerometer/ reach it, CONFIG_WTK_DA218B=y (which the factory config has at
# line 1557 of e977_dg_m13_71_q0.config) is no longer discarded by olddefconfig,
# and the link succeeds. The 10 core functions left out are globals no other
# object calls: they appear in no linker error. The list and the reason are in
# the header of mir3da_core.c, §1.

ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/accelerometer/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/hwmon/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/include

obj-y := mir3da_cust.o
obj-y += mir3da_core.o
