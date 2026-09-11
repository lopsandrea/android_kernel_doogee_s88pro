#
# Two translation units, two different paths, and the path matters: the
# .ddebug of the factory kernel says
#   st21nfc  .../drivers/misc/mediatek/nfc/st21nfc/st21nfc.c
#   st54spi  .../drivers/misc/mediatek/nfc/st54spi.c
# so st21nfc.c lives in a subfolder and st54spi.c does not. The resulting
# KBUILD_MODNAMEs ('st21nfc' and 'st54spi') are the factory ones, field by field.
#
obj-$(CONFIG_NFC_CHIP_SUPPORT)	+= st21nfc/
obj-$(CONFIG_NFC_ST54_SPI)	+= st54spi.o
