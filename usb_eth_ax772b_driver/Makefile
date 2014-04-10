CURRENT	= $(shell uname -r)
TARGET	= asix
OBJS	= asix.o
#MDIR	= drivers/net/usb
#KDIR	= /lib/modules/$(CURRENT)/build
MDIR	= drivers/net/usb
#KDIR	= /opt/broadcom/7581/new_refsw/linux/a0/stblinux-2.6.37
KDIR	= /usr/src/linux-headers-3.2.0-23-generic
SUBLEVEL= $(shell uname -r | cut -d '.' -f 3 | cut -d '.' -f 1 | cut -d '-' -f 1 | cut -d '_' -f 1)

ifneq (,$(filter $(SUBLEVEL),14 15 16 17 18 19 20 21))
MDIR = drivers/usb/net
endif

EXTRA_CFLAGS = -DEXPORT_SYMTAB
PWD = $(shell pwd)
#DEST = /lib/modules/$(CURRENT)/kernel/$(MDIR)
DEST = $(KDIR)/$(MDIR)

obj-m      := $(TARGET).o

default:
	make -C $(KDIR) SUBDIRS=$(PWD) modules

$(TARGET).o: $(OBJS)
	$(LD) $(LD_RFLAG) -r -o $@ $(OBJS)

install:
	su -c "cp -v $(TARGET).ko $(DEST) && /sbin/depmod -a"

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

.PHONY: modules clean

-include $(KDIR)/Rules.make
