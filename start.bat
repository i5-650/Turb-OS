wsl.exe make -j$(nproc --all)
qemu-system-x86_64 -M q35 -cpu max -smp 4 -m 512M -boot d -rtc base=localtime -serial stdio -drive format=raw,file=disk.img -no-reboot -no-shutdown -d int -D log.txt -machine smm=off -cdrom image.iso
