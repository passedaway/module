PWD := $(shell pwd)
obj-m := test.o
test-objs := test_timer.o

default:
	$(MAKE) -C $(KER_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KER_SRC) M=$(PWD) clean

