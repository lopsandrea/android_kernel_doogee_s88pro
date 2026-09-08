# WTK_USB_INSERT_DETECT -- LOTTO PARZIALE, una funzione su cinque.
#
# NEL KERNEL VA IN `drivers/power/supply/mediatek/charger/usb_insert_detect/`,
# accanto a `mt5725/`: nel kernel di fabbrica le sue funzioni cominciano a
# 0xffffff8008ace8f0, cioe' subito dopo `Mt5725_get_rxdetect` che e' l'ultima
# di mt5725.
#
# Esiste per una ragione sola -- chiudere il link di mt5725 -- e il suo Kconfig
# lo dice.  E' `select`ata da MTK_MT5725_WLS_SUPPORT.

obj-y := usb_insert_detect.o
