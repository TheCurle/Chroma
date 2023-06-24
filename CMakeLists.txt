#project config
cmake_minimum_required(VERSION 3.10)

SET(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_COMPILER x86_64-elf-g++)
set(CMAKE_C_COMPILER x86_64-elf-gcc)
set(CMAKE_ASM_COMPILER nasm)
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS} s S)

# cheat the compile test
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_CROSSCOMPILING 1)

enable_language(ASM_NASM)
enable_language(C)
enable_language(CXX)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> \
    <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

project(chroma C CXX ASM_NASM)

SET(src_files
        ${CMAKE_SOURCE_DIR}/src/kernel.cpp
        ${CMAKE_SOURCE_DIR}/src/video/draw.cpp
        ${CMAKE_SOURCE_DIR}/src/video/print.cpp
        ${CMAKE_SOURCE_DIR}/src/system/cpu.cpp
        ${CMAKE_SOURCE_DIR}/src/system/core.cpp
        ${CMAKE_SOURCE_DIR}/src/system/rw.cpp
        ${CMAKE_SOURCE_DIR}/src/system/serial.cpp
        ${CMAKE_SOURCE_DIR}/src/system/interrupts.cpp
        ${CMAKE_SOURCE_DIR}/src/system/acpi/MADT.cpp
        ${CMAKE_SOURCE_DIR}/src/system/acpi/RSDP.cpp
        ${CMAKE_SOURCE_DIR}/src/system/memory/paging.cpp
        ${CMAKE_SOURCE_DIR}/src/system/memory/abstract_allocator.cpp
        ${CMAKE_SOURCE_DIR}/src/system/memory/liballoc.cpp
        ${CMAKE_SOURCE_DIR}/src/system/memory/physmem.c
        ${CMAKE_SOURCE_DIR}/src/system/extern/extern_defs.cpp
        ${CMAKE_SOURCE_DIR}/src/drivers/devices/devices.cpp
        ${CMAKE_SOURCE_DIR}/src/drivers/devices/io/apic.cpp
        ${CMAKE_SOURCE_DIR}/src/drivers/devices/io/ps2_keyboard.cpp
)

SET(lib_files
        ${CMAKE_SOURCE_DIR}/src/lainlib/list/basic_list.cpp
        ${CMAKE_SOURCE_DIR}/src/lainlib/mutex/ticketlock.cpp
        ${CMAKE_SOURCE_DIR}/src/lainlib/string/str.cpp
)

include_directories("inc")

SET(src_as
        ${CMAKE_SOURCE_DIR}/src/global/interrupt.s
)

SET(src_preamble
        ${CMAKE_SOURCE_DIR}/src/global/crt0.o
        ${CMAKE_SOURCE_DIR}/src/global/crti.o
        ${CMAKE_SOURCE_DIR}/src/global/crtbegin.o
)

set(src_epilogue
        ${CMAKE_SOURCE_DIR}/src/global/crtend.o
        ${CMAKE_SOURCE_DIR}/src/global/crtn.o
        ${CMAKE_SOURCE_DIR}/src/assets/font.o
        ${CMAKE_SOURCE_DIR}/src/assets/zerosharp.o
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(kernel)

set_source_files_properties(${src_as} PROPERTIES LANGUAGE ASM_NASM)
target_sources(kernel PUBLIC ${src_preamble} PUBLIC ${src_as} PUBLIC ${src_files} PUBLIC ${lib_files} PUBLIC ${src_epilogue})
target_compile_options(kernel PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-ffreestanding -O0 -Wall -Wextra -Wall -Werror -fPIC -fno-exceptions -fno-omit-frame-pointer -mno-red-zone -fno-stack-protector -fno-strict-aliasing $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ggdb3>)
target_link_options(kernel PRIVATE -T ${CMAKE_SOURCE_DIR}/linker.ld -ffreestanding -O2 -nostdlib -nostartfiles -lgcc)

add_custom_command(TARGET kernel
        POST_BUILD
        COMMAND "C:/Program Files/Git/bin/sh" -l post.sh
)