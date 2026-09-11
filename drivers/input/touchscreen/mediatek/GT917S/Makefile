# WTK_GT917S_E977 -- Goodix `gt1x_*`, Doogee S88 Pro (MT6771).
#
# It goes into `drivers/input/touchscreen/mediatek/GT917S/`, a NEW directory:
# the factory turns on `CONFIG_WTK_GT917S_E977` and leaves
# `CONFIG_TOUCHSCREEN_MTK_GT1151` and `CONFIG_TOUCHSCREEN_MTK_GT5688` off, so
# the factory code is not that of the two ALPS directories. What lives here is
# only the declared sub-batch ALPS cannot give: four functions out of 87.
#
# `obj-y` and not `obj-$(CONFIG_WTK_GT917S_E977)` because the Kconfig entry does
# not exist in the ALPS tree yet; hooking it up -- parent Kconfig plus a line in
# the parent Makefile -- is the merge batch's job.
#
# `ccflags-y` from the parent Makefile does NOT propagate into subdirectories
# (only `subdir-ccflags-y` does), so the -I flags are repeated here.
#
# The working notes -- the measurements against the factory binary, the
# batch-by-batch record and the reasoning behind each choice -- are in
# docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_Makefile.md
# in the oracolo repository. They are kept in Italian, as the project's
# internal record.
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/GT917S/include
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/GT917S/include/config_1080p
# THE -Werror IS RESTRICTED TO OUR OWN FILE. The five units next door are
# the starting copy taken from GT5688/ and do not stand it yet:
# gt1x_generic.c stops on
#   gt1x_generic.c:489:13: error: unused function 'gt1x_sensor_id_check'
# which is NOT noise -- in the factory build gt1x_init measures 1528 bytes against
# ALPS's 980, and a function nobody calls here is the first place to look.
# Each unit takes -Werror back when the batch that corrects it brings it into
# agreement.
CFLAGS_gt1x_wtk.o += -Werror
# gt1x_extents.c took -Werror back on 2026-08-22, when the unit reached
# 11 functions out of 11 byte-identical against the oracle
# (block [0xffffff8008a7221c, 0xffffff8008a73854), 5556 bytes).
CFLAGS_gt1x_extents.o += -Werror
# gt1x_tools.c took -Werror back on 2026-08-22, when the unit reached
# 7 functions out of 7 byte-identical against the oracle
# (block [0xffffff8008a761a4, 0xffffff8008a76ca8), 2820 bytes) and
# ZERO RESIDUE in the encoding comparison on all seven.
CFLAGS_gt1x_tools.o += -Werror
# gt1x_generic.c took -Werror back on 2026-08-22, when the unit reached
# 20 functions out of 21 byte-identical against the oracle
# (block [0xffffff8008a73854, 0xffffff8008a761a4), 10576 bytes). To get
# there `gt1x_sensor_id_check` was REMOVED; it has no caller in the factory
# binary. The removal is a declared choice, and the function header
# explains it.
CFLAGS_gt1x_generic.o += -Werror
# gt1x_update.c took -Werror back on 2026-08-23, when the unit reached
# 24 functions out of 24 byte-identical against the oracle (block
# [0xffffff8008a7839c, 0xffffff8008a7b600) minus dump_to_file, 12540 bytes) and
# ZERO RESIDUE in the encoding comparison on all twenty-four. To get
# there `read_reg` was REMOVED; it is in the factory binary neither
# separately nor inlined: note (13) at the end of the file measures that.
CFLAGS_gt1x_update.o += -Werror
# gt1x_tpd.c took -Werror back on 2026-08-23, when the unit reached
# 19 functions out of 19 byte-identical against the oracle (block
# [0xffffff8008a76cf4, 0xffffff8008a7839c), 5800 bytes) and ZERO RESIDUE in the
# encoding comparison on all nineteen.
#
# THE "ZERO RESIDUE" WAS NOT TRUE WHEN IT WAS WRITTEN, and that has to be said.
# Seven functions out of nineteen had unclassified positions, 39 in all
# (gt1x_power_switch 5, gt1x_debug_proc 5, tpd_on 7, tpd_local_init 6,
# tpd_i2c_probe 4, tpd_registration 7, tpd_event_handler 5), ALL of them __LINE__
# values: the file had no #line directives and the notes at the top shift the
# lines relative to the factory. It was the same debt that had been closed that
# night on ILITEK/ltr2568/qmcx983 and had stayed open here -- and an equal size
# could not reveal it.
#
# Today the residue really is zero: tools/derivaline.py derives the #line
# directives from the format printk receives, and gt1x_tpd.o measures 1143 equal
# instructions out of 1267 with ZERO positions left to read by hand. To get
# there six global functions were REMOVED (gt1x_is_tpd_halt, gt1x_auto_update_done,
# gt1x_pm_notifier, tpd_reregister_from_tui, tpd_enter_tui, tpd_exit_tui) and
# the five variables only they used: none of the six appears in stock.map inside
# the block, and a GLOBAL function that existed in the factory build would
# necessarily be there. The file header declares it.
CFLAGS_gt1x_tpd.o += -Werror

# A SINGLE translation unit, and it is NOT a measurement of the factory
# partition: the four functions it holds live in different files in the factory
# build. By adjacency in map order (scout-gt917s/elenco.txt):
#   gt1x_gesture_debug  between gesture_clear_wakeup_data and gt1x_init_node
#                       -> gt1x_extents.c, and the two neighbours agree;
#   dump_to_file        between gt1x_error_erase and gt1x_hold_ss51_dsp_no_reset
#                       -> gt1x_update.c, and the two neighbours agree;
#   tpd_driver_exit     lives in .exit.text and the map does not hold it at all;
#                       its unit is given by the `tpd_driver_t` it passes,
#                       which in the factory build sits with tpd_local_init/suspend/resume
#                       -> gt1x_tpd.c;
#   get_module_name     CANNOT BE DECIDED by adjacency, and it is the only one of
#                       the four: it sits exactly on the seam, preceded by
#                       `comfirm` (which in ALPS is gt1x_tools.c) and followed by
#                       `gt1x_i2c_read` (which in ALPS is gt1x_tpd.c). It is
#                       either the last of one or the first of the other, and from
#                       here there is no telling. Declared rather than chosen.
# They sit together here because this file is the declared remainder of a batch,
# not a reconstructed factory unit; and the known risk is the one in the ft8719
# header (clang may inline into one another functions that in the factory build
# live in distinct objects). It has not shown up here -- all four measure exactly
# as in the factory -- but the day this directory grows it will have to be
# split into real units.
obj-y += gt1x_wtk.o
obj-y += gt1x_extents.o
obj-y += gt1x_generic.o
obj-y += gt1x_tools.o
obj-y += gt1x_tpd.o
obj-y += gt1x_update.o

# ---------------------------------------------------------------------------
# THE 86 UNRESOLVED SYMBOLS: WHAT WAS MEASURED AND WHAT WAS NOT -- 2026-08-23
# ---------------------------------------------------------------------------
# Commit 0678283 said: "On the archive of the six objects: 168 defined,
# 145 cited U, 86 unresolved -- and NONE of them is a function of the GT917S
# block. The 86 are core and tpd framework symbols that the kernel link
# resolves."
#
# THE FIRST HALF HOLDS, THE SECOND DOES NOT. The three numbers reproduce, and
# the intersection between the 86 and the 85 names of the block really is empty:
# nothing is missing that still has to be reconstructed. But FIVE of the 86 are
# resolved by no kernel link, and they are these:
#
#   fix_tp_proc_info    exists in the factory build (stock.map, 0xffffff8008a5151c)
#                       and in no ALPS tree source. It is added by
#                       patches/kernel-stock/mtk-tpd-wtk-tp-info.patch, which
#                       is NOT applied to the tree.
#   g0fc0b8             a name derived from an address, it exists nowhere --
#                       and this file already declares it UNRESOLVED.
#   gt1x_tpd_driver     cited by gt1x_wtk.c:364 and :856, defined by nobody.
#   gt1x_patch_jump_fw  cited without a guard by gt1x_extents.c:670-683, and
#   hotknot_auth_fw     defined only under CONFIG_GTP_REQUEST_FW_UPDATE, which
#                       in the .config in use is "is not set" -- and in any case
#                       only inside GT5688/ and GT1151/, which are off.
#
# The difference between the sets says "nothing is missing to reconstruct". It
# does not say "it links": see the retraction in HANDOFF.md.
