#project config
cmake_minimum_required(VERSION 3.10)
# cheat the compile test 
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_CROSSCOMPILING 1)

project(chroma)


SET(src_files
    chroma/kernel.c
    chroma/video/draw.c
    chroma/video/print.c
    chroma/system/cpu.c
    chroma/system/rw.c
    chroma/system/serial.c
    chroma/system/memory/paging.c
    chroma/system/memory/physmem.c
    chroma/system/drivers/keyboard.c
)

include_directories("chroma/inc")

SET(src_no_sse
    chroma/system/interrupts.c
)

set_property(SOURCE ${src_no_sse} PROPERTY COMPILE_FLAGS -mgeneral-regs-only)

add_executable(kernel.elf ${src_files} ${src_no_sse} font.o)
target_compile_options(kernel.elf PRIVATE -ffreestanding -O2 -Wall -Wextra -v -fPIC)
target_link_options(kernel.elf PRIVATE -T linker.ld -ffreestanding -O2 -nostdlib -lgcc)

