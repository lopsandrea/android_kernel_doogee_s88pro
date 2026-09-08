# Accelerometro MiraMEMS mir3da (DA218-B) del Doogee S88 Pro.
#
# Il nome della directory e' quello di fabbrica, letto dal __FILE__ che
# l'ASSERT di mir3da_reg_data_store lascia nel binario:
#   .../drivers/misc/mediatek/sensors-1.0/accelerometer/da218-B/mir3da_cust.c
#
# DUE unita' di traduzione, come di fabbrica: mir3da_cust.c (37 funzioni su
# 37, scritte) e il core (19 delle 29 del blocco).  Il nome "mir3da_core.c"
# e' la convenzione del driver MiraMEMS a monte, NON una misura: la stringa
# "mir3da_core" non esiste in stock.elf (revisione indipendente,
# docs/bringup/rapporti/revisione-mir3da.md, rilievo R6b).
#
# Che siano due oggetti separati e non uno solo lo prova la MODULE_VERSION
# built-in di mir3da_cust.c, il cui campo .module_name vale "mir3da_cust":
# KBUILD_MODNAME e' per file oggetto.
#
# Dal 2026-08-16 il driver e' INNESTATO: il Kconfig e il Makefile padre di
# accelerometer/ lo raggiungono, CONFIG_WTK_DA218B=y (che il config di
# fabbrica ha alla riga 1557 di e977_dg_m13_71_q0.config) non viene piu'
# scartato da olddefconfig, e il link riesce.  Le 10 funzioni del core che
# restano fuori sono globali che nessun altro oggetto chiama: non compaiono
# in nessun errore del linker.  L'elenco e la ragione sono nel cappello di
# mir3da_core.c, §1.

ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/accelerometer/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/hwmon/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/include

obj-y := mir3da_cust.o
obj-y += mir3da_core.o
