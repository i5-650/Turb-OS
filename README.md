# Turb-OS
A small OS made in C/C++, made for a supervized project ! The project is currently on hold and may have some updates later.  

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
You may encounter problems while compiling on the asm files, an error about reference. Using development version of those tools may solve it (I used to be on manjaro and have development version tools, didn't find yet where the bug comes from)

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
- [x] scheduler
- [x] VFS
