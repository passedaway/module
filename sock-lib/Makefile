obj-m := test.o
test-objs := test.o omni_socket.o

KERN_DIR ?= /usr/src/linux-headers-3.2.0-23-generic
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERN_DIR) M=$(PWD)

clean:
	-rm -rf *.o *.ko *.mod.c
