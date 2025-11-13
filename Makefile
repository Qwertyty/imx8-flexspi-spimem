# Warning flags
WARN := -W -Wstrict-prototypes -Wmissing-prototypes

# Module name and objects
obj-m += mfspi.o
mfspi-objs := flexspidev.o w25q128jw.o flexspi_chrdev.o

# Kernel source path (must be ARM64 kernel tree)
KDIR ?= /path/kernel

all:
	@echo "Building ARM64 module on $(shell uname -m) host..."
	$(MAKE) $(WARN) -C $(KDIR) M=$(shell pwd) modules

clean:
	rm -rf $(wildcard *.o *.ko mo* Mo* *.mod.c *.order *.symvers .tmp_versions)
