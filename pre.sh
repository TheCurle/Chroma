#!/bin/sh
mkdir -p obj/assets
x86_64-elf-ld -r -b binary -o src/assets/font.o src/assets/font.psf

cmake -G"Unix Makefiles" .
