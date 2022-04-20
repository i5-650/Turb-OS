# Turb-OS
A small OS made in C/C++, made for a supervized project ! 

## What's the goal ?

Have a "usable" OS that can be used on embedded systems. 
Meaning we want to implement:
- dynamic allocation
- scheduling / multitasking
- minimal display

## Tools used:
- clang
- lld
- LLVM
- Make
- nasm
- qemu
- xorriso
- wget
- tar

### Install tools
```
sudo apt-get install clang lld make nasm xorriso wget tar qemu-system-x86
```

if you're on windows, you need to compile with a WSL and run it with qemu ON WINDOWS

After cloning the repo, you can compile and run the OS by using:
```
make all
```
or to go faster and use multiple cores
```
make -j$(nproc --all)
```


# TODO LIST

- [x] minimal display
- [x] dynamic allocation
- [x] interruptions
- [x] keyboard input (PS/2)
- [x] mouse input (PS/2)
- [x] SMP
- [x] UBSAN
- [x] serial debugging
- [x] kernel panic
- [x] HPET (not tested)
- [] scheduler
- [] VFS
- [] THE small detail
