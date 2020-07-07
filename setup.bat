# ignore this as it will be deprecated video/tty.c 



cd chroma

# compile all the tings
x86_64-elf-gcc -ffreestanding -O2 -Wall -Wextra -c^
 -I inc^
 kernel.c video/draw.c video/print.c^
 system/memory/paging.c system/memory/physmem.c^
 system/drivers/keyboard.c^
 system/cpu.c system/rw.c system/serial.c

#gcc will complain about sse instrs if we use intr attr without this flag
x86_64-elf-gcc -ffreestanding -O2 -Wall -Wextra  -mgeneral-regs-only -c^
 -I inc^
 system/interrupts.c




cd ..

# get our font for linking
x86_64-elf-ld -r -b binary -o font.o font.psf

# link all the files
x86_64-elf-gcc -T linker.ld -ffreestanding -O2 -nostdlib -lgcc^
 chroma/kernel.o chroma/draw.o chroma/print.o^
 chroma/paging.o chroma/physmem.o^
 chroma/keyboard.o^
 chroma/cpu.o chroma/rw.o chroma/serial.o^
 chroma/interrupts.o^
 font.o^
 -o kernel.elf

# generate our iso image :D
# copy the elf to where grub wants it 
# and delete any old ones for good measure first

rm iso/boot/kernel.elf
rm chroma.iso
cp kernel.elf iso/boot/initrd

xorriso -as mkisofs -U -no-emul-boot -b boot/bootloader -hide boot/bootloader -V "Chroma v4a" -iso-level 3 -o chroma.iso iso/


pause