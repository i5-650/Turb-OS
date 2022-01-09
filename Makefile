KERNELDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

all: limine stivale2.h
	@$(MAKE) -s -C $(KERNELDIR)/src clean
	@$(MAKE) -s -C $(KERNELDIR)/src

bios: limine stivale2.h
	@$(MAKE) -s -C $(KERNELDIR)/src clean
	@$(MAKE) -s -C $(KERNELDIR)/src bios

test: limine stivale2.h
	@$(MAKE) -s -C $(KERNELDIR)/src clean
	@$(MAKE) -s -C $(KERNELDIR)/src test

stivale2.h:
	@wget -nc https://github.com/stivale/stivale/raw/master/stivale2.h -P $(KERNELDIR)/src/

limine:
	@git clone https://github.com/limine-bootloader/limine.git --single-branch --branch=latest-binary --depth=1
	@$(MAKE) -C $(KERNELDIR)/limine

clean:
	@$(MAKE) -s -C $(KERNELDIR)/src clean

distclean:
	@$(MAKE) -s -C $(KERNELDIR)/source clean
	@rm -rf $(KERNELDIR)/limine
	@rm -f $(KERNELDIR)/src/stivale2.h