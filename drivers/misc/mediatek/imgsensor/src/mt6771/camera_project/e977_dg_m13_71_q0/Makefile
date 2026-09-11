#
# The sensor drivers of THIS project.
#
# That they belong here and not in common/v1_1 is stated by the binary: the
# __FILE__ of gc8034mipi_Sensor.c and of the others carries the full path --
# "/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/drivers/misc/"
# "mediatek/imgsensor/src/mt6771/camera_project/e977_dg_m13_71_q0/"
# "gc8034_mipi_raw/gc8034mipi_Sensor.c"@0xffffff80091a77b9 -- and
# CONFIG_ARCH_MTK_PROJECT is exactly e977_dg_m13_71_q0.
#
#
# gc0310_mipi_yuv: the directory name MUST match the CONFIG_CUSTOM_KERNEL_IMGSENSOR
# entry ("gc0310_mipi_yuv"), because the mt6771 Makefile builds the path from
# it and imgsensor_set_driver compares the same string with
# SENSOR_DRVNAME_GC0310_MIPI_YUV.
#
obj-y += gc0310_mipi_yuv/
obj-y += gc032a_mipi_yuv/
obj-y += gc8034_mipi_raw/
obj-y += s5k3p3sx_mipi_raw/
obj-y += imx219_mipi_raw/
obj-y += imx230_mipi_raw/
obj-y += imx230xinfengda_mipi_raw/
