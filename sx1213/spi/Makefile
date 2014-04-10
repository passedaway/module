CURRENT	= $(shell uname -r)
TARGET	= sx1213
OBJS	= sx1213.o
KDIR	= /opt/broadcom/7230/linux/a0/stblinux-2.6.37

EXTRA_CFLAGS = -DEXPORT_SYMTAB
PWD = $(shell pwd)
DEST = $(KDIR)

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
