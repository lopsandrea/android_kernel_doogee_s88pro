#
# Makefile for the Sunwave TrustKernel TEE driver (fingerprint reader).
#
# Two translation units, and the order matters: in the factory build the .bss band
# of the driver holds first g_tee_version (0xa100b0c) and the version buffer
# (0xa100c6c), which live in sf_ctl.c, and then g_ctl_dev (0xa100c90) with the seven
# pinctrl_states, which live in sf_hw.c. The module is still called
# "sunwave" because KBUILD_MODNAME comes from the name of the composite object.
#
obj-$(CONFIG_WTK_SUNWAVE_TRUSTKERNEL_TEE) += sunwave.o
sunwave-y := sf_ctl.o sf_hw.o
