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
 - [x] manage virtual memory
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
Simply have an [x86_64-elf-gcc](https://github.com/lordmilko/i686-elf-tools) and ld (included!) in your PATH, run `cmake` in the source directory, then `make`.
It will compile the kernel, and create an OS image with `mkbootimg`.


### Linux

The system for linux is a lot easier, but you *do* need an x86_64-elf-gcc cross compiler. You can get one from the AUR on Arch-based distros (like Manjaro), or make one yourself using [the OSDev Wiki guide](https://wiki.osdev.org/GCC_Cross-Compiler)  

On the Chroma side, Simply run the `init.sh` to generate a makefile, then `make` to create the image file.  


The generated IMG works in QEMU, or on a physical test device (unlike a lot of other hobby OSes!)
This means you can use any emulator or hypervisor to run it.

 
## Project structure

The repository has a lot of files and folders after setting up a workspace. This is a guide to all the files and folders.

```

      File Location                          | Description
/
├── bin/                                     |  Binary Output folder
│   └── img/                                 |  Image Output folder.
│       └── chroma.img                       |  Disk Image file, as an alternative to ISO below.
│                                            |                       
├── inc/                                     |  Global header include folder.
│   ├── driver/                              |  Header files for the default driver implementations.
│   ├── editor/                              |  Header files for the builtin editor and debugger.
│   ├── kernel/                              |  Header files for the Chroma kernel itself.
│   └── lainlib/                             |  Header files for the Lainlib standard library.
│                                            | 
├── src/                                     |  Source files.
│   ├── assets/                              |  Assorted linkable files that will be bundled with the built image.
│   ├── drivers/                             |  Handling of the default driver implementations.
│   ├── editor/                              |  Handling of the builtin editor and debugger.
│   ├── global/                              |  Various files used in global objects (ie. the C RunTime, new core bootstrapping, etc)
│   ├── lainlib/                             |  Handling of the Lainlib standard library.
│   ├── system/                              |  Core Kernel files.
│   ├── video/                               |  Writing and drawing on the screen.
│   └── kernel.cpp                           |  The primary kernel entry point.
│                                            | 
├── tools/                                   |  Auxiliary tools used in the buildsystem.
│   └── mkbootimg/                           |  Creates a bootable .img file based on the chroma.json configuration file below.
│                                            | 
├── .gitignore                               |  Git Repository Ignored Files
├── build_and_run.sh                         |  shell script that builds the img file and runs it using run.sh below.
├── choma.bxrc                               |  bxrc (Bochs Runtime Config) file for the Bochs emulator and debugger.
├── chroma.iso                               |  ISO disc image file that can be loaded into a VM or onto a boot disk.
├── chroma.json                              |  MkBootImg configuration file
├── CMakeLists.txt                           |  Buildscript and CMake project configuration file.
├── LICENSE                                  |  Chroma's License. MIT License.
├── linker.ld                                |  GCC linkerscript that places files and addresses at their expected positions.
├── post.sh                                  |  Postprocessing script; Generates the final img file, and attempts to update a VirtualBox configuration named "Chroma" to run this new file instead.
├── pre.sh                                   |  First-time setup script; generates the font file binary and generates the CMake buildscripts.
├── README.md                                |  This file.
└── run.bat                                  |  Attempts to start a Virtualbox VM named "chroma".
```