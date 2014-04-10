PWD := $(shell pwd)
KER_SRC ?= /usr/src/linux-headers-3.2.0-23-generic/
obj-m := test.o
test-objs := kern_unsock.o  omni_socket.o

default:
	$(MAKE) -C $(KER_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KER_SRC) M=$(PWD) clean

