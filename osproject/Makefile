
obj-m := chr_drv_ex1.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules


install:
	$(MAKE) -C $(KDIR) SUBDIRS-$(PWD) module_install

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
