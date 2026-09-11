# -I on the `alsps` directory (not only on `alsps/inc`): this file includes
# "inc/cust_alsps.h" and "inc/alsps.h", which is the form the driver was
# MEASURED in -- `tools/misuraisolata.py` compiles with the real command line
# extracted from `out/.../alsps/.alsps.o.cmd`, which has `-I.../alsps` and not
# `-I.../alsps/inc`. The sibling `ltr303/` uses the short form ("cust_alsps.h")
# because it only compiles in tree. Keeping both -I flags here makes the same
# lines compile in both contexts, instead of having a source that is measured
# one way and compiled another.
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/alsps/inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/hwmon/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/sensors-1.0/include

obj-y	:=  ltr2568.o
