/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/* ==================== Sync Memory Management ==================== */
/* Bear with me.
 * The plan for this file is to contain all of the memory management, as you can probably tell.
 * That means alloc, free, move, set, and AVX of all the above.
 * 
 * Also, this system will be paged. That means virtual address management.
 * That means a lot of work, and a lot of commenting.
 * 
 * TODO: The above.
 */

#include <kernel.h>

// malloc. Allocates relative to the nearest alignment value.

__attribute__((malloc)) void* kalloc(size_t Bytes) {
    if(Bytes <= 16) {
        return kalloc_16(Bytes);
    } else if(Bytes <= 32) {
        return kalloc_32(Bytes);
    } else if(Bytes < 4096) {
        return kalloc_64(Bytes);
    } else {
        return kalloc_pages(EFI_SIZE_TO_PAGES(Bytes));
    }
}

// Alignment allocation

__attribute__((malloc)) void* kalloc_16(size_t Bytes) {
    EFI_PHYSICAL_ADDRESS Buffer = AllocateFreeAddress_By16Bytes(Bytes, Buffer);
    return (void*)Buffer;
}

__attribute__((malloc)) void* kalloc_32(size_t Bytes) {
    EFI_PHYSICAL_ADDRESS Buffer = AllocateFreeAddress_By32Bytes(Bytes, Buffer);
    return (void*)Buffer;
}

__attribute__((malloc)) void* kalloc_64(size_t Bytes) {
    EFI_PHYSICAL_ADDRESS Buffer = AllocateFreeAddress_By64Bytes(Bytes, Buffer);
    return (void*)Buffer;
}

__attribute__((malloc)) void* kalloc_pages(size_t Pages) {
    EFI_PHYSICAL_ADDRESS Buffer = AllocateFreeAddress_ByPage(Pages, Buffer);
    return (void*)Buffer;
}

uint8_t VerifyZero(size_t Length, size_t Base) {
    for(size_t i = 0; i < Length; i++) {
        if ( *(uint8_t* )(Base + i) != 0) {
            return 1;
        }
    }

    return 0;
}

// Returns the byte after the last mapped byte of memory.
// AKA, tells you exactly how much RAM is installed.

size_t FetchMemoryLimit() {
    EFI_MEMORY_DESCRIPTOR* Piece;
    size_t CurrentAddress = 0, MaxAddress = 0;

    for(Piece = Memory_Info.MemoryMap;
         Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*) Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
         Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize)) {
        
        CurrentAddress = Piece->PhysicalStart + EFI_PAGES_TO_SIZE(Piece->NumberOfPages);
        if(CurrentAddress > MaxAddress) {
            MaxAddress = CurrentAddress;
        }
    }

    return MaxAddress;
}

// Calculates how much RAM is visible to the kernel (EG. Not taken by GPU or UEFI)
size_t FetchVisibleMemory() {
    EFI_MEMORY_DESCRIPTOR* Piece;
    size_t Total = 0;

    for(Piece = Memory_Info.MemoryMap;
     Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*) Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
     Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize)) {
        if( (Piece->Type != EfiMemoryMappedIO) &&
            (Piece->Type != EfiMemoryMappedIOPortSpace) &&
            (Piece->Type != EfiPalCode) &&
            (Piece->Type != EfiPersistentMemory) &&
            (Piece->Type != EfiMaxMemoryType)) {
            
            Total += EFI_PAGES_TO_SIZE(Piece->NumberOfPages);
        }
    }

    return Total;
}

 // Calulcate the total EfiConventionalMemory
size_t FetchAvailableMemory() {
    EFI_MEMORY_DESCRIPTOR* Piece;
    size_t Total = 0;

    for(Piece = Memory_Info.MemoryMap;
     Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*) Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
     Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize)) {
        if(Piece->Type == EfiConventionalMemory) {
            Total += EFI_PAGES_TO_SIZE(Piece->NumberOfPages);
        }
    }

    return Total;
}

// Calculates the total EfiPersistentMemory
size_t FetchAvailablePMemory() {
    EFI_MEMORY_DESCRIPTOR* Piece;
    size_t Total;

    for(Piece = Memory_Info.MemoryMap;
     Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*) Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
     Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize)) {
        if(Piece->Type == EfiPersistentMemory) {
            Total += EFI_PAGES_TO_SIZE(Piece->NumberOfPages);
        }
    }

    return Total;
}

size_t FetchInstalledMemory() {
    size_t Total = FetchVisibleMemory();
    Total += (63 << 20); // DDR3 min is 64MB

    return (Total & ~((64 << 20) - 1));
}

// From Syncboot/memory.c
static const char Memory_Segments[20][27] = {
	"EfiReservedMemoryType     ",
	"EfiLoaderCode             ",
	"EfiLoaderData             ",
	"EfiBootServicesCode       ",
	"EfiBootServicesData       ",
	"EfiRuntimeServicesCode    ",
	"EfiRuntimeServicesData    ",
	"EfiConventionalMemory     ",
	"EfiUnusableMemory         ",
	"EfiACPIReclaimMemory      ",
	"EfiACPIMemoryNVS          ",
	"EfiMemoryMappedIO         ",
	"EfiMemoryMappedIOPortSpace",
	"EfiPalCode                ",
	"EfiPersistentMemory       ",
	"EfiMaxMemoryType          ",
    "malloc                    ", // EfiMaxMemoryType + 1
    "vmalloc                   ", // EfiMaxMemoryType + 2
    "MemMap                    ", // EfiMaxMemoryType + 3
    "PageTables                "  // EfiMaxMemoryType + 4
};

void PrintMemoryMap() {
	EFI_MEMORY_DESCRIPTOR* Piece;

	uint16_t line = 0;



	printf("MemMapSize: %qx, MemMapDescriptorSize: %1u, MemMapDescriptorVersion: %u\r\n", Memory_Info.MemoryMapSize, Memory_Info.MemoryMapDescriptorSize, Memory_Info.MemoryMapDescriptorVersion);
	
	for (Piece = Memory_Info.MemoryMap; 
            Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
            Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {

		if (line % 20 == 0) {
			printf("#   Memory Type                Phys Addr Start   Num Of Pages   Attr\r\n");
		}

		printf("%2hu: %s 0x%016qx 0x%qx 0x%qx\r\n", line, Memory_Segments[Piece->Type], Piece->PhysicalStart, Piece->NumberOfPages, Piece->Attribute);
		line++;
	}
}

EFI_MEMORY_DESCRIPTOR* SetIdentityMap(EFI_RUNTIME_SERVICES* RT) {
    EFI_MEMORY_DESCRIPTOR* Piece;

	for (Piece = Memory_Info.MemoryMap; 
            Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
            Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
                Piece->VirtualStart = Piece->PhysicalStart;
    }

    if(EFI_ERROR(RT->SetVirtualAddressMap(Memory_Info.MemoryMapSize, Memory_Info.MemoryMapDescriptorSize, Memory_Info.MemoryMapDescriptorVersion, Memory_Info.MemoryMap))) {
        return NULL;
    }

    return Memory_Info.MemoryMap;
}


// This installs the memory map into itself.
// Sounds crazy, but it works.
void InstallMemoryMap() {
    EFI_MEMORY_DESCRIPTOR* Piece;
    size_t Pages = EFI_SIZE_TO_PAGES(Memory_Info.MemoryMapSize + Memory_Info.MemoryMapDescriptorSize);

    EFI_PHYSICAL_ADDRESS NewMapBase = FindFreeAddress(Pages, 0);

    if(NewMapBase == ~0ULL) {
        printf("Can't move memory map to embiggen it. Ergo, kalloc is not available.\r\n");
    } else {
        // Start moving.
        EFI_MEMORY_DESCRIPTOR* NewMap = (EFI_MEMORY_DESCRIPTOR* )NewMapBase;
        // Clear out the space for the new map.
        memsetAVX(NewMap, 0, Pages << EFI_PAGE_SHIFT);
        // Move the old map to where the new one is.
        memmoveAVX(NewMap, Memory_Info.MemoryMap, Memory_Info.MemoryMapSize);
        // Remove the old map.
        memsetAVX(Memory_Info.MemoryMap, 0, Memory_Info.MemoryMapSize);
        // Set the global map to the new one.
        Memory_Info.MemoryMap = NewMap;

        // Start expanding.
        
        // First, find the PhysicalStart place.
	    for (Piece = Memory_Info.MemoryMap; 
                Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
                Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
            
            if(Piece->PhysicalStart == NewMapBase) {
                break;
            }
        }

        if((uint8_t*) Piece == ((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize)) {
            printf("Memory Map not found.\r\n");
        } else {
             // Piece now contains PhysicalStart
             // Mark the new area as containing the memory map.
            if(Piece->NumberOfPages == Pages) {
                Piece->Type = EfiMaxMemoryType + 3; // Memory Map
            } else {
                // We need to tell the memory map that there's now a section *in* the memory map *for* the memory map.
                // so, temporarily store the current Piece.
                EFI_MEMORY_DESCRIPTOR NewDescriptor;
                NewDescriptor.Type = EfiMaxMemoryType + 3;
                NewDescriptor.Pad = Piece->Pad;
                NewDescriptor.PhysicalStart = Piece->PhysicalStart;
                NewDescriptor.VirtualStart = Piece->VirtualStart;
                NewDescriptor.NumberOfPages = Pages;
                NewDescriptor.Attribute = Piece->Attribute;

                // Shrink the descriptor; ew've added things.
                Piece->PhysicalStart += (Pages << EFI_PAGE_SHIFT);
                Piece->VirtualStart += (Pages << EFI_PAGE_SHIFT);
                Piece->NumberOfPages -= Pages;
                // Move things about
                memmoveAVX((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize, Piece, ((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize) - (uint8_t*)Piece);
                // And move the old piece back
                Piece->Type = NewDescriptor.Type;
                Piece->Pad = NewDescriptor.Pad;
                Piece->PhysicalStart = NewDescriptor.PhysicalStart;
                Piece->VirtualStart = NewDescriptor.VirtualStart;
                Piece->NumberOfPages = NewDescriptor.NumberOfPages;
                Piece->Attribute = NewDescriptor.Attribute;
                
                // We added a descriptor, so tell the map that it's now bigger.
                Memory_Info.MemoryMapSize += Memory_Info.MemoryMapDescriptorSize;
            }
        }
    }
}

EFI_PHYSICAL_ADDRESS AllocatePagetable(size_t PageTableSize) {
    EFI_MEMORY_DESCRIPTOR* Piece;
    size_t Pages = EFI_SIZE_TO_PAGES(PageTableSize);

    EFI_PHYSICAL_ADDRESS PagetableAddress = FindFreeAddress(Pages, 0);

    if(PagetableAddress == ~0ULL) {
        printf("Not enough space for page tables.\r\n");
        panic();
    } else {
        memsetAVX((void*)PagetableAddress, 0, Pages << EFI_PAGE_SHIFT);
	    for (Piece = Memory_Info.MemoryMap; 
                Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
                Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
            if(Piece->PhysicalStart == PagetableAddress) {
                break;
            }
        }

        if((uint8_t* )Piece == ((uint8_t* )Memory_Info.MemoryMap + Memory_Info.MemoryMapSize)) {
            printf("Pagetable space not found.\r\n");
            panic();
        } else {
            if(Piece->NumberOfPages == Pages) {
                Piece->Type = EfiMaxMemoryType + 4; // Pagetables
            } else {
                // We need to tell the memory map that there's now a section *in* the memory map *for* the memory map.
                // so, temporarily store the current Piece.
                EFI_MEMORY_DESCRIPTOR NewDescriptor;
                NewDescriptor.Type = EfiMaxMemoryType + 4;
                NewDescriptor.Pad = Piece->Pad;
                NewDescriptor.PhysicalStart = Piece->PhysicalStart;
                NewDescriptor.VirtualStart = Piece->VirtualStart;
                NewDescriptor.NumberOfPages = Pages;
                NewDescriptor.Attribute = Piece->Attribute;

                // Shrink the descriptor; ew've added things.
                Piece->PhysicalStart += (Pages << EFI_PAGE_SHIFT);
                Piece->VirtualStart += (Pages << EFI_PAGE_SHIFT);
                Piece->NumberOfPages -= Pages;
                // Move things about
                memmoveAVX((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize, Piece, ((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize) - (uint8_t*)Piece);
                // And move the old piece back
                Piece->Type = NewDescriptor.Type;
                Piece->Pad = NewDescriptor.Pad;
                Piece->PhysicalStart = NewDescriptor.PhysicalStart;
                Piece->VirtualStart = NewDescriptor.VirtualStart;
                Piece->NumberOfPages = NewDescriptor.NumberOfPages;
                Piece->Attribute = NewDescriptor.Attribute;
                
                // We added a descriptor, so tell the map that it's now bigger.
                Memory_Info.MemoryMapSize += Memory_Info.MemoryMapDescriptorSize;
            }
        }
    }

    return PagetableAddress;
}

EFI_PHYSICAL_ADDRESS FindFreeAddress(size_t pages, EFI_PHYSICAL_ADDRESS OldAddress) {

	EFI_MEMORY_DESCRIPTOR* Piece;

	for (Piece = Memory_Info.MemoryMap;
         Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
         Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
		if ((Piece->Type == EfiConventionalMemory) && (Piece->NumberOfPages >= pages) && (Piece->PhysicalStart > OldAddress))
			break;
	}

	if (Piece >= (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize)) {
        printf("Out of available memory!\r\n");
		return ~0ULL;
    }
    
	return Piece->PhysicalStart;
}

EFI_PHYSICAL_ADDRESS FindFreeAddress_ByPage(size_t pages, EFI_PHYSICAL_ADDRESS OldAddress) {
	EFI_MEMORY_DESCRIPTOR* Piece;
	EFI_PHYSICAL_ADDRESS PhysicalEnd;
	EFI_PHYSICAL_ADDRESS DiscoveredAddress;

	for (Piece = Memory_Info.MemoryMap;
         Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
         Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize)) {

		if ((Piece->Type == EfiConventionalMemory) && (Piece->NumberOfPages >= pages)) {
			PhysicalEnd = Piece->PhysicalStart + (Piece->NumberOfPages << EFI_PAGE_SHIFT) - EFI_PAGE_MASK; // Get the end of this range, and use it to set a bound on the range (define a max returnable address).
			if ((OldAddress >= Piece->PhysicalStart) && ((OldAddress + (pages << EFI_PAGE_SHIFT)) < PhysicalEnd)) {
				DiscoveredAddress = OldAddress + EFI_PAGE_SIZE; 
				break;
			}
			else if (Piece->PhysicalStart > OldAddress) {
				DiscoveredAddress = Piece->PhysicalStart;
				break;
			}
		}
	}

	if (Piece >= (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize)) {
		printf(L"No more free addresses by page...\r\n");
		return ~0ULL;
	}

	return DiscoveredAddress;
}

EFI_PHYSICAL_ADDRESS AllocateFreeAddress_By16Bytes(size_t Bytes, EFI_PHYSICAL_ADDRESS OldAddress) {
  EFI_MEMORY_DESCRIPTOR* Piece;
  EFI_PHYSICAL_ADDRESS PhysicalEnd;
  EFI_PHYSICAL_ADDRESS DiscoveredAddress;
  size_t X16Bytes = Bytes >> 4;
  if(Bytes & 0xF) { 
    X16Bytes++;
  }

  for(Piece = Memory_Info.MemoryMap;
      Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
      Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize)) {

    if((Piece->Type == EfiConventionalMemory) && ((Piece->NumberOfPages << EFI_PAGE_SHIFT) >= X16Bytes)) {
      // Found a range big enough.
      // Get the end of this range for bounds checking
      PhysicalEnd = Piece->PhysicalStart + (Piece->NumberOfPages << EFI_PAGE_SHIFT) - 1;

      if((OldAddress >= Piece->PhysicalStart) && ((OldAddress + (X16Bytes << 4)) < PhysicalEnd)) { // Bounds check on OldAddress
        // OldAddress + offset is [still] in-bounds, so return the next available x-byte aligned address in the range.
        DiscoveredAddress = OldAddress + X16Bytes; // Left shift num_x_bytes by 1 or 2 to check every 0x10 or 0x100 sets of bytes (must also modify the above PhysicalEnd bound check)
        break;
        // If we would run over PhysicalEnd, we need to go to the next EfiConventionalMemory range
      }
      else if(Piece->PhysicalStart > OldAddress) { // Turns out the nearest compatible PhysicalStart is > OldAddress. Use that, then.
        DiscoveredAddress = Piece->PhysicalStart;
        break;
      }
    }
  }

  // Loop ended without a DiscoveredAddress
  if(Piece >= (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize))
  {
    // Return address -1, which will cause AllocatePages to fail
    printf("No more free physical addresses by 16 bytes...\r\n");
    return ~0ULL;
  }

  return DiscoveredAddress;
}



EFI_PHYSICAL_ADDRESS AllocateFreeAddress_By32Bytes(size_t Bytes, EFI_PHYSICAL_ADDRESS OldAddress) {
  EFI_MEMORY_DESCRIPTOR* Piece;
  EFI_PHYSICAL_ADDRESS PhysicalEnd;
  EFI_PHYSICAL_ADDRESS DiscoveredAddress;
  size_t X32Bytes = Bytes >> 5;
  if(Bytes & 0x1F) { 
    X32Bytes++;
  }

  for(Piece = Memory_Info.MemoryMap;
      Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
      Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize)) {

    if((Piece->Type == EfiConventionalMemory) && ((Piece->NumberOfPages << EFI_PAGE_SHIFT) >= X32Bytes)) {
      // Found a range big enough.
      // Get the end of this range for bounds checking
      PhysicalEnd = Piece->PhysicalStart + (Piece->NumberOfPages << EFI_PAGE_SHIFT) - 1;

      if((OldAddress >= Piece->PhysicalStart) && ((OldAddress + (X32Bytes << 5)) < PhysicalEnd)) { // Bounds check on OldAddress
        // OldAddress + offset is [still] in-bounds, so return the next available x-byte aligned address in the range.
        DiscoveredAddress = OldAddress + X32Bytes; // Left shift num_x_bytes by 1 or 2 to check every 0x10 or 0x100 sets of bytes (must also modify the above PhysicalEnd bound check)
        break;
        // If we would run over PhysicalEnd, we need to go to the next EfiConventionalMemory range
      }
      else if(Piece->PhysicalStart > OldAddress) { // Turns out the nearest compatible PhysicalStart is > OldAddress. Use that, then.
        DiscoveredAddress = Piece->PhysicalStart;
        break;
      }
    }
  }

  // Loop ended without a DiscoveredAddress
  if(Piece >= (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize))
  {
    // Return address -1, which will cause AllocatePages to fail
    printf("No more free physical addresses by 32 bytes...\r\n");
    return ~0ULL;
  }

  return DiscoveredAddress;
}


EFI_PHYSICAL_ADDRESS AllocateFreeAddress_By64Bytes(size_t Bytes, EFI_PHYSICAL_ADDRESS OldAddress) {
  EFI_MEMORY_DESCRIPTOR* Piece;
  EFI_PHYSICAL_ADDRESS PhysicalEnd;
  EFI_PHYSICAL_ADDRESS DiscoveredAddress;
  size_t X64Bytes = Bytes >> 6;
  if(Bytes & 0x3F) { 
    X64Bytes++;
  }

  for(Piece = Memory_Info.MemoryMap;
      Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
      Piece = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize)) {

    if((Piece->Type == EfiConventionalMemory) && ((Piece->NumberOfPages << EFI_PAGE_SHIFT) >= X64Bytes)) {
      // Found a range big enough.
      // Get the end of this range for bounds checking
      PhysicalEnd = Piece->PhysicalStart + (Piece->NumberOfPages << EFI_PAGE_SHIFT) - 1;

      if((OldAddress >= Piece->PhysicalStart) && ((OldAddress + (X64Bytes << 6)) < PhysicalEnd)) { // Bounds check on OldAddress
        // OldAddress + offset is [still] in-bounds, so return the next available x-byte aligned address in the range.
        DiscoveredAddress = OldAddress + X64Bytes; // Left shift num_x_bytes by 1 or 2 to check every 0x10 or 0x100 sets of bytes (must also modify the above PhysicalEnd bound check)
        break;
        // If we would run over PhysicalEnd, we need to go to the next EfiConventionalMemory range
      }
      else if(Piece->PhysicalStart > OldAddress) { // Turns out the nearest compatible PhysicalStart is > OldAddress. Use that, then.
        DiscoveredAddress = Piece->PhysicalStart;
        break;
      }
    }
  }

  // Loop ended without a DiscoveredAddress
  if(Piece >= (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize))
  {
    // Return address -1, which will cause AllocatePages to fail
    printf("No more free physical addresses by 16 bytes...\r\n");
    return ~0ULL;
  }

  return DiscoveredAddress;
}

// per the UEFI Specification (2.7A), EfiBootServicesCode and EfiBootServicesData should be free.
// This function is safe, meaning that calling it more than once doesn't change anything.

void ReclaimEfiBootServicesMemory() {
    EFI_MEMORY_DESCRIPTOR* Piece;

	for (Piece = Memory_Info.MemoryMap; 
            Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
            Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
        if((Piece->Type == EfiBootServicesCode) || (Piece->Type == EfiBootServicesData)) {
            Piece->Type = EfiConventionalMemory;
        }
    }

    MergeFragmentedMemory();
}

// This function does the above, but for EfiLoaderData (Syncboot data)
// This is not recommended, as this stored the FILELOADER_PARAMS.
// Before calling this, make a copy.

void ReclaimEfiLoaderCodeMemory() {
    EFI_MEMORY_DESCRIPTOR* Piece;

	for (Piece = Memory_Info.MemoryMap; 
            Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
            Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
        if(Piece->Type == EfiLoaderData) {
            Piece->Type = EfiConventionalMemory;
        }
    }

    MergeFragmentedMemory();
}


// Cleans up the memory map, merging adjacent EfiConventionalMemory entries.
void MergeFragmentedMemory() {

    EFI_MEMORY_DESCRIPTOR* Piece;
    EFI_MEMORY_DESCRIPTOR* Piece2;
    EFI_PHYSICAL_ADDRESS PhysicalEnd;
    size_t Pages = 1;


	for (Piece = Memory_Info.MemoryMap; 
            Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
            Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
        if(Piece->Type == EfiConventionalMemory) {

            PhysicalEnd = Piece->PhysicalStart + (Piece->NumberOfPages << EFI_PAGE_SHIFT);

        
	        for (Piece2 = Memory_Info.MemoryMap; 
                    Piece2 < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
                    Piece2 = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece2 + Memory_Info.MemoryMapDescriptorSize)) {
                    
                if((Piece2->Type == EfiConventionalMemory) && (PhysicalEnd == Piece2->PhysicalStart)) { // If this segment is adjacent to the last one (Piece)
                    Piece->NumberOfPages += Piece2->NumberOfPages;

                    memsetAVX(Piece2, 0, Memory_Info.MemoryMapDescriptorSize);
                    // Move the memory down a page
                    memmoveAVX(Piece2, (uint8_t* )Piece2 + Memory_Info.MemoryMapDescriptorSize, ((uint8_t* )Memory_Info.MemoryMap + Memory_Info.MemoryMapSize) - ((uint8_t* )Piece2 + Memory_Info.MemoryMapDescriptorSize));

                    Memory_Info.MemoryMapSize -= Memory_Info.MemoryMapDescriptorSize;

                    memsetAVX((uint8_t* )Memory_Info.MemoryMap + Memory_Info.MemoryMapSize, 0, Memory_Info.MemoryMapDescriptorSize);

                    Piece2 = (EFI_MEMORY_DESCRIPTOR* )((uint8_t* )Piece2 - Memory_Info.MemoryMapDescriptorSize);

                    PhysicalEnd = Piece->PhysicalStart + (Piece->NumberOfPages << EFI_PAGE_SHIFT);
                }
            }
        } else if(Piece->Type == EfiMaxMemoryType + 3 ) {
            Pages = Piece->NumberOfPages;
        }
    } 

    size_t Pages2 = (Memory_Info.MemoryMapSize + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;

    if(Pages2 < Pages) {
    
	    for (Piece = Memory_Info.MemoryMap; 
                Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
                Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
                
            if((Piece->Type == EfiMaxMemoryType + 3)) {
                if( ((EFI_MEMORY_DESCRIPTOR* )((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize))->Type == EfiConventionalMemory ) {
                    size_t FreePages = Pages - Pages2;

                    Piece->NumberOfPages = Pages2;

                    ((EFI_MEMORY_DESCRIPTOR* )((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize))->NumberOfPages += FreePages;
                    ((EFI_MEMORY_DESCRIPTOR* )((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize))->PhysicalStart -= (FreePages << EFI_PAGE_SHIFT);
                    ((EFI_MEMORY_DESCRIPTOR* )((uint8_t* )Piece + Memory_Info.MemoryMapDescriptorSize))->VirtualStart -= (FreePages << EFI_PAGE_SHIFT);
                } else if ((Memory_Info.MemoryMapSize + Memory_Info.MemoryMapDescriptorSize) <= (Pages2 << EFI_PAGE_SHIFT)) {
                    
                    EFI_MEMORY_DESCRIPTOR NewDescriptor;
                    NewDescriptor.Type = Piece->Type;
                    NewDescriptor.Pad = Piece->Pad;
                    NewDescriptor.PhysicalStart = Piece->PhysicalStart;
                    NewDescriptor.VirtualStart = Piece->VirtualStart;
                    NewDescriptor.NumberOfPages = Pages2;
                    NewDescriptor.Attribute = Piece->Attribute;

                    Piece->Type = EfiConventionalMemory;

                    // Shrink the descriptor; ew've added things.
                    Piece->PhysicalStart += (Pages2 << EFI_PAGE_SHIFT);
                    Piece->VirtualStart += (Pages2 << EFI_PAGE_SHIFT);
                    Piece->NumberOfPages = Pages - Pages2;
                    // Move things about
                    memmoveAVX((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize, Piece, ((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize) - (uint8_t*)Piece);
                    // And move the old piece back
                    Piece->Type = NewDescriptor.Type;
                    Piece->Pad = NewDescriptor.Pad;
                    Piece->PhysicalStart = NewDescriptor.PhysicalStart;
                    Piece->VirtualStart = NewDescriptor.VirtualStart;
                    Piece->NumberOfPages = NewDescriptor.NumberOfPages;
                    Piece->Attribute = NewDescriptor.Attribute;

                    // We added a descriptor, so tell the map that it's now bigger.
                    Memory_Info.MemoryMapSize += Memory_Info.MemoryMapDescriptorSize;
                } else {
                    size_t PagesPerDescriptor = (Memory_Info.MemoryMapDescriptorSize + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT;

                    if((Pages2 + PagesPerDescriptor) < Pages) {
                        size_t FreePages = Pages - Pages2 - PagesPerDescriptor;
                        
                        EFI_MEMORY_DESCRIPTOR NewDescriptor;
                        NewDescriptor.Type = Piece->Type;
                        NewDescriptor.Pad = Piece->Pad;
                        NewDescriptor.PhysicalStart = Piece->PhysicalStart;
                        NewDescriptor.VirtualStart = Piece->VirtualStart;
                        NewDescriptor.NumberOfPages = Pages2 + PagesPerDescriptor;
                        NewDescriptor.Attribute = Piece->Attribute;

                        Piece->Type = EfiConventionalMemory;

                        // Shrink the descriptor; ew've added things.
                        Piece->PhysicalStart += ((Pages2 + PagesPerDescriptor) << EFI_PAGE_SHIFT);
                        Piece->VirtualStart += ((Pages2 + PagesPerDescriptor) << EFI_PAGE_SHIFT);
                        Piece->NumberOfPages = FreePages;
                        // Move things about
                        memmoveAVX((uint8_t*)Piece + Memory_Info.MemoryMapDescriptorSize, Piece, ((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize) - (uint8_t*)Piece);
                        // And move the old piece back
                        Piece->Type = NewDescriptor.Type;
                        Piece->Pad = NewDescriptor.Pad;
                        Piece->PhysicalStart = NewDescriptor.PhysicalStart;
                        Piece->VirtualStart = NewDescriptor.VirtualStart;
                        Piece->NumberOfPages = NewDescriptor.NumberOfPages;
                        Piece->Attribute = NewDescriptor.Attribute;

                        // We added a descriptor, so tell the map that it's now bigger.
                        Memory_Info.MemoryMapSize += Memory_Info.MemoryMapDescriptorSize;
                    }
                }
                break;
            }
        }
    }
}

EFI_PHYSICAL_ADDRESS PurgeAllMemory() {
    EFI_MEMORY_DESCRIPTOR* Piece;
    EFI_PHYSICAL_ADDRESS Exit = 0;

    
	for (Piece = Memory_Info.MemoryMap; 
            Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
            Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {

        if(Piece->Type == EfiConventionalMemory) {
            memsetAVX( (void* )Piece->PhysicalStart, 0, EFI_PAGES_TO_SIZE(Piece->NumberOfPages));
            Exit = VerifyZero(EFI_PAGES_TO_SIZE(Piece->NumberOfPages), Piece->PhysicalStart);
            if(Exit) {
                printf("Unable to clear memory. First populated address is %#qx. Memory Descriptor starts at %#qx.\r\n", Exit, Piece->PhysicalStart);
            } else {
                printf("Zeroed memory.\r\n");
            }
        }
    }

    return Exit;
}

EFI_PHYSICAL_ADDRESS AllocateFreeAddress_ByPage(size_t Pages, EFI_PHYSICAL_ADDRESS SearchStart) {
    EFI_MEMORY_DESCRIPTOR* Piece;
    EFI_PHYSICAL_ADDRESS End, Address;

	for (Piece = Memory_Info.MemoryMap; 
            Piece < (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)Memory_Info.MemoryMap + Memory_Info.MemoryMapSize);
            Piece = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)Piece + Memory_Info.MemoryMapDescriptorSize)) {
        if((Piece->Type == EfiConventionalMemory) && (Piece->NumberOfPages >= Pages)) {
            End = Piece->PhysicalStart + (Piece->NumberOfPages << EFI_PAGE_SHIFT) - 1;

            if((SearchStart >= Piece->PhysicalStart) && ((SearchStart + (Pages << EFI_PAGE_SHIFT)) << End)) {
                Address = SearchStart + EFI_PAGE_SIZE;
                break;
            } else if(Piece->PhysicalStart > SearchStart) {
                Address = Piece->PhysicalStart;

                if(Piece->NumberOfPages - Pages == 0) {
                    Piece->Type = EfiMaxMemoryType + 1;
                } 
                // TODO: Allocate a new page
                break;
            }
        }
    }

    if(Piece >= (EFI_MEMORY_DESCRIPTOR* )((uint8_t* )Memory_Info.MemoryMap + Memory_Info.MemoryMapSize)) {
        printf("Out of pages!\r\n");
        return ~0ULL;
    }

    return Address;
}

// Here we go.
// This is going to be a mess.
// This abuses AVX (SSE3) to do simultaneous memory operations.
// I expect this to be useful in graphics - we can move up to 16 pixels with one - one! CPU cycle.
