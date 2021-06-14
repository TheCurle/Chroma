#project config
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER x86_64-elf-gcc)

# cheat the compile test 
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_CROSSCOMPILING 1)

project(chroma)



SET(src_files
    ${CMAKE_SOURCE_DIR}/src/chroma/kernel.c
    ${CMAKE_SOURCE_DIR}/src/chroma/video/draw.c
    ${CMAKE_SOURCE_DIR}/src/chroma/video/print.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/cpu.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/rw.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/serial.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/pci.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/memory/stack.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/memory/paging.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/memory/abstract_allocator.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/memory/physmem.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/drivers/keyboard.c
    ${CMAKE_SOURCE_DIR}/src/chroma/system/drivers/elf.c
)

SET(lib_files
    ${CMAKE_SOURCE_DIR}/src/chroma/lainlib/list/basic_list.c
    ${CMAKE_SOURCE_DIR}/src/chroma/lainlib/mutex/ticketlock.c
    ${CMAKE_SOURCE_DIR}/src/chroma/lainlib/compression/lzgmini.c
)

include_directories("src/chroma/inc")

SET(src_no_sse
    ${CMAKE_SOURCE_DIR}/src/chroma/system/interrupts.c
)

SET(src_preamble
    ${CMAKE_SOURCE_DIR}/src/global/crt0.o
    ${CMAKE_SOURCE_DIR}/src/global/crti.o
    ${CMAKE_SOURCE_DIR}/src/global/crtbegin.o
) 

set(src_epilogue
    ${CMAKE_SOURCE_DIR}/src/global/crtend.o
    ${CMAKE_SOURCE_DIR}/src/global/crtn.o
)

set_property(SOURCE ${src_no_sse} PROPERTY COMPILE_FLAGS -mgeneral-regs-only)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(kernel)

target_sources(kernel PUBLIC ${src_preamble} PUBLIC ${src_files} PUBLIC ${src_no_sse} PUBLIC ${lib_files} PUBLIC ${CMAKE_SOURCE_DIR}/src/assets/font.o PUBLIC ${src_epilogue})
target_compile_options(kernel PRIVATE -ffreestanding -O0 -Wall -Wextra -Wall -Werror -pedantic -fPIC -fno-exceptions -fno-omit-frame-pointer -mno-red-zone -fno-stack-protector -ggdb3)
target_link_options(kernel PRIVATE -T linker.ld -ffreestanding -O2 -nostdlib -nostartfiles -lgcc)
