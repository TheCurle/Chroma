/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/* This file contains all of the functions required to handle
 * the system handover from Syncboot.
 */

#include <kernel.h>

static void UpdateSegments(void);
static void InstallInterrupt(size_t ISR, size_t Function);
static void InstallTrap(size_t ISR, size_t Function);

/* The GDT loaded by the kernel to take control from Syncboot.
 * It contains only the segments required to run in kernelmode.
 * The first segment is always 0.
 * The second segment is the Code segment, with exclusive execute permission.
 * The third segment is the Data segment, with r/w permission.
 * The forth segment is the Task segment, which is needed by x86_64.
 * It is static so that it is not stored in EfiLoaderData.
 */

__attribute__((aligned(64))) static size_t InitialGDT[5] = {0, 0x00AF9A000000FFFF, 0x00CF92000000FFFF, 0x0080890000000067, 0};
__attribute__((aligned(64))) static TSS_64 TSS64 = {0};

/* The IDT loaded by UEFI for handling keyboard input is stored in EfiLoaderData.
 * We're gonna need to reclaim that memory back, so we need to load our own IDT.
 * To do that, we can define 256 interrupts for us to use. */

__attribute__((aligned(64))) static IDT_GATE IDTData[256] = {0};

// We might need to allocate pages for the exception stacks.

__attribute__((aligned(4096))) static size_t FirstPageTable[512] = {0};


/*
 *	Following section is taken from the OSDev Wiki page on PC Speaker.
 *  This is beeped first, before *anything* else.
 *  This way, we know that at least *something* works.
 */

 //Play sound using built in speaker
 static void playPCSpeaker(uint32_t frequency) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / frequency;
 	WritePort(0x0043, 0xb6, 1);
 	WritePort(0x0042, (uint8_t) (Div), 1);
 	WritePort(0x0042, (uint8_t) (Div >> 8), 1);
 
        //And play the sound using the PC speaker
 	tmp = ReadPort(0x0061, 1);
  	if (tmp != (tmp | 3)) {
 		WritePort(0x0061, tmp | 3, 1);
 	}
 }
 
 //make it shutup
 static void stopPCSpeaker() {
 	uint8_t tmp = ReadPort(0x0061, 1) & 0xFC;
 
 	WritePort(0x0061, tmp, 1);
 }
 
 //Make a beep
 void beepPCSpeaker() {
 	 playPCSpeaker(1000);
 	 awaitTicks(10);
 	 stopPCSpeaker();
          //set_PIT_2(old_frequency);
 }


void awaitTicks(int ticks) {
    uint64_t FinalTick = time + ticks;

    int i = 0;
    while(i < ticks * 1000) { i++; };
}


/* Main system handover from UEFI.
 * Prepares the processor, the screen, and memory. */
void PrepareSystem(FILELOADER_PARAMS* FLOP) {
    Memory_Info.MemoryMap = FLOP->MemoryMap;
    Memory_Info.MemoryMapSize = FLOP->MemoryMap_Size;
    Memory_Info.MemoryMapDescriptorSize = FLOP->MemoryMapDescriptorSize;
    Memory_Info.MemoryMapDescriptorVersion = FLOP->MemoryMapDescriptorVersion;


    SetupPrinting(FLOP->GPU_Info->GPUs[0]);
    ClearScreen(Print_Info.defaultGPU);
    printf("1");
    SetupPrinting(FLOP->GPU_Info->GPUs[1]);
    ClearScreen(Print_Info.defaultGPU);
    printf("2");
    /* All print functions are now available. */
    printf("ready!");
    
	serialInit();
	serialPrintf("Kernel has been given control of the computer.\nStarting bootstrap init.\n");
	
    FLOP->RTServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, L"");
    
    InstallGDT();
    InstallIDT();

    // WARNING!
    // W A R N I N G !
    // This is  L O U D!
    // Turn your volume down!
    // Seriously!
    // LOWER!
    // IT'S LOUD!

	//beep();

    if(SetIdentityMap(FLOP->RTServices) == NULL) {
        Memory_Info.MemoryMap = FLOP->MemoryMap;
    }


    PrepareAVX();



    /* Bit 5 of CR0 is Numeric Error, which enables
     * the internal x87 Floating Point Math error
     * reporting. */
    size_t CR0 = ReadControlRegister(0);
    // Mask CR0 to only see bit 5
    if( !(CR0 & (1 << 5))) {
        // Preserve CR0, but set bit 5.
        size_t TempReg = CR0 ^ (1 << 5);
        // Write it back
        WriteControlRegister(0, TempReg);

        // Double check. Some processors can be tricky.
        TempReg = ReadControlRegister(CR0);
        if(TempReg == CR0)
            printf("Error setting CR0.NE\r\n");

    }

    /* Bit 10 of CR4 is OSXMMEXCPT, which enables
     * SSE instructions. */
    size_t CR4 = ReadControlRegister(4);
    // Mask CR4 to only see bit 10
    if( !(CR4 & (1 << 10))) {
        // Preserve CR4, but set bit 10.
        size_t TempReg = CR4 ^ (1 << 10);
        // Write it back
        WriteControlRegister(4, TempReg);

        // Double check. Some processors can be tricky.
        TempReg = ReadControlRegister(4);
        if(TempReg == CR4)
            printf("Error setting CR4.OSXMMEXCPT\r\n");
    }

    // Set up memory management
    InstallMemoryMap();
    InstallPaging();

    // Clean up UEFI's mess
    ReclaimEfiBootServicesMemory();
    ReclaimEfiLoaderCodeMemory();

    // Let Intel ME take over power management
    PreparePowerManagement();

}



/* A temporary system for keeping track of system performance. */

size_t ClockTick() {

    // We will be reading a register, so we need to split it up.
    size_t RegHigh = 0, RegLow = 0;
    
    __asm__ __volatile__("rdtscp" : "=a" (RegLow), "=d" (RegHigh) : : "%rcx");
    return (RegHigh << 32 | RegLow);
}


void PrepareAVX() {

    size_t RFLAGS = ReadControlRegister('f');
    
    // Bit 21 of EFLAGS is ID, which tells whether the CPUID instruction is supported.
    size_t ID = RFLAGS ^ (1 << 21);

    WriteControlRegister('f', ID);

    ID = ReadControlRegister('f');
    if  (ID == RFLAGS) {
        printf("CPUID is not supported.\r\n");
    } else {
        // We're going to be receiving input at these 3 registers
        size_t RBX = 0, RCX = 0, RDX = 0;

        __asm__ __volatile__("cpuid" :                  // Instruction
                             "=c" (RCX), "=d" (RDX) :   // Outputs (our results come back through here)
                             "a" (0x01) :               // Inputs (to rax)
                             "%rbx");                   // (anti-)Clobber list
        // Passing 0x01 as the "leaf" to CPUID gives us the Processor Feature information.
        
        // As part of the instruction, we get output in both RCX and RDX.
        // Bit 27 of RCX is the OSXSAVE bit - it says whether XSAVE was enabled by the operating system (us)
        if  (RCX  & (1 << 27)) {
            AVXStub();
        } else {
            // If we get here, OSXSAVE is not set, so it is our duty to enable it, so that we can use extended processor features.

            // To do that though, we need to know if it is even supported.
            // That information is stored in bit 26 of RCX; xsave.
            if(RCX & (1 << 26)) {
                // Okay, XSAVE is supported, so we need to set it.
                // To do that, we set bit 18 of CR4.

                size_t CR4 = ReadControlRegister(4);
                WriteControlRegister(4, CR4 ^ (1 << 18));
                
                // Double check, because some processors... etc.
                if(CR4 & (1 << 18)) {
                    // XSAVE enabled.
                    // Now we do the checks for AVX and AVX512.
                    AVXStub();
                } else {
                    // For some reason we weren't able to enable OSXSAVE.
                    printf("Unable to set OSXSaVE.\r\n");
                }
            } else {
                // XSAVE is not supported, so we cannot enable any AVX features.
                printf("XSAVE is not supported.\r\n");
            }
        }
    }
}

/*  All of this code is called twice when setting up AVX.
    The compiler would just use a jmp instruction anyway, but to make this file cleaner,
    I chose to put it here.
    This function is not visible to any other code. */

void AVXStub() {
    size_t RBX = 0, RCX = 0, RDX = 0;

    __asm__ __volatile__("cpuid" :                  // Instruction
                         "=c" (RCX), "=d" (RDX) :   // Outputs (our results come back through here)
                         "a" (0x01) :               // Inputs (to rax)
                         "%rbx");                   // (anti-)Clobber list
                         
    // Bit 28 of RCX is the AVX bit - it says whether or not AVX (Advanced Vector Extensions) is available.
    if  (RCX & (1 << 28)) {
        // If we get here, AVX is available, so we need to enable it.
        // To do that, we need to set bit 7 of XCR0 (eXtended Control Register 0)
        // So, read the current register..
        size_t XCR0 = ReadExtendedControlRegister(0);
        // Write it back with bit 7 set
        WriteExtendedControlRegister(0, XCR0 | 0x7);
        // Double check it was set properly..
        XCR0 = ReadExtendedControlRegister(0);
        if  ((XCR0 & 0x7) == 0x7) {
            // AVX is now available, we can move on to AVX2 and AVX512.
            // Like before, we need to check first.
            __asm__ __volatile__("cpuid":                               // Instruction
                                 "=b" (RBX), "=c" (RCX), "=d" (RDX) :   // Get our values out
                                 "a" (0x07), "c" (0x00) :);             // Pass 0x07 to RAX and 0x00 to RCX.
            // Leaf 7/0 (0x07 in EAX and 0x00 in ECX) means Extended Features.
            // We're interested in bit 16, which is "avx512f", which says whether AVX-512 is available.
            if  (RBX & (1 << 16)) {
                // If we get here, AVX512 is available, so we need to enable it.
                // To do that, we need to set bits 0xE7 of XCR0 (eXtended Control Register 0)
                size_t XCR0 = ReadExtendedControlRegister(0);
                WriteExtendedControlRegister(0, XCR0 | 0xE7);
                // Double check it was set properly..
                if  ((XCR0 & 0xE7) == 0xE7) {
                    // AVX512 was enabled. We can now use parallel-optimised functions.
                    FillScreen(Print_Info.defaultGPU, Print_Info.charBGColor);
                    printf("AVX512 available and enabled.\r\n");
                } else {
                    // AVX512 was not enabled for some reason.
                    printf("Unable to set AVX512. Please debug later.\r\n");
                }
                // avx512f also includes a bunch of other AVX512 related features, so we should check those too.
                printf("Checking for other AVX512 features..");

                if  (RBX & (1 << 17)) {
                    printf("AVX512-DQ is available.\r\n");
                }
                if  (RBX & (1 << 21)) {
                    printf("AVX512-IFMA is available.\r\n");
                }
                if  (RBX & (1 << 26)) {
                    printf("AVX512-PF is available.\r\n");
                }
                if  (RBX & (1 << 27)) {
                    printf("AVX512-ER is available.\r\n");
                }
                if  (RBX & (1 << 28)) {
                    printf("AVX512-CD is available.\r\n");
                } 
                if  (RBX & (1 << 30)) {
                    printf("AVX512-BW is available.\r\n");
                }
                if  (RBX & (1 << 31)) {
                    printf("AVX512-VL is available.\r\n");
                }
                if  (RCX & 1) {
                    printf("AVX512-VBMI is available.\r\n");
                }
                if  (RCX & (1 << 6)) {
                    printf("AVX512-VBMI2 is available.\r\n");
                }
                if  (RCX & (1 << 11)) {
                    printf("AVX512-VNNI is available.\r\n");
                }
                if  (RCX & (1 << 12)) {
                    printf("AVX512-BITALG is available.\r\n");
                }
                if  (RCX & (1 << 14)) {
                    printf("AVX512-VPOPCNTDQ is available.\r\n");
                }
                if  (RDX & (1 << 2)) {
                    printf("AVX512-4VNNIW is available.\r\n");
                }
                if  (RDX & (1 << 3)) {
                    printf("AVX512-4FMAPS is available.\r\n");
                }   
                printf("End of AVX512 features.\r\n");
            } else {
                // If we get here, AVX512 is not supported.
                FillScreen(Print_Info.defaultGPU, Print_Info.charBGColor);
                printf("AVX/AVX2 supported and enabled.\r\nAVX512 is not supported.\r\n");
            }
            // Now we can check for AVX2.
            // This is stored in bit 5 of RBX returned from CPUID.
            if  (RBX & (1 << 5)) {
                printf("AVX2 is supported.\r\n");
            } else {
                printf("AVX2 is not supported.\r\n");
            }
        } else {
            // If we get here, AVX is supported but for whatever reason, we couldn't enable it.
            printf("Unable to enable AVX.\r\n");
        }
    } else {
        // If we get here, AVX is not supported.
        // So, we check for the latest features of the CPU.
        printf("AVX is not supported.\r\nChecking for latest CPU features..\r\n");
            
        // Bit 20 of RCX is sse4.2, which says whether SSE4.2 instructions are supported.
        if  (RCX & (1 << 20)) {
            printf("SSE 4.2 is supported.\r\n");
        } else {
            // This must be quite an old processor.
            // Bit 19 of RCX is sse4.1, which says whether SSE4.1 instructions are supported..
            if  (RCX & (1 << 19)) {
                printf("SSE 4.1 is supported.\r\n");
            } else {
                // Going back in time..
                // Bit 9 of RCX is ssse3, which says whether Supplemental SSE3 instructions are supported.
                if  (RCX & (1 << 9)) {
                    printf("SSE3 is supported.\r\n");
                } else {
                    // Gee gosh. Okay, there's another place we can check for SSE3..
                    // Bit 1 of RCX is sse3, which says whether Prescott SSE3 instructions are supported.
                    if  (RCX & 1) {
                        printf("SSE3 is supported.\r\n");
                    } else {
                        // Dear me. We might have an issue..
                        // Bit 26 of RDX is sse2, which says whether SSE2 instructions are supported.
                        // SSE2 is required for a processor to be x86_64.
                        if  (RDX & (1 << 26)) {
                            printf("SSE2 is supported.\r\n");
                        } else {
                            // If we get here, the computer is a paradox. Or it isn't to spec.
                            // This kernel is x86_64, therefore for a computer to load it, the processor must also be x86_64.
                            // But if we get here, SSE2 is not supported, which is a requirement for x86_64.
                            printf("Bad CPU detected - x86_64 requires SSE2 but the processor does not support it.\r\n");
                        }
                    }
                }
            }
        }
    }
}


void PrepareMaskableInterrupts(void) {
    // Maskable Interrupts include things like keyboard inputs.
    // To enable them, we set a few flags in RFLAGS.
    size_t RFLAGS = ReadControlRegister('f');
    
    // Bit 9 is IE (Interrupt Enable).
    if  (RFLAGS & (1 << 9)) {
        printf("Interrupts are already enabled.\r\n");
        // This should be the default state after booting from Syncboot.
    } else {
        // Write bit 9 into the register
        WriteControlRegister('f', RFLAGS | (1 << 9));

        // Double check, some processors are tricky sometimes.
        size_t IE = ReadControlRegister('f');
        if  (RFLAGS == IE) { 
            printf("Unable to enable interrupts.\r\n");
        } else {
            printf("Interrupts enabled.\r\n");
        }
    }
}

void PreparePowerManagement() {
    // The CPU can handle most power management features since Skylake.
    // We can enable this with Model-Specific registers.
    size_t RAX = 0;

    // To get whether or not Hardware Power Management is available, we need to check 
    // CPUID.
    // To do *that*, we neeed to use ASM.
    // The terms are separated by ':'.
    // The first term is the instruction.
    // The second term is the outputs. There can and will be more than one, as such happened above.
    // The third term is inputs. Usually these are stored in "ekx", where k is one of a, b, c, d.
    // The forth and last term is the clobber list. Most instructions will clobber (leave garbage in) registers unless they are specified here.
    __asm__ __volatile__("cpuid" :
                         "=a" (RAX) :
                         "a" (0x06) :
                         "%rbx", "%rcx", "%rdx");

    // Now that we have our information, we cna do some checks.
    // Since we passed leaf 6 (0x06 in EAX) to CPUID, we get special information back in the registers.
    // We only told it to output to one register, RAX.
    // Thus, we can check the result in our RAX variable.
    // We're interested in bit 7, which tells us whether or not HWP is available.
    if  (RAX & (1 << 7)) {
        // If we get here, HWP is available.
        // To know whether or not to *enable* HWP, we need to read a Model-Specific Register.
        if  (ReadModelSpecificRegister(0x770) & 1) {
            printf("Hardware Power Management is enabled.\r\n");
        } else {
            // If we get here, we need to enable it manually. To do this, we just set the bit we checked just now.
            WriteModelSpecificRegister(0x770, 1);

            // We should double check that the register was changed. In some cases, this means that HWP is either managed by ME, or the CPU is being funky.
            if(ReadModelSpecificRegister(0x770) & 1) {
                printf("Hardware Power Management has been enabled.\r\n");
                // The message here is slightly different on purpose.
            } else {
                printf("Unable to set Hardware Power Management.\r\n");
            }
        }
    } else {
        // Sadly, PowerManagement is not available for this processor.
        printf("Hardware Power Management is not supported.\r\n");
    }
}

/*  CheckForHypervisor:
 *  If we're running in a virtual machine, a certain bit in CPUID is set. 
 *  This is set by the Hypervisor that runs the VM.
 *  We can check it to see if we're running in a virtual machine.
 */

uint8_t CheckForHypervisor() {
    size_t RCX;

    __asm__ __volatile__("cpuid" :
                         "=c" (RCX) :
                         "a" (1) :
                         "%rbx", "%rdx");
    
    // Bit 31 of the 1st leaf tells us whether there is a hypervisor running.
    return (RCX & (1 << 31));
}

uint8_t ReadPerformance(size_t* Perfs) {
    printf("Starting performance check..\r\n");
	// We cannot read the performance MSRs in virtual machines.
	if(CheckForHypervisor()) {
        printf("Hypervisor detected. Unable to read performance.\r\n");
        return 0;
    }

    // We need to disable interrupts first.
    // They will be enabled by the next function.
    size_t RFLAGS = ReadControlRegister('f');
    if((RFLAGS & (1 << 9))) {
        WriteControlRegister('f', RFLAGS & ~(1 << 9));
    }

    size_t IE = ReadControlRegister('f');
    if(IE == RFLAGS) {
        printf("Unable to disable interrupts for reading performance. Results may be skewed.\r\n");
    }

    // First we need to check for CPU-specific speed features.

    size_t SpeedCheck = ReadModelSpecificRegister(0x1A0);

    // Bit 16 is Enhanced SpeedStep.
    if(SpeedCheck & (1 << 16)) {
        printf("Enhanced SpeedStep is enabled.\r\n");
    }

    // Bit 38 is Turbo Boost.
    if(SpeedCheck & (1ULL << 38)) { // Since this is larger than 32 bits, we need to specify Unsigned Long Long, which makes it 64 bits.
        printf("Turbo Boost is enabled.\r\n");
    }

    Perfs[0] = ReadModelSpecificRegister(0xE8);
    Perfs[1] = ReadModelSpecificRegister(0xE7);
    return 1;
}

size_t ReadCPUFrequency(size_t* Perfs, uint8_t AverageOrDirect) {
    size_t RAX = 0, RBX = 0, RCX = 0, MaxLEAF = 0, APerf = 1, MPerf = 1;
    size_t RFLAGS = 0, TFLAGS = 0;

    if(AverageOrDirect == 1) {
        // Measure
        __asm__ __volatile__ ("cpuid":::"%rax", "%rbx", "%rcx", "%rdx");

        size_t TAPerf = ReadModelSpecificRegister(0xE8);
        size_t TMPerf = ReadModelSpecificRegister(0xE7);

        APerf = TAPerf - Perfs[0];
        MPerf = TMPerf - Perfs[0];
    } else {
        // Average (since last poweroff)
        ReadPerformance(Perfs);
    }

    __asm__ __volatile__("cpuid" :
                         "=a" (MaxLEAF) :
                         "a" (0) :
                         "%rbx", "%rcx", "%rdx");
    
    size_t BusMultiplier = (ReadModelSpecificRegister(0xCE) & 0xFF00) >> 8;
    size_t TurboSpeedControlFrequency = BusMultiplier * 100;

    if(MaxLEAF >= 0x15) {
        __asm__ __volatile__("cpuid" :
                             "=a" (RAX), "=b" (RBX), "=c" (RCX) :
                             "a" (0x15) :
                             "%rdx");
        
        if((RCX) && (RBX)) {
            // RCX contains the nominal clock frequency
            return ((RCX / 1000000) * RBX * APerf) / (RAX * MPerf);
        }

        if(RCX == 0) {
            size_t Val;
            __asm__ __volatile__("cpuid" :
                                 "=a" (Val) :
                                 "a" (1) :
                                 "%rbx", "%rcx", "%rdx");
            if( ((Val & 0xF0FF0) == 0x906E0) || ((Val & 0xF0FF0) == 0x806E0) || ((Val & 0xF0FF0) == 0x506E0) || ((Val & 0xF0FF0) == 0x406E0)) {
                return (24 * RBX * APerf) / (RAX * MPerf);
            }
            
            // There are far more edge cases here. Maybe peek at the Linux kernel?
        }
    }

    // CPUID is not useful. So, fall back to the Sandybridge method.
    size_t Frequency = (TurboSpeedControlFrequency * APerf) / MPerf;

    // Re-enable interrupts. Assuming they're disabled by the prior function.
    RFLAGS = ReadControlRegister('f');

    WriteControlRegister('f', RFLAGS | (1 << 9));
    TFLAGS = ReadControlRegister('f');

    if(TFLAGS == RFLAGS) {
        printf("Unable to enable interrupts after reading performance.\r\n");
    }

    printf("CPU Frequency is %llu\r\n", Frequency);
    return Frequency;
}

void InstallGDT() {
    DESCRIPTOR_TABLE_POINTER GDTData = {0};

    size_t TSS64Address = (size_t)&TSS64;

    uint16_t TSSBase1 = (uint16_t)TSS64Address;
    uint8_t TSSBase2 = (uint8_t)(TSS64Address >> 16);
    uint8_t TSSBase3 = (uint8_t)(TSS64Address >> 24);
    uint32_t TSSBase4 = (uint32_t)(TSS64Address >> 32);

    GDTData.Limit = sizeof(InitialGDT) - 1;
    GDTData.BaseAddress = (size_t)InitialGDT;

    ((TSS_ENTRY*) &((GDT_ENTRY*)InitialGDT)[3])->BaseLow        = TSSBase1;
    ((TSS_ENTRY*) &((GDT_ENTRY*)InitialGDT)[3])->BaseMiddle1    = TSSBase2;
    ((TSS_ENTRY*) &((GDT_ENTRY*)InitialGDT)[3])->BaseMiddle2    = TSSBase3;
    ((TSS_ENTRY*) &((GDT_ENTRY*)InitialGDT)[3])->BaseHigh       = TSSBase4;
    
    SetGDT(GDTData);
    SetTSR(0x18); // 0x18 >> 3 == GDT[3]
    UpdateSegments();
}

static void UpdateSegments() {
    // We can't use the ASM method in x86_64. AKA, we cannot far jump into the code segment to update CS.
    // As such, we have to do some juggling with registers.
    // This will look insane.

    __asm__ __volatile__ ("mov $16, %ax\n\t" // 16 >> 3 = GDT[2] = Data Segment
                          "mov %ax, %ds\n\t"
                          "mov %ax, %es\n\t"
                          "mov %ax, %fs\n\t"
                          "mov %ax, %gs\n\t"
                          "mov %ax, %ss\n\t"
                          "movq $8, %rdx\n\t" // 8 >> 3 = GDT[1] == Code Segment
                          "leaq 4(%rip), %rax\n\t" // Store the instruction immediately after iretq.
                          "pushq %rdx\n\t"
                          "pushq %rax\n\r"
                          "lretq\n\t");
                          // the instruction stored here points to here, so execution continues immediately after the return.
                          // This way, the compiler is happy, and we're now in the new segments.
}

// Exception stacks
#define PAGE (1 << 12)
__attribute((aligned(64))) static volatile uint8_t NMIStack[PAGE] = {0};
__attribute((aligned(64))) static volatile uint8_t DoubleFaultStack[PAGE] = {0};
__attribute((aligned(64))) static volatile uint8_t MachineCheckStack[PAGE] = {0};
__attribute((aligned(64))) static volatile uint8_t BreakPointStack[PAGE] = {0};

void InstallIDT() {
    DESCRIPTOR_TABLE_POINTER IDT_Data = {0};
    IDT_Data.Limit = sizeof(IDTData) - 1;
    IDT_Data.BaseAddress = (size_t) IDTData;

    TSS64.IST1 = (size_t) NMIStack;
    TSS64.IST2 = (size_t) DoubleFaultStack;
    TSS64.IST3 = (size_t) MachineCheckStack;
    TSS64.IST4 = (size_t) BreakPointStack;

    // Set the gates
    InstallInterrupt(0, (size_t) ISR0Handler);
    InstallInterrupt(1, (size_t) ISR1Handler);
    InstallInterrupt(2, (size_t) ISR2Handler);
    InstallInterrupt(3, (size_t) ISR3Handler);
    InstallInterrupt(4, (size_t) ISR4Handler);
    InstallInterrupt(5, (size_t) ISR5Handler);
    InstallInterrupt(6, (size_t) ISR6Handler);
    InstallInterrupt(7, (size_t) ISR7Handler);
    InstallInterrupt(8, (size_t) ISR8Handler);
    InstallInterrupt(9, (size_t) ISR9Handler);   
    InstallInterrupt(10, (size_t) ISR10Handler);
    InstallInterrupt(11, (size_t) ISR11Handler);
    InstallInterrupt(12, (size_t) ISR12Handler);
    InstallInterrupt(13, (size_t) ISR13Handler);
    InstallInterrupt(14, (size_t) ISR14Handler);
    InstallInterrupt(15, (size_t) ISR15Handler);
    InstallInterrupt(16, (size_t) ISR16Handler);
    InstallInterrupt(17, (size_t) ISR17Handler);
    InstallInterrupt(18, (size_t) ISR18Handler);
    InstallInterrupt(19, (size_t) ISR19Handler);
    InstallInterrupt(20, (size_t) ISR20Handler);
    InstallInterrupt(30, (size_t) ISR30Handler);

    // 21 - 31 are reserved.
    for(size_t i = 1; i < 11; i++) {
        if( i != 9 ) { // Don't want to overwrite ISR30
            InstallInterrupt(i + 20, (size_t)ReservedISRHandler);
        }
    }

    // Put custom ISRs here.

    SetIDT(IDT_Data);
}

// Sets the correct entry in the IDT.
// Might be worth looking into using IST stack switching.

static void InstallInterrupt(size_t ISR, size_t Address) {
    uint16_t ISRBase1 = (uint16_t) Address;
    uint16_t ISRBase2 = (uint16_t) (Address >> 16);
    uint32_t ISRBase3 = (uint16_t) (Address >> 32);


    IDTData[ISR].LowBase = ISRBase1;
    IDTData[ISR].Segment = 0x08; // Code Segment
    IDTData[ISR].IST = 0;

    IDTData[ISR].SegmentType = 0x8E; // Interrupt
    IDTData[ISR].MiddleBase = ISRBase2;
    IDTData[ISR].HighBase = ISRBase3;
    IDTData[ISR].Reserved = 0;
}

// Set up paging in the CPU.
// This is going to be the most changed thing, so it should be moved to its own file.
#define PAGETABLE_SIZE 512 * 8

void InstallPaging() {

    // Before we start, we need to write a control register.
    // Bit 7 of CR4 is PGE (Page Global Enabled), which says whtether address translations can be used across address spaces
    // AKA, it says whether or not the page table is an identity map (1:1 relation between page table to RAM)

    size_t CR4 = ReadControlRegister(4);
    if(CR4 & (1 << 7)) { // If bit 7 is set
        // We need to turn it off, otherwise UEFI will leave stuff behind.
        WriteControlRegister(4, CR4 ^ (1 << 7));

        // Double check it was set
        size_t PGE = ReadControlRegister(4);
        if(PGE == CR4) {
            printf("Error disabling Page Global.\r\n");
        }
    }

    // Before we start mapping memory, we need to know how much there is.
    size_t MaxMemory = FetchMemoryLimit();

    // We also need to know how big the pages can be.
    // CPUID can do this for us.

    size_t RDX = 0;
    __asm__ __volatile__("cpuid" : "=d" (RDX) : "a" (0x80000001) : "%rbx", "%rcx");

    if(RDX & (1 << 26)) {
        printf("Using 1GB pages.\r\n");
        
        // For future proofing, we can check if Level-5 paging is enabled.
        // To do this, we check bit 12 (LA57) of CR4.
        CR4 = ReadControlRegister(4);
        if (CR4 & (1 << 12)) {
            // We can use 5 level paging.
            printf("Using 5-Level paging.\r\n");

            // We need to check, just in case.
            if(MaxMemory > (1ULL << 57)) { // 1 << 57 = 128PB
                printf("Max RAM is 128PB. Please consider using a better OS with your supercomputer.\r\n");
                printf("This isn't an error. You'll just be limited to 128PB. *just*.\r\n");
            }

            // PML = Page Map Level

            // Keeps track of how many PML5 entries there are
            size_t MaxPML5 = 1;

            // Keeps track of how many PML4 entries there are
            size_t MaxPML4 = 1;

            // Keeps track of how many PDP entries there are
            size_t MaxPDP = 512;

            // Going to top-down search from 512
            size_t LastPL4Entry = 512;
            size_t LastPDPEntry = 512;

            // PML5 entries can track 256PB of RAM. So we need to count how many x256PB sections can fit into memory.
            // This is usually one.
            while   (MaxMemory > (256ULL << 30)) {
                MaxPML5++;
                MaxMemory -= (256ULL << 30);
            }

            // We can't have more than 512 entries. This is a *lot* of RAM, though.
            if  (MaxPML5 > 512) {
                MaxPML5 = 512;
            }
    
            // We will always need to make sure the rest of memory is paged.
            if  (MaxMemory) {
                // (MaxMemory + ((1 << 30) - 1 )) undoes the MaxMemory -= (256ULL << 30) from earlier.
                // Masking with ~0ULL (64 bits of 1) makes sure that all of the available memory is captured
                // Shifting it back 30 bits allows us to capture the memory address.
                LastPDPEntry = ( (MaxMemory + ((1 << 30) - 1)) & (~0ULL << 30)) >> 30;

                // We need to truncate again.
                if  (MaxPML5 > 512) {
                    MaxPML5 = 512;
                }
            }
        

            // Now we need to calculate how much space the page tables themselves will consume.
            size_t PML4Size = PAGETABLE_SIZE * MaxPML5;
            size_t PDPSize = PML4Size * MaxPML4;

            EFI_PHYSICAL_ADDRESS PML4Base = AllocatePagetable(PML4Size + PDPSize);
            EFI_PHYSICAL_ADDRESS PDPBase = PML4Base + PML4Size;

            // Now we know how big the tables are, and where they are, we can start populating them.
            for(size_t PML5Entry = 0; PML5Entry < MaxPML5; PML5Entry++) {
                
                // Set PML4, make sure it's page-aligned.
                FirstPageTable[PML5Entry] = PML4Base + (PML5Entry << 12);

                if  (PML5Entry == (MaxPML5 - 1)) {
                    MaxPML4 = LastPL4Entry;
                }

                for(size_t PML4Entry = 0; PML4Entry < MaxPML4; PML4Entry++) {

                    ((size_t* )FirstPageTable[PML5Entry])[PML4Entry] = PDPBase + (((PML5Entry << 9) + PML4Entry) << 12);

                    if( (PML5Entry == (MaxPML5 - 1)) && (PML4Entry == (MaxPML4 - 1)) ) {
                        MaxPDP = LastPDPEntry;
                    }

                    for(size_t PDPEntry = 0; PDPEntry < MaxPDP; PDPEntry++) {
                        // This is the table that defines the 1GB pages.
                        // There will only be 1 PML4 entry, unless the system has >512GB of RAM.
                        // There will only be 1 PML5 entry, unless the system has >256TB of RAM.
                        ((size_t* ) ((size_t* )FirstPageTable[PML5Entry])[PML4Entry])[PDPEntry] = ( ((PML5Entry << 18) + (PML4Entry << 9) + PDPEntry) << 30) | (0x83);
                    }

                    // Set R/W and P to 1
                    ((size_t* )FirstPageTable[PML5Entry])[PML4Entry] |= 0x3;
                }

                // Set R/W and P to 1
                FirstPageTable[PML5Entry] |= 0x3;
            }
        } else {
            // 5-level paging isn't supported.
            // We can use 4-level paging.
            // It supports up to 256TB of RAM, which is overkill.

            printf("4-Level paging enabled.\r\n");

            if(MaxMemory > (1ULL << 48)) {
                printf("RAM will be limited to 256TB.\r\n");
            }

            size_t MaxPML4 = 1;
            size_t MaxPDP = 512;

            size_t LastPDPEntry = 512;

            // Each PML4 entry is for a whole 512GB of RAM.
            // Again, usually there will only be one of these.

            while(MaxMemory > (512ULL << 30)) {
                MaxPML4++;
                MaxMemory -= (512ULL << 30);
            }

            if(MaxPML4 > 512) {
                MaxPML4 = 512;
            }

            // Start paging the rest of memory
            if(MaxMemory) {
                LastPDPEntry = ( (MaxMemory + ((1 << 30) - 1)) & (~0ULL << 30)) >> 30;

                if(LastPDPEntry > 512) {
                    LastPDPEntry = 512;
                }

            }

            size_t PDPSize = PAGETABLE_SIZE * MaxPML4;
            EFI_PHYSICAL_ADDRESS PDPBase = AllocatePagetable(PDPSize);

            for(size_t PML4Entry = 0; PML4Entry < MaxPML4; PML4Entry++ ) {
                FirstPageTable[PML4Entry] = PDPBase + (PML4Entry << 12);

                if(PML4Entry == (MaxPML4 - 1)) {
                    MaxPDP = LastPDPEntry;
                }

                for(size_t PDPEntry = 0; PDPEntry < MaxPDP; PDPEntry++) {
                    ((size_t* )FirstPageTable[PML4Entry])[PDPEntry] = (((PML4Entry << 9) + PDPEntry) << 30) | 0x83;
                }

                FirstPageTable[PML4Entry] |= 0x3;
            }
        }
    } else {
        // We can't use 1GiB pages. Fall back to 2MiB pages.

        printf("1GiB pages are not supported. Falling back to 2MiB pages.\r\n");

        if(MaxMemory > (1ULL << 48)) {
            printf("RAM will be limited to 256TB. Page tables will occupy 1GiB of space.\r\nHowever, that is only 1/500000 of the available space.\r\n");
        }

        size_t MaxPML4 = 1;
        size_t MaxPDP = 512;
        size_t MaxPD = 512;
        size_t LastPDPEntry = 1;

        while(MaxMemory > (512ULL << 30)) {
            MaxPML4++;
            MaxMemory -= (512ULL << 30);
        }

        if(MaxPML4 > 512) {
            MaxPML4 = 512;
        }

        if(MaxMemory) {
            LastPDPEntry = ((MaxMemory + ((1 << 30) - 1)) & (~0ULL << 30)) > 30;

            if(LastPDPEntry > 512) {
                LastPDPEntry = 512;
            }
        }

        size_t PDPSize = PAGETABLE_SIZE * MaxPML4;
        size_t PDSize = PDPSize * MaxPDP;

        EFI_PHYSICAL_ADDRESS PDPBase = AllocatePagetable(PDPSize + PDSize);
        EFI_PHYSICAL_ADDRESS PDBase = PDPBase + PDSize;

        for(size_t PML4Entry = 0; PML4Entry < MaxPML4; PML4Entry++) {
            FirstPageTable[PML4Entry] = PDBase + (PML4Entry << 12);

            if(PML4Entry == (MaxPML4 - 1)) {
                MaxPDP = LastPDPEntry;
            }

            for(size_t PDPEntry = 0; PDPEntry < MaxPDP; PDPEntry++) {
                ((size_t* )FirstPageTable[PML4Entry])[PDPEntry] = PDBase + (((PML4Entry << 9) + PDPEntry) << 12);

                for(size_t PDEntry = 0; PDEntry < MaxPD; PDEntry++) {
                    ((size_t* )((size_t* )FirstPageTable[PML4Entry])[PDPEntry])[PDEntry] == (((PML4Entry << 18) + (PDPEntry << 9) + PDEntry) << 21) | 0x83;
                }

                ((size_t* )FirstPageTable[PML4Entry])[PDPEntry] |= 0x3;
            }

            FirstPageTable[PML4Entry] |= 0x3;
        }
    }

    WriteControlRegister(3, (size_t)FirstPageTable);
    // Hyper-V has an issue with this line.
    // TODO: Look into this.

    // Now that we've set the page table, we can re-enable Page Global.

    CR4 = ReadControlRegister(4);

    if(!(CR4 & (1 << 7))) {
        WriteControlRegister(4, CR4 ^ (1 << 7));

        if(ReadControlRegister(4) == CR4) {
            printf("Error setting CR4.PGE.");
        }
    }
}


// Gets the name of the processor.
char* FetchBrandStr(uint32_t* String) {
    size_t RAX = 0, RBX = 0, RCX = 0, RDX = 0;

    // This is done using our old friend CPUID.
    // This clobbers every register, so we need to use them all.
    __asm__ __volatile__("cpuid" : "=a" (RAX), "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (0x80000000) :);
    
    // From the Wiki article on CPUID: It is necessary to check whether the feature is present in the CPU by issuing CPUID with EAX = 80000000h first and checking if the returned value is greater or equal to 80000004h.
    if(RAX >= 0x80000004) {
        // To get the full 48-byte string, we need to call CPUID with:
        // 80000002
        // 80000003
        // 80000004
        // In sequence.

        __asm__ __volatile__("cpuid" : "=a" (RAX), "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (0x8000002) :);

        BrandStr[0] = ((uint32_t*) &RAX)[0];
        BrandStr[1] = ((uint32_t*) &RBX)[0];
        BrandStr[2] = ((uint32_t*) &RCX)[0];
        BrandStr[3] = ((uint32_t*) &RDX)[0];

        __asm__ __volatile__("cpuid" : "=a" (RAX), "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (0x8000003) :);
        
        BrandStr[4] = ((uint32_t*) &RAX)[0];
        BrandStr[5] = ((uint32_t*) &RBX)[0];
        BrandStr[6] = ((uint32_t*) &RCX)[0];
        BrandStr[7] = ((uint32_t*) &RDX)[0];

        __asm__ __volatile__("cpuid" : "=a" (RAX), "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (0x8000004) :);
        
        BrandStr[8] = ((uint32_t*) &RAX)[0];
        BrandStr[9] = ((uint32_t*) &RBX)[0];
        BrandStr[10] = ((uint32_t*) &RCX)[0];
        BrandStr[11] = ((uint32_t*) &RDX)[0];

        return (char* )BrandStr;
    } else {
        // Brand String not supported.
        // TODO: Maybe some tests to try to figure out the processor manually?
        printf("BrandStr not supported by the processor.\r\n");
        return NULL;
    }
}

/* 
The following are known processor manufacturer ID strings:

"AMDisbetter!" – early engineering samples of AMD K5 processor
"AuthenticAMD" – AMD
"CentaurHauls" – Centaur (Including some VIA CPU)
"CyrixInstead" – Cyrix
"HygonGenuine" – Hygon
"GenuineIntel" – Intel
"TransmetaCPU" – Transmeta
"GenuineTMx86" – Transmeta
"Geode by NSC" – National Semiconductor
"NexGenDriven" – NexGen
"RiseRiseRise" – Rise
"SiS SiS SiS " – SiS
"UMC UMC UMC " – UMC
"VIA VIA VIA " – VIA
"Vortex86 SoC" – Vortex

The following are known ID strings from virtual machines:

"bhyve bhyve " – bhyve
"KVMKVMKVM" – KVM
"Microsoft Hv" – Microsoft Hyper-V or Windows Virtual PC
" lrpepyh vr" – Parallels (it possibly should be "prl hyperv ", but it is encoded as " lrpepyh vr" due to an endianness mismatch)
"VMwareVMware" – VMware
"XenVMMXenVMM" – Xen HVM
"ACRNACRNACRN" - Project ACRN */

char* FetchManufacturer(char* ManufacturerStr) {
    size_t RBX = 0, RCX = 0, RDX = 0;

    // The manufacturer string is in the first leaf.
    __asm__ __volatile__("cpuid" : "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (0) :);
    
    ManufacturerStr[0] = ((char* )&RBX)[0];
    ManufacturerStr[1] = ((char* )&RBX)[1];
    ManufacturerStr[2] = ((char* )&RBX)[2];
    ManufacturerStr[3] = ((char* )&RBX)[3];

    ManufacturerStr[4] = ((char* )&RDX)[0];
    ManufacturerStr[5] = ((char* )&RDX)[1];
    ManufacturerStr[6] = ((char* )&RDX)[2];
    ManufacturerStr[7] = ((char* )&RDX)[3];

    ManufacturerStr[8] = ((char* )&RCX)[0];
    ManufacturerStr[9] = ((char* )&RCX)[1];
    ManufacturerStr[10] = ((char* )&RCX)[2];
    ManufacturerStr[11] = ((char* )&RCX)[3];

    ManufacturerStr[12] = '\0';

    return ManufacturerStr;
}

void ScanCPUFeatures(size_t RAXIn, size_t RCXIn) {
    size_t RAX = 0, RBX = 0, RCX = 0, RDX = 0;

    if(RAXIn == 1) {
        // Scan CPU Features (duh)
        // This should be the default
        __asm__ __volatile__("cpuid" : "=a" (RAX), "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (1) :);

        if(RCX & (1 << 31)) {
            printf("Sync is being run in a Hypervisor.\r\n");
        }
        if(RCX & (1 << 12)) {
            printf("Processor supports FMA.");
        } else {
            printf("Processor does not support FMA.\r\n");
        }

        if(RCX & 1) {
            if(RCX & (1 << 25)) { 
                printf("AESNI + PCLMULQDQ supported.\r\n");
            } else {
                printf("PCLMULQDQ supported, but not AESNI.\r\n");
            }
        }

        AVXStub();

        if(RCX & (1 << 29)) {
            printf("F16C supported.\r\n");
        }

        if(RDX & (1 << 22)) {
            printf("ACPI via MSR supported.\r\n");
        } else {
            printf("ACPI via MSR not supported.\r\n");
        }

        if(RDX & (1 << 24)) {
            printf("FXSR supported.\r\n");
        }
    } else if(RAXIn == 7 && RCXIn == 0) {
        // AVX Features

        AVXStub();

    } else if(RAXIn == 0x80000000) {
        // Processor Brand String
        char* Brand[48] = {0};
        FetchBrandStr(&Brand);
        printf("Processor Brand: %.48sr\\n", Brand);
    } else if(RAXIn == 0x8000001) {
        // Paging features

        __asm__ __volatile__("cpuid" : "=a" (RAX), "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (RAXIn) :);

        if(RDX & (1 << 26)) {
            printf("1GiB pages supported.\r\n");
        } else {
            printf("1GiB pages are not supported.\r\n");
        }

        if(RDX & (1 << 29)) {
            printf("Long Mode is supported.\r\n");
        }
    } else {
        // Just do what is asked of us.
        __asm__ __volatile__("cpuid" : "=a" (RAX), "=b" (RBX), "=c" (RCX), "=d" (RDX) : "a" (RAXIn) :);

        printf("rax: %#qx\r\nrbx: %#qx\r\nrcx: %#qx\r\nrdx: %#qx\r\n", RAX, RBX, RCX, RDX);
    }
}