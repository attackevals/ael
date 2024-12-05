#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <synchapi.h>
#include <string>
#include <set>
#include "util/winapihelper.hpp"

typedef NTSTATUS (WINAPI *FP_NtQueryInstallUILanguage)(LANGID*);
typedef NTSTATUS (WINAPI *FP_NtQueryDefaultUILanguage)(LANGID*);

using FP_CreateMutexW = decltype(&CreateMutexW);

namespace guardrails {

std::set<LANGID> GetDefaultLanguageExclusionSet();

// Defined in src/lockbit_main/guardrails/mutex.cpp
// Generates mutex name by taking the hex representation of the DJB2 hash of the seed
// and appending it to "Global\". Example: "Global\58d4e671"
std::wstring GenerateMutexName(const std::string& seed);

/*
 * CheckCreateMutex (defined in src/lockbit_main/guardrails/mutex.cpp):
 *      About:
 *          Checks for the presence of a global mutex ("Global\<DJB2 hash of seed>") 
 *          to determine if there are multiple concurrent instances of this process 
 *          on the local host. If the function returns true, the current process will 
 *          keep a handle to the mutex, which must be closed prior to termination by 
 *          calling guardrails::ReleaseMutex();
 *          Mutex creation and checks are performed using the CreateMutexW API call.
 *      Result:
 *          Returns true is the mutex does not currently exist - in other words, if
 *          this process is the only LockBit instance running on the local host.
 *          Returns false if the mutex already exists, or if mutex creation fails.
 *      MITRE ATT&CK Techniques:
 *          T1480: Execution Guardrails
 *      CTI:
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 */
bool CheckCreateMutex(const std::string& seed);

// Defined in src/lockbit_main/guardrails/mutex.cpp
// Release the global mutex, if created
void ReleaseMutex();

/*
 * SystemHasExcludedLang (defined in src/lockbit_main/guardrails/lang.cpp):
 *      About:
 *          Returns true if the system language appears in the lang_filter_list, false otherwise.
 *          If the function cannot check the system language for whatever reason, it will log
 *          an error message and return false.
 *          Language infromation is obtained using the NtQueryInstallUILanguage and NtQueryDefaultUILanguage
 *          API calls.
 *      Result:
 *          True if the system install UI or default UI languages are in the provided exclusion list,
 *          false otherwise or if the function cannot retrieve the languagae IDs.
 *      MITRE ATT&CK Techniques:
 *          T1614.001: System Location Discovery: System Language Discovery
 *          T1480: Execution Guardrails
 *          T1106: Native API
 *      CTI:
 *          https://www.antiy.net/p/analysis-of-lockbit-ransomware-samples-and-considerations-for-defense-against-targeted-ransomware/
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-075a
 *          https://www.cisa.gov/news-events/cybersecurity-advisories/aa23-165a
 */
bool SystemHasExcludedLang(const std::set<LANGID>& lang_filter_list);

} // namespace