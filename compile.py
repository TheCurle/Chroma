import os
import subprocess

CurrentPath = os.getcwd()
print(CurrentPath)

# Here goes the absolute path to your x86_64-mingw-w64 folder.
# There should be a \bin\gcc.exe relative to that folder.
GCC_FOLDER = ""

if os.path.exists('objects.list'):
    os.remove("objects.list")

HFILES = "-Iinclude/ -Iinclude/reqs/ -Iinclude/bitfont/"
OBJECTS = ""

with open('c_files.txt') as file:
    line = file.readline().strip()
    while line:
        line = line[:-2]
        if line == "kernel\interrupts":
            print(f'{HFILES}')
            subprocess.run([f'{GCC_FOLDER}/bin/gcc.exe', '-ffreestanding', '-march=skylake', '-mgeneral-regs-only', '-fno-exceptions', '-fno-stack-protector', '-fno-stack-check', '-fno-strict-aliasing', '-fno-merge-all-constants', '-mno-stack-arg-probe', '-m64', '-mno-red-zone', '-maccumulate-outgoing-args', '--std=gnu11', '-Iinclude/', '-Iinclude/reqs', '-Iinclude/bitfont', '-Og', '-g3', '-Wall', '-Wextra', '-Wdouble-promotion', '-Wpedantic', '-fmessage-length=0', '-ffunction-sections', '-c', '-MMD', '-MP', f'-Wa,-adghlmns={line}.out', f'-MT{line}.o', '-o', f'{line}.o', f'{line}.c'], shell=True)
        else: 
            print(f'{HFILES}')
            subprocess.run([f'{GCC_FOLDER}/bin/gcc.exe', '-ffreestanding', '-march=skylake', '-mavx2', '-fno-exceptions', '-fno-stack-protector', '-fno-stack-check', '-fno-strict-aliasing', '-fno-merge-all-constants', '-mno-stack-arg-probe', '-m64', '-mno-red-zone', '-maccumulate-outgoing-args', '--std=gnu11', '-Iinclude/', '-Iinclude/reqs', '-Iinclude/bitfont', '-Og', '-g3', '-Wall', '-Wextra', '-Wdouble-promotion', '-Wpedantic', '-fmessage-length=0', '-ffunction-sections', '-c', '-MMD', '-MP', f'-Wa,-adghlmns={line}.out', f'-MT{line}.o', '-o', f'{line}.o', f'{line}.c'], shell=True)
        OBJECTS = line + ".o "

        os.path.normpath(OBJECTS)


        with open('objects.list', 'a') as objectsfile:
            ofile = OBJECTS.replace('\\', '/')
            objectsfile.write(ofile + "\n")
        
        line = file.readline().strip()
        
subprocess.run([f'{GCC_FOLDER}/bin/gcc.exe', "-march=skylake", "-mavx2", "-s", "-nostdlib", "-static-pie", "-Wl,--allow-multiple-definition", "-Wl,-e,kernel_main", "-Wl,--dynamicbase,--export-all-symbols",  "-Wl,--subsystem,10", "-Wl,-Map=output.map", "-Wl,--gc-sections", "-o Sync.exe", "@objects.list"])
