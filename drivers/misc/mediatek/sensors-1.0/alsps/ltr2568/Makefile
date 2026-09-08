# -I sulla cartella `alsps` (non solo su `alsps/inc`): questo file include
# "inc/cust_alsps.h" e "inc/alsps.h", che e' la forma con cui il driver e'
# stato MISURATO -- `tools/misuraisolata.py` compila con la riga di comando
# vera estratta da `out/.../alsps/.alsps.o.cmd`, che ha `-I.../alsps` e non
# `-I.../alsps/inc`. Il sibling `ltr303/` usa la forma corta ("cust_alsps.h")
# perche' compila solo in albero. Tenere qui entrambe le -I fa compilare le
# stesse righe nei due contesti, invece di avere un sorgente che si misura in
# un modo e si compila in un altro.
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/hwmon/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/include

obj-y	:=  ltr2568.o
