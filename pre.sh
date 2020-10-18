#!/bin/sh
x86_64-elf-ld -r -b binary -o font.o font.psf
cmake -G"Unix Makefiles" .
