/*
 * api_helper.hpp:
 *      About:
 *          Dynamically resolve Windows APIs using the FNV1A hash.
 *          Modified from: https://github.com/Tai7sy/vs-obfuscation/blob/2092b064f2d204e16e794448d677e4471e43314b/obfuscation.h
 *      MITRE ATT&CK Techniques:
 *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
 *      CTI:
 *          https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
 */

#pragma once

#include "win_helper.h"
#include "fnv1a.hpp"

static void* _getModuleProcAddressByHash(DWORD_PTR library_base, uint32_t api_hash) {
    // Verify DOS and Image NT headers
    PIMAGE_DOS_HEADER p_dos_header = (PIMAGE_DOS_HEADER)library_base;
    if (p_dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        return NULL;
    }

    PIMAGE_NT_HEADERS p_image_nt_headers = (PIMAGE_NT_HEADERS)(library_base + p_dos_header->e_lfanew);
    if (p_image_nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        return NULL;
    }

    // Verify image is a DLL
    if ((p_image_nt_headers->FileHeader.Characteristics & IMAGE_FILE_DLL) != IMAGE_FILE_DLL) {
        return NULL;
    }

    // Make sure library has exports
    if (p_image_nt_headers->FileHeader.SizeOfOptionalHeader < 96 || p_image_nt_headers->OptionalHeader.NumberOfRvaAndSizes == 0) {
        return NULL;
    }
    DWORD rva_export_dir = p_image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD rva_export_size = p_image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    if (rva_export_dir == 0) {
        return NULL;
    } else if (rva_export_size == 0) {
        return NULL;
    }

    // Get addresses to export info
    PIMAGE_EXPORT_DIRECTORY p_export_dir = (PIMAGE_EXPORT_DIRECTORY)(library_base + rva_export_dir);
    PDWORD p_export_table = (PDWORD)(library_base + p_export_dir->AddressOfFunctions);
    PDWORD p_name_table = (PDWORD)(library_base + p_export_dir->AddressOfNames);
    PWORD p_ordinal_table = (PWORD)(library_base + p_export_dir->AddressOfNameOrdinals);

    // Iterate through exported functions, grab the one that matches our hash
    for (int i = 0; i < p_export_dir->NumberOfNames; i++) {
        // Get function name
        DWORD rva_func_name = p_name_table[i];
        char *func_name = (char *)(library_base + rva_func_name);

        // Check if we want this API
        if (cx_fnv_hash(func_name) == api_hash) {
            // Use ordinal to get function address
            DWORD rva_func_addr = p_export_table[p_ordinal_table[i]];
            void* func_addr = (void*)(library_base + rva_func_addr);

            // Check if address is actually a forwarder - return NULL if so
            if (rva_func_addr >= rva_export_dir && rva_func_addr < rva_export_dir + rva_export_size) {
                return NULL;
            }
            return func_addr;
        }
    }

	return NULL;
}

static void *_get_func_by_hash(uint32_t api_hash, const wchar_t* dll_name) {
	// Get Pointer to PEB structure
	PPEB peb = (PPEB)__readgsqword(0x60);

    // Iterate through modules until we find the target module
    PPEB_LDR_DATA ldr_data = peb->Ldr;
    LIST_ENTRY* list_entry = ldr_data->InMemoryOrderModuleList.Flink;
    do {
        PLDR_DATA_TABLE_ENTRY1 peb_module_entry = CONTAINING_RECORD(list_entry, LDR_DATA_TABLE_ENTRY1, InMemoryOrderLinks);
        if (_wcsnicmp(peb_module_entry->BaseDllName.Buffer, dll_name, peb_module_entry->BaseDllName.Length / sizeof(wchar_t)) == 0) {
            return _getModuleProcAddressByHash((DWORD_PTR)(peb_module_entry->DllBase), api_hash);
        }

        // Iterate to next module. Final module will loop back to beginning of list.
        list_entry = list_entry->Flink;
    } while (list_entry != &(ldr_data->InMemoryOrderModuleList));

    return NULL;
}

template <uint32_t hash>
static void* _lazyimport_get(const wchar_t* dll_name) {
	static void* pfn = NULL;
	if (!pfn) {
        pfn = _get_func_by_hash(hash, dll_name);
    }
	return pfn;
}

template <uint32_t hash>
static void* _lazyimport_get_by_hmodule(HMODULE h_module) {
	static void* pfn = NULL;
	if (!pfn) {
        pfn = _getModuleProcAddressByHash((DWORD_PTR)(h_module), hash);
    }
	return pfn;
}

// Define macros for obtaining API calls
// CTI: https://www.trendmicro.com/en_us/research/22/k/earth-preta-spear-phishing-governments-worldwide.html
#define RESOLVE_FN_FNV1A(dll_name, api_name) (reinterpret_cast<decltype(&api_name)>(_lazyimport_get<cx_fnv_hash(#api_name)>(dll_name)))
#define RESOLVE_FN_PTR_FNV1A(dll_name, api_name) (_lazyimport_get<cx_fnv_hash(#api_name)>(dll_name))
#define RESOLVE_FN_FNV1A_BY_HMODULE(h_dll, api_name) (reinterpret_cast<decltype(&api_name)>(_lazyimport_get_by_hmodule<cx_fnv_hash(#api_name)>(h_dll)))
