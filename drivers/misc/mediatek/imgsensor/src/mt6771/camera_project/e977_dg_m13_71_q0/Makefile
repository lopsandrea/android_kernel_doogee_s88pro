#
# I driver di sensore di QUESTO progetto.
#
# Che stiano qui e non in common/v1_1 lo dice il binario: il __FILE__ di
# gc8034mipi_Sensor.c e degli altri porta il percorso per esteso --
# "/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/drivers/misc/"
# "mediatek/imgsensor/src/mt6771/camera_project/e977_dg_m13_71_q0/"
# "gc8034_mipi_raw/gc8034mipi_Sensor.c"@0xffffff80091a77b9 -- e
# CONFIG_ARCH_MTK_PROJECT vale proprio e977_dg_m13_71_q0.
#
#
# gc0310_mipi_yuv: il nome della directory DEVE combaciare con la voce di
# CONFIG_CUSTOM_KERNEL_IMGSENSOR ("gc0310_mipi_yuv"), perche' il Makefile di
# mt6771 costruisce il percorso da li' e imgsensor_set_driver confronta la
# stessa stringa con SENSOR_DRVNAME_GC0310_MIPI_YUV.
#
obj-y += gc0310_mipi_yuv/
obj-y += gc032a_mipi_yuv/
obj-y += gc8034_mipi_raw/
obj-y += s5k3p3sx_mipi_raw/
obj-y += imx219_mipi_raw/
obj-y += imx230_mipi_raw/
obj-y += imx230xinfengda_mipi_raw/
