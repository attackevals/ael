#include "rat.hpp"
#include <print>
#include <Windows.h>

// Stubbed functionality of the real RAT payload for testing. If the
// signatures diverge there should be a compiler error related to overloading.
extern "C" __declspec(dllexport)
bool run() {
    std::println("RAT export executed");
    return true;
}

extern "C"
bool DllMain(HINSTANCE, DWORD, void*){
    return true;
}