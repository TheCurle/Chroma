![Chroma Logo](https://gemwire.uk/img/chroma/logo/480p.png)

# Chroma
The Chromatic OS

## About
Chroma is an x86_64 kernel, soon to be Operating System.  
It uses the [bootboot](https://gitlab.com/bztsrc/bootboot) bootloader.

## Features
It can currently: 
 - [x] read keyboard input
 - [x] draw to the screen, including text and basic images.
 - [x] output audio over the PC Speaker
 - [x] manage physical memory
 - [ ] manage virtual memory
 - [ ] switch to ring 3
 - [ ] switch tasks
 - [ ] schedule tasks
 - [ ] handle processes and threads
 - [ ] handle mouse input
 - [ ] display a basic 3D object
 - [ ] display a basic 3D world
 - [ ] display a basic 3D world *in VR*

Once we reach this point... well, the world is our oyster.

## Building

Chroma can be built on Windows or Linux.

### Windows
I (Curle) use Windows for developing Chroma.  
Simply have an [x86_64-elf-gcc](https://github.com/lordmilko/i686-elf-tools) and ld (included!) in your PATH, and run setup.bat.  
You will need to use WSL to generate the iso; copy the created kernel.elf file to iso/boot/ as `initrd`, and run `grub-mkrescue -o chroma.iso iso/`.  
A new system is in the works for creating this ISO natively on Windows.  

The generated ISO works in QEMU, or on a physical test device (unlike a lot of other hobby OSes!)

### Linux

The system for linux is a lot easier, but you *do* need an x86_64-elf-gcc cross compiler. You can get one from the AUR on Arch-based distros (like Manjaro), or make one yourself using [the OSDev Wiki guide](https://wiki.osdev.org/GCC_Cross-Compiler)  
Simply run the `init.sh` to generate a makefile, then `make` to create the kernel.elf file.  
Same thing applies as Windows now - copy the kernel.elf file to iso/boot/ as `initrd` and run `grub-mkrescue -o chroma.iso iso/`.


 
