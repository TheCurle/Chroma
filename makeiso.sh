# generate our iso image :D
# copy the elf to where grub wants it 
# and delete any old ones for good measure first
rm iso/boot/initrd
rm sync.iso
cp kernel.elf iso/boot/initrd
#xorriso -as mkisofs -no-emul-boot -U -b boot/bootloader -hide boot/bootloader -V "Chroma v4a" -iso-level 3 -o chroma.iso iso/
grub-mkrescue -o sync.iso iso
