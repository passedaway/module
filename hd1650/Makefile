PWD := $(shell pwd)
obj-m := hd1650.o
hd1650-objs := kernel.o hd1650-imp.o

default:
	$(MAKE) -C $(KER_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KER_SRC) M=$(PWD) clean

test:
	mipsel-linux-gcc test.c -o test

install:default
	-cp hd1650.ko $(INSTALL_DIR)

