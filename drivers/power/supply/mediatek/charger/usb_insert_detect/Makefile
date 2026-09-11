# WTK_USB_INSERT_DETECT -- A PARTIAL BATCH, one function out of five.
#
# It goes into `drivers/power/supply/mediatek/charger/usb_insert_detect/`,
# next to `mt5725/`: in the factory kernel its functions start at
# 0xffffff8008ace8f0, that is right after `Mt5725_get_rxdetect`, the last
# one in mt5725.
#
# It exists for one reason only -- to close the mt5725 link -- and its Kconfig
# says so. It is `select`ed by MTK_MT5725_WLS_SUPPORT.

obj-y := usb_insert_detect.o
