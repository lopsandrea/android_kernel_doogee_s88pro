#
# Due unita' di traduzione, due percorsi diversi, e il percorso conta: il
# .ddebug del kernel di fabbrica dice
#   st21nfc  .../drivers/misc/mediatek/nfc/st21nfc/st21nfc.c
#   st54spi  .../drivers/misc/mediatek/nfc/st54spi.c
# quindi st21nfc.c sta in una sottocartella e st54spi.c no. I KBUILD_MODNAME che
# ne risultano ('st21nfc' e 'st54spi') sono quelli di fabbrica, campo per campo.
#
obj-$(CONFIG_NFC_CHIP_SUPPORT)	+= st21nfc/
obj-$(CONFIG_NFC_ST54_SPI)	+= st54spi.o
