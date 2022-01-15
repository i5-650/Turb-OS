SOURCEDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BACKGROUND := $(SOURCEDIR)/misc/bg.bmp
FONT := $(SOURCEDIR)/misc/font.bin
SFNFONT := $(SOURCEDIR)/misc/font.sfn
INITRD := $(SOURCEDIR)/initrd.tar.gz
KERNEL := $(SOURCEDIR)/kernel.elf
ISO := $(SOURCEDIR)/../image.iso
DISK = $(SOURCEDIR)/../disk.img

CC = clang
CPP = clang++
LD = ld.lld
AS = nasm

CXXFILT = llvm-cxxfilt

LIMINE = $(SOURCEDIR)/../limine/limine-install
XORRISO = xorriso
QEMU = qemu-system-x86_64

QEMUFLAGS = -M q35 \
	-cpu max -smp 4 -m 512M \
	-boot d -rtc base=localtime \
	-serial stdio -drive format=raw,file=$(DISK)

XORRISOFLAGS = -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-eltorito-efi.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label

CFLAGS = -Ofast -pipe -Werror

CPPFLAGS = -Ofast -pipe -Werror -Wno-c99-designator 

LDFLAGS = 
ASMFLAGS = -f elf64 -g

INTERNALLDFLAGS :=           \
	-T$(SOURCEDIR)/linker.ld \
	-nostdlib                \
	-zmax-page-size=0x1000   \
	-static                  \
	-pie                     \
	--no-dynamic-linker     \
	-ztext

INTERNALCFLAGS  :=             \
	-target x86_64-pc-none-elf \
	-I$(SOURCEDIR)/            \
	-std=gnu17                 \
	-ffreestanding             \
	-fno-exceptions            \
	-fno-stack-protector       \
	-fsanitize=undefined       \
	-fpie                      \
	-mno-80387                 \
	-mno-mmx                   \
	-mno-3dnow                 \
	-mno-red-zone              \
	-mno-sse                   \
	-mno-sse2                  \
	-mcmodel=kernel

INTERNALCPPFLAGS  :=           \
	-target x86_64-pc-none-elf \
	-I$(SOURCEDIR)/            \
	-std=gnu++17               \
	-ffreestanding             \
	-fno-exceptions            \
	-fno-stack-protector       \
	-fsanitize=undefined       \
	-fpie                      \
	-mno-80387                 \
	-mno-mmx                   \
	-mno-3dnow                 \
	-mno-red-zone              \
	-mno-sse                   \
	-mno-sse2                  \
	-mcmodel=kernel

CFILES = $(shell find $(SOURCEDIR)/ -type f -name '*.c' -not -path "$(SOURCEDIR)/initrd/*")
CPPFILES = $(shell find $(SOURCEDIR)/ -type f -name '*.cpp' -not -path "$(SOURCEDIR)/initrd/*")
ASMFILES = $(shell find $(SOURCEDIR)/ -type f -name '*.asm' -not -path "$(SOURCEDIR)/initrd/*")

OBJ = $(CFILES:.c=.o)
OBJ += $(CPPFILES:.cpp=.o)
OBJ += $(ASMFILES:.asm=_asm.o)

.PHONY: all
all: $(KERNEL) initrd
	$(MAKE) iso
	$(MAKE) clean run

$(KERNEL):
	$(MAKE) $(OBJ)
	@printf "Linking\t%s\n" $(lastword $(subst /, ,$(KERNEL)))
	$(LD) $(LDFLAGS) $(INTERNALLDFLAGS) $(SFNFONT) $(OBJ) -o $@

%.o: %.c
	@printf "C\t%s\n" $(lastword $(subst /, ,$<))
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

%.o: %.cpp
	@printf "C++\t%s\n" $(lastword $(subst /, ,$<))
	$(CPP) $(CPPFLAGS) $(INTERNALCPPFLAGS) -c $< -o $@

%_asm.o: %.asm
	@printf "ASM\t%s\n" $(lastword $(subst /, ,$<))
	$(AS) $(ASMFLAGS) $< -o $@

.PHONY: initrd
initrd:
	@printf "TAR\t%s\n" $(lastword $(subst /, ,$(INITRD)))
	tar czf $(INITRD) -C $(SOURCEDIR)/initrd/ ./ --format=ustar

iso:
	cp $(KERNEL) $(BACKGROUND) $(FONT) $(INITRD) $(SOURCEDIR)/../limine.cfg $(SOURCEDIR)/../limine/limine.sys \
		$(SOURCEDIR)/../limine/limine-cd.bin $(SOURCEDIR)/../limine/limine-eltorito-efi.bin $(SOURCEDIR)/../iso_root/

	@printf "XORRISO\t%s\n" $(lastword $(subst /, ,$(ISO)))
	$(XORRISO) $(XORRISOFLAGS) $(SOURCEDIR)/../iso_root -o $(ISO) 2> /dev/null || echo "\e[31mFailed to build iso!\e[0m"

	@printf "LIMINE\t%s\n" $(lastword $(subst /, ,$(ISO)))
	$(LIMINE) $(ISO) 2> /dev/null || echo "\e[31mFailed to install limine!\e[0m"

clean:
ifndef NOCLEAN
	rm -rf $(KERNEL) $(OBJ) $(HDR_DEPS) $(INITRD) $(SOURCEDIR)/../iso_root/* $(SOURCEDIR)/../null.d
endif

run:
	echo "Turbooting \n"
	echo $(QEMU) $(QEMUFLAGS) -no-reboot -no-shutdown -d int -D $(SOURCEDIR)/../log.txt -machine smm=off -cdrom $(ISO)
	$(QEMU) $(QEMUFLAGS) -no-reboot -no-shutdown -d int -D $(SOURCEDIR)/../log.txt -machine smm=off -cdrom $(ISO) | $(CXXFILT)