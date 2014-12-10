PWD := $(shell pwd)
obj-m := type.o
type-objs := types.o

KER_SRC := /lib/modules/$(shell uname -r)/build

all:default user

default:
	$(MAKE) -C $(KER_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KER_SRC) M=$(PWD) clean
	-rm -rf types

user:
	$(CC) -D__linux_user__ types.c -o types



