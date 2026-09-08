# WTK_ILITEK_E977 -- touchscreen ILITEK, Doogee S88 Pro (MT6771).
#
# NEL KERNEL VA IN `drivers/input/touchscreen/mediatek/ilitek_e977/`, ed e' una
# directory NUOVA: la fabbrica accende `CONFIG_WTK_ILITEK_E977` e l'albero ALPS
# quel simbolo non ce l'ha.  L'aggancio -- `Kconfig` qui accanto, piu' una riga
# `source` e una `obj-$(CONFIG_WTK_ILITEK_E977) += ilitek_e977/` nel padre -- e'
# stato fatto il 2026-08-23.
#
# QUESTO FILE E' RIMASTO PER GIORNI SOLO SULLA MACCHINA DI BUILD, e va detto:
# il repo aveva i sorgenti e non la ricetta per compilarli.  E' lo stesso
# difetto che il progetto ha gia' incontrato col `pytest.ini` -- una cosa che
# esiste solo dove gira non e' una cosa che il repository possiede.
#
# `ccflags-y` del padre NON si propaga alle sottodirectory (solo
# `subdir-ccflags-y` lo fa), e questi file includono "tpd.h" del padre: le -I
# vanno ripetute qui.
#
# IL `-Werror` ARRIVA, ed e' quello del padre (`subdir-ccflags-y += -Werror` in
# drivers/input/touchscreen/mediatek/Makefile).  Arriva pero' SOLO quando
# kbuild discende dal padre, cioe' da quando c'e' la riga `obj-$(CONFIG_...)`:
# col comando `make ... <dir>/` che questo progetto usa per misurare, la
# variabile KBUILD_SUBDIR_CCFLAGS non viene mai esportata e il -Werror non c'e'.
# Il Makefile di GT917S documenta il meccanismo per esteso.

ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include

obj-y := ilitek_main.o ilitek_bus.o ilitek_plat.o ilitek_ic.o ilitek_touch.o ilitek_fw.o ilitek_node.o ilitek_mp.o
