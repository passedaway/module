CURRENT	= $(shell uname -r)
KDIR	= /opt/broadcom/7230/linux/a0/stblinux-2.6.37

#EXTRA_CFLAGS = -DEXPORT_SYMTAB
PWD := $(shell pwd)


obj-m := sx1213.o
sx1213-objs := sx1213_gpio.o spi.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

