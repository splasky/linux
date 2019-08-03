MOD   := syshook
obj-m := $(addsuffix .o,$(MOD))
PWD   := $(shell pwd)
ccflags-y += -g -DDEBUG -O0

ALREADY_INSTERTTED=$(shell lsmod | grep $(MOD) | awk '{print $$1;}')

ifneq ($(KERNELPATH),)
	KDIR	:= $(KERNELPATH)
else
	# Default kernel path
	KDIR	:= /home/splasky/workspace/flexsc/linux/
endif

ARCH	:= $(shell uname -p)

ifeq ($(ARCH),x86_64)
	KDEF	:= X86_64
else
	KDEF	:= AARCH64
endif

EXTRA_CFLAGS	:= -D $(KDEF)

default :
	$(MAKE) -C $(KDIR) M=$(PWD) modules

run :
ifneq ($(ALREADY_INSTERTTED),)
	sudo rmmod $(MOD)
endif
	sudo insmod $(MOD).ko
	# dmesg
	../user/user-program

rm :
	sudo rmmod $(MOD)
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm $(obj) $(exe)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
