#include <Windows.h>

int wmain(int argc, wchar_t* argv[]) {
    MessageBoxW(nullptr, L"Test", L"Hook", MB_OK);
    while (true) {};
}
