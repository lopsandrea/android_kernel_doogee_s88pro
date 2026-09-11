# BEWARE: `ccflags-y` from the parent Makefile does not propagate into
# subdirectories; `subdir-ccflags-y` does, but ONLY when kbuild descends from the
# parent Makefile. Building this directory alone
# (`make .../charger/mt5725/`) never reads the parent, and its -I flags do not
# arrive. They are repeated below, so the file compiles with the same command
# lines in both contexts — which is the condition for the MEASUREMENT made on
# the directory alone to hold in tree as well.
MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
ccflags-y += -Werror
ccflags-y += -I$(srctree)/drivers/power/supply/mediatek/charger
ccflags-y += -I$(srctree)/drivers/power/supply/mediatek/misc/
ccflags-y += -I$(srctree)/drivers/power/supply/mediatek/battery/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/pmic/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/typec/tcpc/inc
ccflags-$(CONFIG_MTK_PMIC_CHIP_MT6356) += -I$(srctree)/drivers/misc/mediatek/pmic/include/mt6356
ccflags-$(CONFIG_USB_MTK_HDRC) += -I$(srctree)/drivers/misc/mediatek/usb20/

obj-y := mt5725.o
