#include <iostream>
#include <windows.h>
#include <TlHelp32.h>




extern "C" __declspec (dllexport) int calc()
{
    ShellExecuteA(0, "open", "calc.exe", 0, 0, SW_SHOWNORMAL);

    return 0;
}










