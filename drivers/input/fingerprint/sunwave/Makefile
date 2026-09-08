#
# Makefile per il driver Sunwave TrustKernel TEE (lettore di impronte).
#
# Due unita' di traduzione, e l'ordine conta: di fabbrica la fascia .bss del
# driver ha per prime g_tee_version (0xa100b0c) e il buffer di versione
# (0xa100c6c), che stanno in sf_ctl.c, e poi g_ctl_dev (0xa100c90) con i sette
# pinctrl_state, che stanno in sf_hw.c. Il modulo si chiama comunque
# "sunwave" perche' KBUILD_MODNAME viene dal nome dell'oggetto composito.
#
obj-$(CONFIG_WTK_SUNWAVE_TRUSTKERNEL_TEE) += sunwave.o
sunwave-y := sf_ctl.o sf_hw.o
