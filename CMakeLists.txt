#project config
cmake_minimum_required(VERSION 3.10)
# cheat the compile test 
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_CROSSCOMPILING 1)

project(chroma)


SET(src_files
    ${CMAKE_SOURCE_DIR}/chroma/kernel.c
    ${CMAKE_SOURCE_DIR}/chroma/video/draw.c
    ${CMAKE_SOURCE_DIR}/chroma/video/print.c
    ${CMAKE_SOURCE_DIR}/chroma/system/cpu.c
    ${CMAKE_SOURCE_DIR}/chroma/system/rw.c
    ${CMAKE_SOURCE_DIR}/chroma/system/serial.c
    ${CMAKE_SOURCE_DIR}/chroma/system/pci.c
    ${CMAKE_SOURCE_DIR}/chroma/system/memory/paging.c
    ${CMAKE_SOURCE_DIR}/chroma/system/memory/abstract_allocator.c
    ${CMAKE_SOURCE_DIR}/chroma/system/memory/physmem.c
    ${CMAKE_SOURCE_DIR}/chroma/system/drivers/keyboard.c
)

SET(lib_files
    ${CMAKE_SOURCE_DIR}/chroma/lainlib/list/basic_list.c
    ${CMAKE_SOURCE_DIR}/chroma/lainlib/mutex/ticketlock.c
    ${CMAKE_SOURCE_DIR}/chroma/lainlib/compression/lzgmini.c
)

include_directories("chroma/inc")

SET(src_no_sse
    ${CMAKE_SOURCE_DIR}/chroma/system/interrupts.c
)

SET(src_preamble
    ${CMAKE_SOURCE_DIR}/global/crt0.o
    ${CMAKE_SOURCE_DIR}/global/crti.o
    ${CMAKE_SOURCE_DIR}/global/crtbegin.o
) 

set(src_epilogue
    ${CMAKE_SOURCE_DIR}/global/crtend.o
    ${CMAKE_SOURCE_DIR}/global/crtn.o
)

set_property(SOURCE ${src_no_sse} PROPERTY COMPILE_FLAGS -mgeneral-regs-only)

add_executable(kernel)

target_sources(kernel PUBLIC ${src_preamble} PUBLIC ${src_files} PUBLIC ${src_no_sse} PUBLIC ${lib_files} PUBLIC ${CMAKE_SOURCE_DIR}/font.o PUBLIC ${src_epilogue})
target_compile_options(kernel PRIVATE -ffreestanding -O2 -Wall -Wextra -Wall -Werror -pedantic -fPIC -fno-exceptions )
target_link_options(kernel PRIVATE -T linker.ld -ffreestanding -O2 -nostdlib -nostartfiles -lgcc)

add_custom_command(
    TARGET kernel
    POST_BUILD
    COMMAND cp ${CMAKE_SOURCE_DIR}/kernel ${CMAKE_SOURCE_DIR}/img/boot/exe
    COMMAND ${CMAKE_SOURCE_DIR}/mkbootimg chroma.json chroma.img
)