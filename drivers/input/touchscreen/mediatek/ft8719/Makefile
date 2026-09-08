# WTK_FT8719_E977 -- FocalTech FT8719, Doogee S88 Pro (MT6771).
#
# `ccflags-y` del Makefile padre NON si propaga alle sottodirectory (solo
# `subdir-ccflags-y` lo fa), e questo file include "tpd.h" del padre: le -I
# vanno ripetute qui.
#
# IL `-Werror` DEL PADRE, CORRETTO il 2026-08-21 (revisione indipendente).
# Qui c'era scritto che il `-Werror` del padre «ARRIVA, perche' e' dichiarato
# come subdir-ccflags-y in drivers/input/touchscreen/mediatek/Makefile».  La
# riga nel padre c'e' davvero:
#   $ grep -n 'Werror' drivers/input/touchscreen/mediatek/Makefile
#   1:subdir-ccflags-y += -Werror
# ma NON arriva, per due ragioni indipendenti.
#
# (1) `subdir-ccflags-y` viaggia via KBUILD_SUBDIR_CCFLAGS, che scripts/
#     Makefile.lib ESPORTA mentre legge il Makefile del padre:
#       scripts/Makefile.lib:11:export KBUILD_SUBDIR_CCFLAGS := $(KBUILD_SUBDIR_CCFLAGS) $(subdir-ccflags-y)
#       scripts/Makefile.lib:105:orig_c_flags   = $(KBUILD_CPPFLAGS) $(KBUILD_CFLAGS) $(KBUILD_SUBDIR_CCFLAGS) \
#     Il meccanismo funziona solo se kbuild DISCENDE dal padre.  Il comando
#     con cui questo lotto compila -- `make ... <dir>/` -- fa saltare
#     direttamente a Makefile.build sulla sottodirectory, e il Makefile del
#     padre non viene mai letto.  Il comando registrato dal kernel lo prova:
#       $ grep -o '\-Werror[^ ]*' out-ft8719/.../.focaltech_core.o.cmd | sort | uniq -c
#             1 -Werror-implicit-function-declaration
#             1 -Werror=date-time
#             1 -Werror=implicit-int
#             1 -Werror=incompatible-pointer-types
#             1 -Werror=strict-prototypes
#             1 -Werror=unknown-warning-option
#     Sono i `-Werror=<specifico>` di serie del kernel; nessun `-Werror` nudo.
#
# (2) Anche una build dell'albero intero non ci passerebbe: il Makefile del
#     padre NON ha ancora la riga per questa directory (e' il delta di header
#     n.3 del cappello di focaltech_core.c).  Le sue uniche righe focaltech
#     sono focaltech_touch/ e focaltech_fhd_touch/.
#
# CHE IL FILE REGGA `-Werror` E' VERO, ma va misurato a parte, imponendolo:
#   $ make ... KCFLAGS=-Werror drivers/input/touchscreen/mediatek/focaltech_ft8719/
#     CC      drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_core.o
#     AR      drivers/input/touchscreen/mediatek/focaltech_ft8719/built-in.o
# (zero righe error/warning).  E' questa la condizione in cui si trovera' il
# lotto di merge quando aggancera' la directory al Makefile padre.
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include

# UN FILE OGGETTO PER OGNI UNITA' DI TRADUZIONE DELLA FABBRICA.  Non e' una
# preferenza di stile: mettere due unita' nello stesso .c darebbe a clang la
# facolta' di incorporare fra loro funzioni che di fabbrica stanno in file
# oggetto distinti (nessun LTO), e cambierebbe la dimensione di funzioni gia'
# misurate.  La partizione in sei unita' e' misurata -- testo, .data e .bss
# danno la stessa -- ed e' la tabella §5 di scout-ft8719/RIASSUNTO.md.
#
#   focaltech_core.c   unita' C  0xffffff8008a7eac0..0xa8020c + .init + .exit
#   focaltech_i2c.c    unita' E  0xffffff8008a802ac..0xa807e0
#   focaltech_ex_mode.c unita' D 0xffffff8008a8020c..0xa802ac
#   focaltech_ft8719_upgrade.c unita' F 0xffffff8008a807e0..0xa80b54
#   focaltech_sysfs.c  unita' A  0xffffff8008a7b600..0xa7cc4c  (22 fn su 22)
#   focaltech_flash.c  unita' B  0xffffff8008a7cc4c..0xa7eac0  (17 fn su 17)
#
# L'UNITA' B E' IL SESTO OGGETTO, aggiunto dal lotto B1 il 2026-08-22.  IL SUO
# NOME E' UNA SCELTA E NON UNA MISURA: il binario non contiene i nomi dei file
# sorgente di FocalTech (nessun descrittore `_ddebug` -- `ddebug.py
# oracolo/stock.elf --file focaltech` da' "dopo il filtro: 0" -- e nessuna
# stringa di percorso `.c`).  La scelta e' argomentata nel §0 del cappello di
# focaltech_flash.c.  I lotti B2 e B3 scrivono in QUEL file, non in uno nuovo:
# due unita' nello stesso `.c` cambierebbero le dimensioni gia' misurate,
# una unita' spezzata in due `.c` pure.
#
# IL LOTTO B2 (2026-08-22) ha aggiunto le otto funzioni fra 0xffffff8008a7cc4c
# e 0xffffff8008a7dbc0: l'unita' B e' passata da sei a QUATTORDICI funzioni su
# diciassette, e tutte e quattordici misurano la dimensione ESATTA di fabbrica
# col compilatore di fabbrica, con residuo di codifica zero.
#
# IL LOTTO B3 (2026-08-22) ha aggiunto le ultime TRE (fts_read_file,
# fts_upgrade_bin, fts_fwupg_upgrade): L'UNITA' B E' COMPLETA, diciassette
# funzioni su diciassette, tutte e diciassette della dimensione ESATTA di
# fabbrica col compilatore di fabbrica e con residuo di codifica zero.  Che
# sia chiusa e non solo completa e' misurato: l'oggetto misura 7796 byte,
# esattamente 0xffffff8008a7eac0 - 0xffffff8008a7cc4c.
#
# IL LOTTO A4 (2026-08-22) ha aggiunto le ultime QUATTRO dell'unita' A
# (fts_debug_read, fts_debug_write, fts_tprwreg_show, fts_tprwreg_store, 3508
# byte) piu' i sei simboli .bss che le ultime due si scambiano: L'UNITA' A E'
# COMPLETA, ventidue funzioni su ventidue, tutte della dimensione ESATTA di
# fabbrica col compilatore di fabbrica.  Anche l'unita' A e' CHIUSA e non solo
# completa: la somma delle ventidue fa 5708 byte, esattamente
# 0xffffff8008a7cc4c - 0xffffff8008a7b600.
#
# CON QUESTO IL DRIVER E' COMPLETO: 67 funzioni su 67, e in built-in.o NON
# RESTA IRRISOLTA NESSUNA funzione di FT8719 (erano quattro).  I 68 simboli
# che restano irrisolti sono tutti del kernel o della piattaforma (printk,
# kfree, snprintf, tpd, ...) e si risolvono al link finale.
#
# IL `-Werror` E IL COMPILATORE DI FABBRICA, CORRETTO DAL LOTTO A4.  Fino al
# 2026-08-22 la directory NON si compilava affatto col compilatore di fabbrica
# -- non per un errore di codice, ma perche' focaltech_sysfs.c spegneva
# `-Wfortify-source` con un `#pragma clang diagnostic ignored`, e quel gruppo
# di avvisi in clang 9.0.3 NON ESISTE:
#   error: unknown warning group '-Wfortify-source', ignored
#          [-Werror,-Wunknown-warning-option]
# (il kernel porta `-Werror=unknown-warning-option` di serie, quindi era un
# errore anche senza KCFLAGS=-Werror).  Adesso il pragma spegne PRIMA
# `-Wunknown-warning-option`, e la directory intera si compila pulita con
# TUTTI E DUE i compilatori e con KCFLAGS=-Werror imposto.
obj-y := focaltech_core.o
obj-y += focaltech_i2c.o
obj-y += focaltech_ex_mode.o
obj-y += focaltech_ft8719_upgrade.o
obj-y += focaltech_sysfs.o
obj-y += focaltech_flash.o
