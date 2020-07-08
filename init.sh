#!/bin/sh
ld -r -b binary -o font.o font.psf
cmake -DCMAKE_C_COMPILER=x86_64-elf-gcc .
