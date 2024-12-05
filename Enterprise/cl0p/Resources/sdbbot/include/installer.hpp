// installer.hpp is the installer's public interface. It should only contain
// information about its exported functions for consumers.
# pragma once
#include <Windows.h>

extern "C" __declspec(dllexport)
void install(HWND, HINSTANCE, char*, int);
