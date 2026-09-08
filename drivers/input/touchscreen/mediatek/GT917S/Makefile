# WTK_GT917S_E977 -- Goodix `gt1x_*`, Doogee S88 Pro (MT6771).
#
# NEL KERNEL VA IN `drivers/input/touchscreen/mediatek/GT917S/`, che e' una
# directory NUOVA: la fabbrica accende `CONFIG_WTK_GT917S_E977` e tiene spente
# `CONFIG_TOUCHSCREEN_MTK_GT1151` e `CONFIG_TOUCHSCREEN_MTK_GT5688`
#   $ grep -nE "GT1151|GT5688|GT917S" /mnt/s88pro/kernel-work/e977_dg_m13_71_q0.config
#   2210:# CONFIG_TOUCHSCREEN_MTK_GT1151 is not set
#   2216:# CONFIG_TOUCHSCREEN_MTK_GT5688 is not set
#   2220:CONFIG_WTK_GT917S_E977=y
# quindi il codice di fabbrica NON e' quello delle due directory ALPS. Qui
# dentro c'e' solo il SOTTO-LOTTO che ALPS non puo' dare (vedi il cappello di
# gt1x_wtk.c): quattro funzioni su 87.
#
# `obj-y` e non `obj-$(CONFIG_WTK_GT917S_E977)` per la stessa ragione delle
# altre directory ricostruite: la voce Kconfig non esiste ancora nell'albero
# ALPS, e inventarne una qui senza agganciarla al Kconfig padre darebbe una
# directory che non si compila mai. L'aggancio -- Kconfig padre + una riga nel
# Makefile padre -- e' lavoro del lotto di merge.
#
# `ccflags-y` DEL MAKEFILE PADRE NON SI PROPAGA alle sottodirectory (solo
# `subdir-ccflags-y` lo fa) e questo file usa i tipi del padre: le -I vanno
# ripetute qui.
#
# IL `-Werror` QUI ARRIVA DAVVERO, e non e' quello del padre: e' questa riga.
# La differenza si vede nel comando che kbuild registra, confrontando il
# NOSTRO oggetto con quello di ALPS compilato nello stesso giro:
#   $ tr ' ' '\n' < out-gt917s/.../GT917S/.gt1x_wtk.o.cmd     | grep -cx -- -Werror
#   1
#   $ tr ' ' '\n' < out-gt917s/.../GT5688/.gt1x_generic.o.cmd | grep -cx -- -Werror
#   0
# e l'oggetto nostro compila senza una riga di error o warning. Quello di ALPS
# NO: con `-Werror` imposto a tutto l'albero (`KCFLAGS=-Werror`) si ferma su
#   gt1x_generic.c:489:13: error: unused function 'gt1x_sensor_id_check'
#       [-Werror,-Wunused-function]
# che e' il delta di header D6 del cappello di gt1x_wtk.c. Per questo la
# MISURA del blocco -- che compila anche `GT5688/` -- non usa `KCFLAGS`: il
# `-Werror` sta solo qui, dove il codice e' nostro.
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/GT917S/include
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/GT917S/include/config_1080p
# IL -Werror E' RISTRETTO AL FILE NOSTRO. Le cinque unita' qui accanto sono
# la copia di partenza presa da GT5688/ e non lo reggono ancora:
# gt1x_generic.c si ferma su
#   gt1x_generic.c:489:13: error: unused function 'gt1x_sensor_id_check'
# che NON e' rumore -- di fabbrica gt1x_init misura 1528 byte contro i 980
# di ALPS, e una funzione che qui nessuno chiama e' il primo posto dove
# guardare. Ogni unita' riprende -Werror quando il lotto che la corregge la
# porta a combaciare.
CFLAGS_gt1x_wtk.o += -Werror
# gt1x_extents.c ha ripreso il -Werror il 2026-08-22, quando l'unita' e'
# arrivata a 11 funzioni su 11 identiche al byte contro l'oracolo
# (blocco [0xffffff8008a7221c, 0xffffff8008a73854), 5556 byte).
CFLAGS_gt1x_extents.o += -Werror
# gt1x_tools.c ha ripreso il -Werror il 2026-08-22, quando l'unita' e'
# arrivata a 7 funzioni su 7 identiche al byte contro l'oracolo
# (blocco [0xffffff8008a761a4, 0xffffff8008a76ca8), 2820 byte) e a
# RESIDUO ZERO nel confronto per codifica su tutte e sette.
CFLAGS_gt1x_tools.o += -Werror
# gt1x_generic.c ha ripreso il -Werror il 2026-08-22, quando l'unita' e'
# arrivata a 20 funzioni su 21 identiche al byte contro l'oracolo
# (blocco [0xffffff8008a73854, 0xffffff8008a761a4), 10576 byte).  Per
# arrivarci e' stata TOLTA `gt1x_sensor_id_check`, che nel binario di
# fabbrica non ha nessun chiamante: la rimozione e' una scelta dichiarata,
# e il cappello della funzione la spiega.
CFLAGS_gt1x_generic.o += -Werror
# gt1x_update.c ha ripreso il -Werror il 2026-08-23, quando l'unita' e'
# arrivata a 24 funzioni su 24 identiche al byte contro l'oracolo (blocco
# [0xffffff8008a7839c, 0xffffff8008a7b600) meno dump_to_file, 12540 byte) e a
# RESIDUO ZERO nel confronto per codifica su tutte e ventiquattro.  Per
# arrivarci e' stata TOLTA `read_reg`, che nel binario di fabbrica non c'e'
# ne' separata ne' incorporata: la nota (13) in coda al file lo misura.
CFLAGS_gt1x_update.o += -Werror
# gt1x_tpd.c ha ripreso il -Werror il 2026-08-23, quando l'unita' e'
# arrivata a 19 funzioni su 19 identiche al byte contro l'oracolo (blocco
# [0xffffff8008a76cf4, 0xffffff8008a7839c), 5800 byte) e a RESIDUO ZERO nel
# confronto per codifica su tutte e diciannove.
#
# IL "RESIDUO ZERO" NON ERA VERO QUANDO E' STATO SCRITTO, e va detto.  Sette
# funzioni su diciannove avevano posizioni non classificate, 39 in tutto
# (gt1x_power_switch 5, gt1x_debug_proc 5, tpd_on 7, tpd_local_init 6,
# tpd_i2c_probe 4, tpd_registration 7, tpd_event_handler 5), TUTTE valori di
# __LINE__: il file non aveva nessuna direttiva #line e le note in testa
# spostano le righe rispetto alla fabbrica.  Era lo stesso debito che quella
# notte era stato chiuso su ILITEK/ltr2568/qmcx983 e che qui era rimasto
# aperto -- e la dimensione uguale non lo poteva denunciare.
#
# Oggi il residuo e' zero davvero: tools/derivaline.py ricava le #line dal
# formato che printk riceve, e gt1x_tpd.o misura 1143 istruzioni uguali su
# 1267 con ZERO posizioni da leggere a mano.  Per arrivarci sono state
# TOLTE sei funzioni globali (gt1x_is_tpd_halt, gt1x_auto_update_done,
# gt1x_pm_notifier, tpd_reregister_from_tui, tpd_enter_tui, tpd_exit_tui) e
# le cinque variabili che solo loro usavano: nessuna delle sei compare in
# stock.map dentro il blocco, e una funzione GLOBALE che esistesse di
# fabbrica ci sarebbe per forza.  Il cappello del file lo dichiara.
CFLAGS_gt1x_tpd.o += -Werror

# UNA sola unita' di traduzione, e NON e' una misura della partizione di
# fabbrica: le quattro funzioni che contiene, di fabbrica, stanno in file
# diversi. Per adiacenza nell'ordine della mappa (scout-gt917s/elenco.txt):
#   gt1x_gesture_debug  fra gesture_clear_wakeup_data e gt1x_init_node
#                       -> gt1x_extents.c, e i due vicini concordano;
#   dump_to_file        fra gt1x_error_erase e gt1x_hold_ss51_dsp_no_reset
#                       -> gt1x_update.c, e i due vicini concordano;
#   tpd_driver_exit     e' in .exit.text e la mappa non la contiene affatto;
#                       la sua unita' la da' il `tpd_driver_t` che passa,
#                       che di fabbrica sta con tpd_local_init/suspend/resume
#                       -> gt1x_tpd.c;
#   get_module_name     NON SI DECIDE per adiacenza, ed e' l'unica delle
#                       quattro: sta esattamente sulla cucitura, preceduta da
#                       `comfirm` (che in ALPS e' gt1x_tools.c) e seguita da
#                       `gt1x_i2c_read` (che in ALPS e' gt1x_tpd.c). O e'
#                       l'ultima dell'una o la prima dell'altra, e da qui non
#                       si sa quale. Dichiarato invece che scelto.
# Stanno insieme qui
# perche' questo file e' il residuo dichiarato di un lotto, non un'unita' di
# fabbrica ricostruita; e il rischio noto e' quello del cappello del ft8719
# (clang puo' incorporare fra loro funzioni che di fabbrica stanno in oggetti
# distinti). Qui non si e' presentato -- le quattro misurano tutte esattamente
# come di fabbrica -- ma il giorno in cui questa directory crescera' andra'
# ripartita per unita' vere.
obj-y += gt1x_wtk.o
obj-y += gt1x_extents.o
obj-y += gt1x_generic.o
obj-y += gt1x_tools.o
obj-y += gt1x_tpd.o
obj-y += gt1x_update.o

# ---------------------------------------------------------------------------
# GLI 86 SIMBOLI IRRISOLTI: COSA E' STATO MISURATO E COSA NO -- 2026-08-23
# ---------------------------------------------------------------------------
# Il commit 0678283 scriveva: "Sull'archivio dei sei oggetti: definiti 168,
# citati U 145, irrisolti 86 -- e ZERO di essi e' una funzione del blocco
# GT917S.  Gli 86 sono simboli del nucleo e del quadro tpd che il link del
# kernel risolve."
#
# LA PRIMA META' REGGE, LA SECONDA NO.  I tre numeri si riproducono, e
# l'intersezione fra gli 86 e gli 85 nomi del blocco e' davvero vuota: non
# manca niente che vada ancora ricostruito.  Ma CINQUE degli 86 nessun link di
# kernel li risolve, e sono questi:
#
#   fix_tp_proc_info    esiste nella fabbrica (stock.map, 0xffffff8008a5151c)
#                       e in nessun sorgente dell'albero ALPS.  La aggiunge
#                       patches/kernel-stock/mtk-tpd-wtk-tp-info.patch, che
#                       NON e' applicata all'albero.
#   g0fc0b8             nome derivato da un indirizzo, non esiste da nessuna
#                       parte -- e questo file lo dichiara gia' IRRISOLTO.
#   gt1x_tpd_driver     citata da gt1x_wtk.c:364 e :856, definita da nessuno.
#   gt1x_patch_jump_fw  citate senza guardia da gt1x_extents.c:670-683, e
#   hotknot_auth_fw     definite solo sotto CONFIG_GTP_REQUEST_FW_UPDATE, che
#                       nel .config in uso e' "is not set" -- e comunque solo
#                       dentro GT5688/ e GT1151/, che sono spente.
#
# La differenza fra insiemi dice "non manca niente da ricostruire".  Non dice
# "si linka": vedi la ritrattazione in HANDOFF.md.
