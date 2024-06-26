SOURCEDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BACKGROUND := $(SOURCEDIR)/misc/bg.bmp
FONT := $(SOURCEDIR)/misc/font.bin
SFNFONT := $(SOURCEDIR)/misc/font.sfn
KERNEL := $(SOURCEDIR)/kernel.elf
ISO := $(SOURCEDIR)/../image.iso
DISK = $(SOURCEDIR)/../disk.img

LVL5_PAGING = 0

CC = clang
CPP = clang++
LD = ld.lld
AS = nasm

CXXFILT = llvm-cxxfilt

LIMINE = $(SOURCEDIR)/../limine-old-version-saved/limine-install
XORRISO = xorriso
QEMU = qemu-system-x86_64

QEMUFLAGS = -M q35 \
	-cpu max -smp 1 -m 512M \
	-boot d -rtc base=localtime \
	-serial stdio -drive format=raw,file=$(DISK)

XORRISOFLAGS = -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-eltorito-efi.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label

CFLAGS = -Ofast -pipe -Werror -DLVL5_PAGING=$(LVL5_PAGING)

CPPFLAGS = -Ofast -pipe -Werror -Wno-c99-designator -DLVL5_PAGING=$(LVL5_PAGING)

LDFLAGS =
ASMFLAGS = -f elf64 -g

override INTERNALLDFLAGS :=  \
	-T$(SOURCEDIR)/linker.ld \
	-nostdlib                \
	-zmax-page-size=0x1000   \
	-static

override INTERNALCFLAGS :=     \
	-target x86_64-pc-none-elf \
	-I$(SOURCEDIR)/            \
	-std=gnu17                 \
	-ffreestanding             \
	-fno-exceptions            \
	-fno-stack-protector       \
	-fno-use-cxa-atexit        \
	-fno-omit-frame-pointer    \
	-fsanitize=undefined       \
	-fno-rtti                  \
	-fno-pic                   \
	-mabi=sysv                 \
	-mno-80387                 \
	-mno-mmx                   \
	-mno-3dnow                 \
	-mno-sse                   \
	-mno-sse2                  \
	-mno-red-zone              \
	-mcmodel=kernel

override INTERNALCPPFLAGS :=   \
	-target x86_64-pc-none-elf \
	-I$(SOURCEDIR)/            \
	-std=gnu++17               \
	-ffreestanding             \
	-fno-exceptions            \
	-fno-stack-protector       \
	-fno-use-cxa-atexit        \
	-fsanitize=undefined       \
	-fno-omit-frame-pointer    \
	-fno-rtti                  \
	-fno-pic                   \
	-mabi=sysv                 \
	-mno-80387                 \
	-mno-mmx                   \
	-mno-3dnow                 \
	-mno-sse                   \
	-mno-sse2                  \
	-mno-red-zone              \
	-mcmodel=kernel

CFILES = $(shell find $(SOURCEDIR)/ -type f -name '*.c' -not -path "$(SOURCEDIR)/initrd/*")
CPPFILES = $(shell find $(SOURCEDIR)/ -type f -name '*.cpp' -not -path "$(SOURCEDIR)/initrd/*")
ASMFILES = $(shell find $(SOURCEDIR)/ -type f -name '*.asm' -not -path "$(SOURCEDIR)/initrd/*")

OBJ = $(CFILES:.c=.o)
OBJ += $(CPPFILES:.cpp=.o)
OBJ += $(ASMFILES:.asm=_asm.o)

.PHONY: all
all: $(KERNEL)
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

iso:
	cp $(KERNEL) $(BACKGROUND) $(FONT) $(SOURCEDIR)/../limine.cfg $(SOURCEDIR)/../limine-old-version-saved/limine.sys \
		$(SOURCEDIR)/../limine-old-version-saved/limine-cd.bin $(SOURCEDIR)/../limine-old-version-saved/limine-eltorito-efi.bin $(SOURCEDIR)/../iso_root/

	@printf "XORRISO\t%s\n" $(lastword $(subst /, ,$(ISO)))
	$(XORRISO) $(XORRISOFLAGS) $(SOURCEDIR)/../iso_root -o $(ISO) 2> /dev/null || echo "\e[31mFailed to build iso!\e[0m"

	@printf "LIMINE\t%s\n" $(lastword $(subst /, ,$(ISO)))
	$(LIMINE) $(ISO) 2> /dev/null || echo "\e[31mFailed to install limine!\e[0m"

clean:
ifndef NOCLEAN
	rm -rf $(KERNEL) $(OBJ) $(HDR_DEPS) $(SOURCEDIR)/../iso_root/* $(SOURCEDIR)/../null.d
endif

run:
	echo "Turbooting \n"
	echo $(QEMU) $(QEMUFLAGS) -d int -D log.txt -machine smm=off -cdrom image.iso
	$(QEMU) $(QEMUFLAGS) -cdrom $(ISO) -drive if=pflash,format=raw,unit=0,file=$(SOURCEDIR)/../OVMF/OVMF_CODE-pure-efi.fd,readonly=on -drive if=pflash,format=raw,unit=1,file=$(SOURCEDIR)/../OVMF/OVMF_VARS-pure-efi.fd
