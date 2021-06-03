/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/* This file should do the work of bootloading the kernel.
 * Most of this work is taken from the Syncboot project, 
 * https://git.gemwire.uk/gwdev/Syncboot
 * 
 * All it does it set up graphics and transfer into our kernel.
 * At this stage the kernel is available for configuration, but
 *  this will require more testing before being properly implemented.
 */

#include <efiboot.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
	InitializeLib(ImageHandle, SystemTable); // Initialize EFILIB.
	/* Provides the following definitions:
		- ST = SystemTable
		- BS = SystemTable->BootServices
		- RT = SystemTable->RuntimeServices
	*/

    EFI_STATUS Status;
    size_t Mode;

	Print(L"\n%H========== Sync Bootloading ==========%N\r\n");

    /* Get the time */
	EFI_TIME Now;
	Status = RT->GetTime(&Now, NULL);
	if (EFI_ERROR(Status)) {
		Print(L"Error fetching time.\r\n");
    } else {
		/* and then print it. */
		Print(L"Time now is %02hhu/%02hhu/%04hu - %02hhu:%02hhu:%02hhu.%u\r\n\n", Now.Month, Now.Day, Now.Year, Now.Hour, Now.Minute, Now.Second, Now.Nanosecond);
    }

    Print(L"Press S for slow mode, D for debug mode, or nothing for blitz mode.\r\n");

    Status = WaitForSingleEvent(ST->ConIn->WaitForKey, 10000000);
    if(Status != EFI_TIMEOUT) {
        EFI_INPUT_KEY Key = { 0 };
        Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);
        if(!(EFI_ERROR(Status))) {
            switch(Key.UnicodeChar) {
                case L"s":
                    Mode = 1;
                    break;
                case L"d":
                    #define DEBUG
                    break;
                default: 
                    Mode = 0;
                    break;
            }

            Status = ST->ConIn->Reset(ST->ConIn, FALSE);
        }
    }

    GFX_INFO* Gfx;
    Status = BS->AllocatePool(EfiLoaderData, sizeof(GFX_INFO), (void**)&Gfx);
    if(EFI_ERROR(Status)) {
        Print(L"Error at Gfx AllocatePool\r\n");
    }

    Gfx->FBCount = 0;
    size_t GfxInfoSize, xGfxHandles, xNm2Handles, xDevPathHandles, DeviceInd;
    uint32_t GfxMode;
    EFI_INPUT_KEY Key;
    Key.UnicodeChar = 0;

    CHAR16* DriverDisplayName = DefaultDriverName;
    CHAR16* ControllerDisplayName = DefaultControllerName;
    CHAR16* ChildDisplayName = DefaultChildName;

    EFI_HANDLE* GfxHandles;

    Status = BS->LocateHandleBuffer(ByProtocol, &GraphicsOutputProtocol, NULL, &xGfxHandles, &GfxHandles);
    if(EFI_ERROR(Status)) {
        Print(L"Error at Gfx LocateHAndle\r\n");
    }

    if(xGfxHandles == 1) {
        Print(L"There is 1 GOP device.\r\n");
    } else {
        Print(L"There are %llu GOP devices.\r\n", xGfxHandles);
    }

    CHAR16** DeviceNames;
    Status = BS->AllocatePool(EfiBootServicesData, sizeof(CHAR16*)*xGfxHandles, (void**)&DeviceNames);
    if(EFI_ERROR(Status)) {
        Print(L"Error at Gfx NameBuffer AllocatePool\r\n");
    }

    EFI_HANDLE* Name2Handles;
    Status = BS->LocateHandleBuffer(ByProtocol, &ComponentName2Protocol, NULL, &xNm2Handles, &Name2Handles);
    if(EFI_ERROR(Status)) {
        Print(L"Error at Gfx Name2Handles LocateBuffer\r\n");
    }

    EFI_HANDLE* DevicePaths;
    Status = BS->LocateHandleBuffer(ByProtocol, &DevicePathProtocol, NULL, &xDevPathHandles, &DevicePaths);
	if (EFI_ERROR(Status)) {
		Print(L"DevicePathHandles LocateBuffer error\r\n");
	}

    for(DeviceInd = 0; DeviceInd < xGfxHandles; DeviceInd++) {
        DriverDisplayName = DefaultDriverName;
        ControllerDisplayName = DefaultControllerName;
        ChildDisplayName = DefaultChildName;

        EFI_DEVICE_PATH* GfxDevicePath;

        Status = BS->OpenProtocol(GfxHandles[DeviceInd], &DevicePathProtocol, (void**)&GfxDevicePath, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if(Status == EFI_SUCCESS) {
            size_t ControllerPathSize = DevicePathSize(GfxDevicePath) - DevicePathNodeLength(GfxDevicePath) + 4;
            EFI_DEVICE_PATH* CurDevicePath;
            size_t ControllerInd = 0;

            for(ControllerInd = 0; ControllerInd < xDevPathHandles; ControllerInd++) {
                Status = BS->OpenProtocol(DevicePaths[ControllerInd], &DriverBindingProtocol, NULL, NULL, NULL, EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
				if (!EFI_ERROR(Status))
					continue;

				Status = BS->OpenProtocol(DevicePaths[ControllerInd], &LoadedImageProtocol, NULL, NULL, NULL, EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
				if (!EFI_ERROR(Status))
					continue;

				// Graphics controllers don't have SimpleFileSystem, so we need to load a proper filesystem.
				Status = BS->OpenProtocol(DevicePaths[ControllerInd], &FileSystemProtocol, NULL, NULL, NULL, EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
				if (!EFI_ERROR(Status))
					continue;

				// I'll be honest, i don't know why. There are no PS/2 keyboard ports on UEFI2 compatible PCIe GPUs, but without this check it fails to boot.
				Status = BS->OpenProtocol(DevicePaths[ControllerInd], &SerialIoProtocol, NULL, NULL, NULL, EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
				if (!EFI_ERROR(Status))
					continue;

				Status = BS->OpenProtocol(DevicePaths[ControllerInd], &DevicePathProtocol, (void**)&CurDevicePath, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
				if (EFI_ERROR(Status)) {
					Print(L"DevicePathHandles OpenProtocol error\r\n");
				}

                size_t CurControllerPathSize = DevicePathSize(GfxDevicePath);

                if(CurControllerPathSize != ControllerPathSize)
                    continue;
                
                if(LibMatchDevicePaths(CurDevicePath, GfxDevicePath)) {
                    for(size_t Name2DriverIndex = 0; Name2DriverIndex < xNm2Handles; Name2DriverIndex++) {
                        void* ManagedInterface;
                        Status = BS->OpenProtocol(DevicePaths[ControllerInd], &PciIoProtocol, &ManagedInterface, Name2Handles[Name2DriverIndex]. DevicePaths[ControllerInd], EFI_OPEN_PROTOCOL_BY_DRIVER);
                        if(!EFI_ERROR(Status)) {
                            Status = BS->CloseProtocol(DevicePaths[ControllerInd], &PciIoProtocol, Name2Handles[Name2DriverIndex], DevicePaths[ControllerInd]);
                            if(EFI_ERROR(Status)) {
                                Print(L"DevicePaths Name2Handles CloseProtocol error\r\n");
                            }
                            continue;
                        } else if (Status != EFI_ALREADY_STARTED) {
                            continue;
                        }

                        EFI_COMPONENT_NAME2_PROTOCOL* Name2Device;

                        Status = BS->OpenProtocol(Name2Handles[Name2DriverIndex], &ComponentName2Protocol, (void**)&Name2Device, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
                        if(EFI_ERROR(Status)) {
                            Print(L"Name2Device OpenProtocol error\r\n");
                        }

                        Status = Name2Device->GetDriverName(Name2Device, Language, &DriverDisplayName);
						if (Status == EFI_UNSUPPORTED) { // Wrong language.
							Status = Name2Device->GetDriverName(Name2Device, Language2, &DriverDisplayName);
							if (Status == EFI_UNSUPPORTED) {
								Status = Name2Device->GetDriverName(Name2Device, Language3, &DriverDisplayName);
							}
						}

                        if (EFI_ERROR(Status)) {
							Print(L"Name2Device GetDriverName error\r\n");

							if (Status == EFI_UNSUPPORTED) { // None of our languages will work.
								Print(L"The first 10 characters of the supported language are:\r\n");
								for (uint32_t p = 0; p < 10; p++) {
									Print(L"%c", Name2Device->SupportedLanguages[p]);
								}
								Print(L"\r\n");
							}
							// The driver doesn't follow specifications if we get this far, so we give it the default.
							DriverDisplayName = DefaultDriverName;
						}

                        Status = Name2Device->GetControllerName(Name2Device, DevicePaths[ControllerInd], NULL, Language, &ControllerDisplayName); // The child should be NULL to get the controller's name.
						if (Status == EFI_UNSUPPORTED) {
							Status = Name2Device->GetControllerName(Name2Device, DevicePaths[ControllerInd], NULL, Language2, &ControllerDisplayName);
							if (Status == EFI_UNSUPPORTED) {
								Status = Name2Device->GetControllerName(Name2Device, DevicePaths[ControllerInd], NULL, Language3, &ControllerDisplayName);
							}
						}
                        if(EFI_ERROR(Status)) {
                            Print(L"Name2Device GetControllerName error\r\n");
                        }

                        Status = Name2Device->GetControllerName(Name2Device, DevicePaths[ControllerInd], GfxHandles[DeviceInd], Language, &ChildDisplayName);
						if (Status == EFI_UNSUPPORTED) {
							Status = Name2Device->GetControllerName(Name2Device, DevicePaths[ControllerInd], GfxHandles[DeviceInd], Language2, &ChildDisplayName);
							if (Status == EFI_UNSUPPORTED) {
								Status = Name2Device->GetControllerName(Name2Device, DevicePaths[ControllerInd], GfxHandles[DeviceInd], Language3, &ChildDisplayName);
								
							}
						}
						

						if (EFI_ERROR(Status)) {
                            Print(L"Name2Device GetControllerName error\r\n");

							if (Status == EFI_UNSUPPORTED) {
								Print(L"First 10 characters of the supported language:\r\n");
								for (uint32_t p = 0; p < 10; p++) {
									Print(L"%c", Name2Device->SupportedLanguages[p]);
								}
								Print(L"\r\n");
							}

							ChildDisplayName = DefaultChildName;
						}

                        break;
                    }
                    
                    break;
                }
            } // End of ControllerIndex


            if((ControllerDisplayName == DefaultControllerName) && (DriverDisplayName == DefaultDriverName) && (ChildDisplayName == DefaultChildName)) {
                // Filter controller index by DriverBinding, LoadedImage, FileSystem and SerialIO protocols.
            }
        } else if (Status == EFI_UNSUPPORTED) {
            Print(L"VM or BIOS graphics device found.\r\n");
        } else if (EFI_ERROR(Status)) {
            Print(L"GraphicsHandles GfxDevicePath error.\r\n");
        }
    }

    if(xGfxHandles > 1) {
        DeviceInd = 2;
        size_t Timeout = 5;

        Print(L"Graphics devices:\r\n");
        for(size_t DeviceNumber = 0; DeviceNumber < xGfxHandles; DeviceNumber++) {
            Print(L"%s", DeviceNames[DeviceNumber]);
        }
        Print(L"\r\n");

        if(Mode) {
            Print(L"%E==================== Graphics Configuration ==================== %N \r\n");
			Print(L"Choose an option: \r\n");
			Print(L"0. Configure each device individually\r\n");
			Print(L"1. Configure one device\r\n");
			Print(L"2. Configure all devices to use the native resolution of the device they're connected to \r\n");
			Print(L"3. Configure all devices to use a 1024x768 resolution.\r\n");
			Print(L"%ENote: Not all displays may be active. The ones which are, are determined by the GPU firmware. I have no control over them.%N\r\n");

            while(Timeout) {
                Print(L"Please select an option. Defaulting to %llu in %llu .\r", DeviceInd, Timeout);
				Status = WaitForSingleEvent(ST->ConIn->WaitForKey, 10000000);
				if (Status != EFI_TIMEOUT) {
					Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);
					if (EFI_ERROR(Status)) {

						Print(L"\nError reading keystroke. 0x%llx\r\n", Status);
						return Status;
					}

					Print(L"\n\nOption %c.\r\n\n", Key.UnicodeChar);
					break;
				}
				Timeout -= 1;

				if (!Timeout) {
					Print(L"Defaulting to option %llu.\r\n\n", DeviceInd);
					break;
				}
			}

			if (Timeout) {
				DeviceInd = (size_t)(Key.UnicodeChar - 0x30);
            }

			Key.UnicodeChar = 0;
			Status = ST->ConIn->Reset(ST->ConIn, FALSE);
			if (EFI_ERROR(Status)) {
				Print(L"Error resetting input buffer: 0x%llx\r\n", Status);
				return Status;
			}
        } else {
            // Blitz mode. Straight into mode 3.
            DeviceInd = 3;
        }

    }
    size_t GOPInfoSize;

    if((xGfxHandles > 1) && (DeviceInd == 0)) {
        Gfx->FBCount = xGfxHandles;

        Status = BS->AllocatePool(EfiLoaderData, Gfx->FBCount * sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE), (void**)&Gfx->GPUs);
        if(EFI_ERROR(Status)) {
            Print(L"GPUs AllocatePool error");
        }

        for(DeviceInd = 0; DeviceInd < xGfxHandles; DeviceInd++) {
            EFI_GRAPHICS_OUTPUT_PROTOCOL* GOPTable;

            Status = BS->OpenProtocol(GfxHandles[DeviceInd], &GraphicsOutputProtocol, (void**)&GOPTable, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
            if(EFI_ERROR(Status)) {
                Print(L"GraphicsTable OpenProtocol error");
            }

            if(GOPTable->Mode->MaxMode == 1) {
                GfxMode = 0;
            } else {
                EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* GOPInfo2;
				while (0x30 > Key.UnicodeChar || Key.UnicodeChar > (0x30 + GOPTable->Mode->MaxMode - 1)) {
					Print(L"%s", DeviceNames[DeviceInd]);
					Print(L"\r\n");
					Print(L"%u available graphics modes.\r\n\n", GOPTable->Mode->MaxMode);
					Print(L"Current mode: %c\r\n", GOPTable->Mode->Mode + 0x30);
					for (GfxMode = 0; GfxMode < GOPTable->Mode->MaxMode; GfxMode++) {
						Status = GOPTable->QueryMode(GOPTable, GfxMode, &GOPInfoSize, &GOPInfo2);
						if (EFI_ERROR(Status)) {
							Print(L"GraphicsTable QueryMode error: 0x%llx\r\n", Status);
							return Status;
						}
						Print(L"%c. %ux%u, Pixels Per SCNLN: &u, Pixel format: %s\r\n", GfxMode + 0x30, GOPInfo2->HorizontalResolution, GOPInfo2->VerticalResolution, GOPInfo2->PixelsPerScanLine, GOPInfo2->PixelFormat);
						Status = BS->FreePool(GOPInfo2);
						if (EFI_ERROR(Status)) {
							Print(L"Error freeing GOPInfo2: 0x%llx\r\n", Status);
							return Status;
						}
					}

					Print(L"\r\nPlease select a graphics mode. (0-%c)\r\n", 0x30 + GOPTable->Mode->MaxMode - 1);
					Status = ST->ConIn->Reset(ST->ConIn, NULL);
					//while (Key.UnicodeChar < 0x30 || Key.UnicodeChar - 0x30 < GOPTable->Mode->MaxMode) {
					Status = ST->ConIn->Reset(ST->ConIn, NULL);
					while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);
					//}
					Print(L"\r\nSelected graphics mode %c.\r\n", Key.UnicodeChar);
				}

				GfxMode = (uint32_t)(Key.UnicodeChar - 0x30);

				Print(L"Setting graphics mode %u.\r\n", GfxMode + 1);
			}

			Status = GOPTable->SetMode(GOPTable, GfxMode);
			if (EFI_ERROR(Status)) {
				Print(L"GraphicsTable SetMode error: 0x%llx\r\n", Status);
				return Status;
			}
			Status = BS->AllocatePool(EfiLoaderData, GOPTable->Mode->SizeOfInfo, (void**)&Gfx->GPUs[DeviceInd].Info);
			if (EFI_ERROR(Status)) {
				Print(L"GOPMode->Info AllocatePool error: 0x%llx\r\n", Status);
				return Status;
			}

			Gfx->GPUs[DeviceInd].MaxMode = GOPTable->Mode->MaxMode;
			Gfx->GPUs[DeviceInd].Mode = GOPTable->Mode->Mode;
			Gfx->GPUs[DeviceInd].SizeOfInfo = GOPTable->Mode->SizeOfInfo;
			Gfx->GPUs[DeviceInd].FrameBufferSize = GOPTable->Mode->FrameBufferSize;
			Gfx->GPUs[DeviceInd].FrameBufferBase = GOPTable->Mode->FrameBufferBase;
			*(Gfx->GPUs[DeviceInd].Info) = *(GOPTable->Mode->Info);

		} // End loop
        
    } else if((xGfxHandles > 1) && (DeviceInd == 1)) {
        // Configure one device
        Gfx->FBCount = 1;
		Status = BS->AllocatePool(EfiLoaderData, Gfx->FBCount * sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE), (void**)&Gfx->GPUs);
		if (EFI_ERROR(Status)) {
			Print(L"GPUs AllocatePool error: 0x%llx\r\n", Status);
			return Status;
		}
		while (0x30 > Key.UnicodeChar || Key.UnicodeChar > (0x30 + xGfxHandles - 1)) {
			for (size_t DeviceNumIterator = 0; DeviceNumIterator < xGfxHandles; DeviceNumIterator++) {
				Print(L"%s", DeviceNames[DeviceNumIterator]);
			}
			Print(L"\r\n");
			Print(L"Please select an output device to configure. (0-%llu)\r\n", xGfxHandles - 1);
			while ((ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);
			Print(L"\r\nDevice %c selected.\r\n\n", Key.UnicodeChar);
		}
		DeviceInd = (size_t)(Key.UnicodeChar - 0x30);
		Key.UnicodeChar = 0;

		EFI_GRAPHICS_OUTPUT_PROTOCOL *GOPTable;
		Status = BS->OpenProtocol(GfxHandles[DeviceInd], &GraphicsOutputProtocol, (void**)&GOPTable, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (EFI_ERROR(Status)) {
			Print(L"GraphicsTable OpenProtocol error: 0x%llx\r\n", Status);
			return Status;
		}

        if(GOPTable->Mode->MaxMode == 1) {
            GfxMode = 0;
        } else {
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* GOPInfo2;
			while (0x30 > Key.UnicodeChar || Key.UnicodeChar > (0x30 + GOPTable->Mode->MaxMode - 1)) {
				Print(L"%s\r\n", DeviceNames[DeviceInd]);
				Print(L"%u available graphics modes.\r\n\n", GOPTable->Mode->MaxMode);
				Print(L"Current mode: %c\r\n", GOPTable->Mode->Mode + 0x30);
				for (GfxMode = 0; GfxMode < GOPTable->Mode->MaxMode; GfxMode++) {
					Status = GOPTable->QueryMode(GOPTable, GfxMode, &GOPInfoSize, &GOPInfo2);
					if (EFI_ERROR(Status)) {
						Print(L"GraphicsTable QueryMode error: 0x%llx\r\n", Status);
					}
					Print(L"%c. %ux%u, PxPerScanLine: %u, PxFormat: %s\r\n", GfxMode + 0x30, GOPInfo2->HorizontalResolution, GOPInfo2->VerticalResolution, GOPInfo2->PixelsPerScanLine, pixelFormats[GOPInfo2->PixelFormat]);
					// Don't need GOPInfo2 anymore
					Status = BS->FreePool(GOPInfo2);
					if (EFI_ERROR(Status)) {
						Print(L"Error freeing GOPInfo2 pool. 0x%llx\r\n", Status);
					}
				}

				Print(L"\r\nPlease select a graphics mode. (0 - %c)\r\n", 0x30 + GOPTable->Mode->MaxMode - 1);
				while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);
				Print(L"\r\nSelected graphics mode %c.\r\n\n", Key.UnicodeChar);
			}

			GfxMode = (uint32_t)(Key.UnicodeChar - 0x30);
			Key.UnicodeChar = 0;
			Print(L"Setting graphics mode %u of %u.\r\n\n", GfxMode + 1, GOPTable->Mode->MaxMode);

		}

		Status = GOPTable->SetMode(GOPTable, GfxMode);
		if (EFI_ERROR(Status)) {
			Print(L"GraphicsTable SetMode error. 0x%llx\r\n", Status);
        }
		DeviceInd = 0;
		Status = BS->AllocatePool(EfiLoaderData, GOPTable->Mode->SizeOfInfo, (void**)&Gfx->GPUs[DeviceInd].Info);
		if (EFI_ERROR(Status)) {
			Print(L"GOP Mode->Info AllocatePool error. 0x%llx\r\n", Status);
		}

		Gfx->GPUs[DeviceInd].MaxMode = GOPTable->Mode->MaxMode;
		Gfx->GPUs[DeviceInd].Mode = GOPTable->Mode->Mode;
		Gfx->GPUs[DeviceInd].SizeOfInfo = GOPTable->Mode->SizeOfInfo;
		Gfx->GPUs[DeviceInd].FrameBufferSize = GOPTable->Mode->FrameBufferSize;
		Gfx->GPUs[DeviceInd].FrameBufferBase = GOPTable->Mode->FrameBufferBase;
		*(Gfx->GPUs[DeviceInd].Info) = *(GOPTable->Mode->Info);
    } else if((xGfxHandles > 1) && (DeviceInd == 2)) {
        // Native resolutions. Does not work right now.
        Gfx->FBCount = xGfxHandles;
        Status = BS->AllocatePool(EfiLoaderData, Gfx->FBCount * sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE), (void**)&Gfx->GPUs);
        if(EFI_ERROR(Status)) {
            Print(L"GPUs AllocatePool error.\r\n");
        }

        for(DeviceInd = 0; DeviceInd < xGfxHandles; DeviceInd++) {
            EFI_GRAPHICS_OUTPUT_PROTOCOL* GOPTable;

            Status = BS->OpenProtocol(GfxHandles[DeviceInd], &GraphicsOutputProtocol, (void**)&GOPTable, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
            if(EFI_ERROR(Status)) {
                Print(L"GraphicsTable OpenProtocol error.\r\n");
            }

            GfxMode = 0;
            Status = GOPTable->SetMode(GOPTable, GfxMode);
            if(EFI_ERRROR(Status)) {
                Print(L"GraphicsTable SetMode error\r\n");
            }

            Status = BS->AllocatePool(EfiLoaderData, GOPTable->Mode->SizeOfInfo, (void**)&Gfx->GPUs[DeviceInd].Info);
            if(EFI_ERROR(Status)) {
                Print(L"GOP Mode->Info AllocatePool error.\r\n");
            }

            Gfx->GPUs[DeviceInd].MaxMode = GOPTable->Mode->MaxMode;
            Gfx->GPUs[DeviceInd].Mode = GOPTable->Mode;
            Gfx->GPUs[DeviceInd].SizeOfInfo = GOPTable->Mode->SizeOfInfo;
            Gfx->GPUs[DeviceInd].FrameBufferSize = GOPTable->Mode->FrameBufferSize;
            Gfx->GPUs[DeviceInd].FrameBufferBase = GOPTable->Mode->FrameBufferBase;
            *(Gfx->GPUs[DeviceInd].Info) = *(GOPTable->Mode->Info);
        }

    } else if((xGfxHandles > 1) && (DeviceInd == 3)) {
        // Set all to 1024x768
        // This is Windows' default resolution, so every UEFI device supports it.

        Gfx->FBCount = xGfxHandles;
        Status = BS->AllocatePool(EfiLoaderData, Gfx->FBCount * sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE), (void**)&Gfx->GPUs);

        if(EFI_ERROR(Status)) {
            Print(L"GPUs AllocatePool error.\r\n");
        }

        for(DeviceInd = 0; DeviceInd < xGfxHandles; DeviceInd++) {
            EFI_GRAPHICS_OUTPUT_PROTOCOL* GOPTable;
            Status = BS->OpenProtocol(GfxHandles[DeviceInd], &GraphicsOutputProtocol, (void**)&GOPTable, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
            if(EFI_ERROR(Status)) {
                Print(L"GraphicsTable OpenProtocol error.\r\n");
            }

            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* GOPInfo2;
            size_t GOPInfoSize;

            for(GfxMode = 0; GfxMode < GOPTable->Mode->MaxMode; GfxMode++) {
                Status = GOPTable->QueryMode(GOPTable, GfxMode, &GOPInfoSize, &GOPInfo2);
                if(EFI_ERROR(Status)) {
                    Print(L"GraphicsTable QueryMode error\r\n");
                }

                if((GOPInfo2->HorizontalResolution == 1024) & (GOPInfo2->VerticalResolution == 768)) {
                    Status = BS->FreePool(GOPInfo2);
                    if(EFI_ERROR(Status)) {
                        Print(L"Error freeing GOPInfo2\r\n");
                    }

                    break;
                }

                Status = BS->FreePool(GOPInfo2);
                if(EFI_ERROR(Status)) {
                    Print(L"Error freeing GOPInfo2\r\n");
                }

            }

            if(GfxMode == GOPTable->Mode->MaxMode) {
                Print(L"No 1024x768 mode. Defaulting to first available mode.\r\n");
                GfxMode = 0;
            }

            Status = GOPTable->SetMode(GOPTable, GfxMode);
            if(EFI_ERROR(Status)) {
                Print(L"GraphicsTable SetMode Error\r\n");
            }
            
            Status = BS->AllocatePool(EfiLoaderData, GOPTable->Mode->SizeOfInfo, (void**)Gfx->GPUs[DeviceInd].Info);
            if(EFI_ERROR(Status)) {
                Print(L"GOP Mode->Info AllocatePool Error\r\n");
            }

            Gfx->GPUs[DeviceInd].MaxMode = GOPTable->Mode->MaxMode;
            Gfx->GPUs[DeviceInd].Mode = GOPTable->Mode->Mode;
            Gfx->GPUs[DeviceInd].SizeOfInfo = GOPTable->Mode->SizeOfInfo;
            Gfx->GPUs[DeviceInd].FrameBufferSize = GOPTable->Mode->FrameBufferSize;
            Gfx->GPUs[DeviceInd].FrameBufferBase = GOPTable->Mode->FrameBufferBase;
            *(Gfx->GPUs[DeviceInd].Info) = *(GOPTable->Mode->Info);
            
        }
        
    } else {
        // xGfxDevices = 1
        // AKA, only one GPU.
        // The most common case for hardware.
        // Rare for VMs.

        Gfx->FBCount = 1;
        Status = BS->AllocatePool(EfiLoaderData, Gfx->FBCount * sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE), (void**)&Gfx->GPUs);

        if(EFI_ERROR(Status)) {
            Print(L"GPUs AllocatePool error.\r\n");
        }

        DeviceInd = 0;

        EFI_GRAPHICS_OUTPUT_PROTOCOL* GOPTable;
        Status = BS->OpenProtocol(GfxHandles[DeviceInd], &GraphicsOutputProtocol, (void**)&GOPTable, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if(EFI_ERROR(Status)) {
            Print(L"GraphicsTable OpenProtocol error.\r\n");
        }

        if(GOPTable->Mode->MaxMode == 1) {
            GfxMode = 0;
        } else {
            uint32_t DefaultMode = 0;
			size_t Timeout = 5;
			EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* GOPInfo2;
			while (0x30 > Key.UnicodeChar || Key.UnicodeChar > (0x30 + GOPTable->Mode->MaxMode - 1)) {
				Print(L"%s\r\n", DeviceNames[DeviceInd]);
				Print(L"%u available graphics modes.\r\n\n", GOPTable->Mode->MaxMode);
				Print(L"Current mode: %c\r\n", GOPTable->Mode->Mode + 0x30);
				for (GfxMode = 0; GfxMode < GOPTable->Mode->MaxMode; GfxMode++) {
					Status = GOPTable->QueryMode(GOPTable, GfxMode, &GOPInfoSize, &GOPInfo2);
					if (EFI_ERROR(Status)) {
						Print(L"GraphicsTable QueryMode error.\r\n");
					}
					Print(L"%c, %ux%u, %u pixels per scan line, Pixel format %s\r\n", GfxMode + 0x30, GOPInfo2->HorizontalResolution, GOPInfo2->VerticalResolution, GOPInfo2->PixelsPerScanLine, pixelFormats[GOPInfo2->PixelFormat]);
					Status = BS->FreePool(GOPInfo2);
					if (EFI_ERROR(Status)) {
						Print(L"Error freeing GOPInfo2 pool.\r\n");
					}
				}
				Print(L"\r\n");
				while (Timeout) {
					Print(L"Please select a graphics mode. (0 - %c). Defaulting to mode %c in %llu... \r", 0x30 + GOPTable->Mode->MaxMode - 1, DefaultMode + 0x30, Timeout);
					Status = WaitForSingleEvent(ST->ConIn->WaitForKey, 10000000);
					if (Status != EFI_TIMEOUT) {
						Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);
						if (EFI_ERROR(Status)) {
							Print(L"Error reading keystroke.\r\n");
						}
						Print(L"\n\nSelected graphics mode %c.\r\n\n", Key.UnicodeChar);
						break;
					}
					Timeout -= 1;
				}
				if (!Timeout) {
					Print(L"\n\nDefaulting to mode %c..\r\n\n", DefaultMode + 0x30);
					GfxMode = DefaultMode;
					break;
				}
			}

			if (Timeout) {
				GfxMode = (uint32_t)(Key.UnicodeChar - 0x30);
            }

			Key.UnicodeChar = 0;
			Print(L"Setting graphics mode %u of %u.\r\n\n", GfxMode + 1, GOPTable->Mode->MaxMode);
        }

        Status = GOPTable->SetMode(GOPTable, GfxMode);
		if (EFI_ERROR(Status)) {
			Print(L"GOPTable SetMode error.\r\n");
		}

        Status = BS->AllocatePool(EfiLoaderData, GOPTable->Mode->SizeOfInfo, (void**)&Gfx->GPUs[DeviceInd].Info);
		if (EFI_ERROR(Status)) {
			Print(L"GOP Mode->Info AllocatePool error.\r\n");
		}

        Gfx->GPUs[DeviceInd].MaxMode = GOPTable->Mode->MaxMode;
        Gfx->GPUs[DeviceInd].Mode = GOPTable->Mode->Mode;
        Gfx->GPUs[DeviceInd].SizeOfInfo = GOPTable->Mode->SizeOfInfo;
        Gfx->GPUs[DeviceInd].MaxMode = GOPTable->Mode->MaxMode;
        Gfx->GPUs[DeviceInd].FrameBufferSize = GOPTable->Mode->FrameBufferSize;
        Gfx->GPUs[DeviceInd].FrameBufferBase = GOPTable->Mode->FrameBufferBase;

        *(Gfx->GPUs[DeviceInd].Info) = *(GOPTable->Mode->Info);
    }

    for(size_t StringNamesFree = 0; StringNamesFree < xGfxHandles; StringNamesFree++) {
        Status = BS->FreePool(DeviceNames[StringNamesFree]);
        if (EFI_ERROR(Status)) {
			Print(L"NameBuffer[%llu] (%s) FreePool error.\r\n");
		}
    }

    Status = BS->FreePool(DeviceNames);
    if (EFI_ERROR(Status)) {
		Print(L"NameBuffer FreePool error.\r\n");
	}

    Status = BS->FreePool(GfxHandles);
    if (EFI_ERROR(Status)) {
		Print(L"GfxHandles FreePool error.\r\n");
	}

    /* ================================== END OF GRAPHICS ================================== */

    // Do we dare?
    FILELOADER_PARAMS* Params;

    Status = BS->AllocatePool(EfiLoaderData, sizeof(FILELOADER_PARAMS), (void**)&Params);
    if(EFI_ERROR(Status)) {
        Print(L"Error allocating Loader Params.\r\n");
    }

    // Get information about the boot file

    EFI_PHYSICAL_ADDRESS KernelBase = 0;
    size_t KernelPages = 0;

    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;

    Status = BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(Status)) {
        Print(L"LoadedImageProtocol error\r\n");
    }

    Print(L"Sync loaded at 0x%llx\n", LoadedImage->ImageBase);

    CHAR16* ESPRootTemp = DevicePathToStr(DevicePathFromHandle(LoadedImage->DeviceHandle));
    size_t ESPRootSize = StrSize(ESPRootTemp);

    CHAR16* ESPRoot;

    Status = BS->AllocatePool(EfiLoaderData, ESPRootSize, (void**)&ESPRoot);
    if(EFI_ERROR(Status)) {
        Print(L"ESPRoot AllocatePool error.\r\n");
    }

    CopyMem(ESPRoot, ESPRootTemp, ESPRootSize);

    Status = BS->FreePool(ESPRootTemp);
    if(EFI_ERROR(Status)) {
        Print(L"ESPRoot FreePool error.\r\n");
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;

    Status = BS->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&FileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if(EFI_ERROR(Status)) {
        Print(L"FileSystem OpenProtocol Error\r\n");
    }

    EFI_FILE* CurrentDriveRoot;

    Status = FileSystem->OpenVolume(FileSystem, &CurrentDriveRoot);
    if(EFI_ERROR(Status)) {
        Print(L"FileSystem OpenVolume error\r\n");
    }

    size_t KernelPathSize = ESPRootSize + StrSize("BOOTX64.EFI");
    CHAR16* KernelPath = ESPRoot + L"BOOTX64.EFI";
    Print(L"Kernel Path is %s", KernelPath);

    Status = BS->AllocatePool(EfiLoaderData, KernelPathSize, (void**)&KernelPath);
    if(EFI_ERROR(Status)) {
        Print(L"KernelPath AllocatePool error.\r\n");
    }

    EFI_FILE* KernelFile;

    Status = CurrentDriveRoot->Open(CurrentDriveRoot, &KernelFile, KernelPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if(EFI_ERROR(Status)) {
        Print(L"Unknown error opening boot file. Aborting.\r\n");
        for(;;) {}
    }

    size_t FileInfoSize;
    Status = KernelFile->GetInfo(KernelFile, &gEfiFileInfoGuid, &FileInfoSize, NULL);
    EFI_FILE_INFO* KernelFileInfo;

    Status = BS->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&KernelFileInfo);
    if(EFI_ERROR(Status)) {
        Print(L"FileInfo AllocatePool error.\r\n");
    }

    Status = KernelFile->GetInfo(KernelFile, &gEfiFileInfoGuid, &FileInfoSize, KernelFileInfo);
    if(EFI_ERROR(Status)) {
        Print(L"Kernel file GetInfo error.\r\n");
    }

    size_t MapSize = 0;
    size_t MapKey = 0;
    size_t MapDescSize = 0;

    uint32_t MapDescriptorVersion = 0;

    EFI_MEMORY_DESCRIPTOR* Map = NULL;

    Status = BS->GetMemoryMap(&MapSize, Map, &MapKey, &MapDescSize, &MapDescriptorVersion);
    if(Status == EFI_BUFFER_TOO_SMALL) {
        Status = BS->AllocatePool(EfiLoaderData, MapSize, (void**)&Map);
        if(EFI_ERROR(Status)) {
            Print(L"Memory map AllocatePool error.\r\n");
            for(;;) {}
        }

        Status = BS->GetMemoryMap(&MapSize, Map, &MapKey, &MapDescSize, &MapDescriptorVersion);
    }

    Status = BS->ExitBootServices(ImageHandle, MapKey);
    if(EFI_ERROR(Status)) {
        Status = BS->FreePool(Map);
        if(EFI_ERROR(Status)) {
            Print(L"Error freeing memory map after failed EBS.\r\n");
            for(;;) {}
        }

        MapSize = 0;
        Status = BS->GetMemoryMap(&MapSize, Map, &MapKey, &MapDescSize, &MapDescriptorVersion);
        if(Status == EFI_BUFFER_TOO_SMALL) {
            Status = BS->AllocatePool(EfiLoaderData, MapSize, (void**)&Map);
            if(EFI_ERROR(Status)) {
                Print(L"Memory map AllocatePool error.\r\n");
                for(;;) {}
            }

            Status = BS->GetMemoryMap(&MapSize, Map, &MapKey, &MapDescSize, &MapDescriptorVersion);
        }

        Status = BS->ExitBootServices(ImageHandle, MapKey);
    }


    if(EFI_ERROR(Status)) {
        Print(L"Errors occurred during boot. Will not go blindly into the night.\r\n");
        for (;;) {}
    }

    
	Params->UEFI_Version = ST->Hdr.Revision;
	Params->Bootloader_MajorVersion = MAJOR_VER;
	Params->Bootloader_MinorVersion = MINOR_VER;
	Params->Memory_Descriptor_Version = MapDescriptorVersion;
	Params->Memory_Descriptor_Size = MapDescSize;

	Params->Kernel_Base = KernelBase;
	Params->Kernel_Pages = KernelPages;

	Params->ESP_Path = ESPRoot;
	Params->ESP_Path_Length = ESPRootSize;
	Params->Kernel_Path = KernelPath;
	Params->Kernel_Path_Length = KernelPathSize;
	Params->Kernel_Options = L"";
	Params->Kernel_Options_Length = 0;
	Params->RTServices = RT;
	Params->GPU_INFO = Gfx;
	Params->FileMeta = KernelFileInfo;

	Params->ConfigTables = ST->ConfigurationTable;
	Params->ConfigTables_Length = ST->NumberOfTableEntries;

    kernel_main(Params);

}