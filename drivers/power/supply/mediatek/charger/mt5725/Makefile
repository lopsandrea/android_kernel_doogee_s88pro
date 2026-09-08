# ATTENZIONE: `ccflags-y` del Makefile padre non si propaga alle
# sottodirectory; `subdir-ccflags-y` si', ma SOLO quando kbuild scende dal
# Makefile padre. Compilando questa sola directory
# (`make .../charger/mt5725/`) il padre non viene letto, e le sue -I non
# arrivano. Qui sotto sono ripetute, cosi' il file si compila con le stesse
# righe di comando in tutti e due i contesti — che e' la condizione perche'
# la MISURA fatta sulla directory sola valga anche in albero.
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
