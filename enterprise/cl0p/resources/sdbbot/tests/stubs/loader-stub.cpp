#include "loader.hpp"
#include <print>
#include <Windows.h>

// Stubbed functionality of the real loader payload for testing. If the
// signatures diverge there should be a compiler error related to overloading.
extern "C" __declspec(dllexport)
bool load() {
    std::println("Loader export executed");
    return true;
}

//Convenient define for the fdwReason
#define VERIFIER_LOAD 4

/*
	Prototypes for additional event callbacks (dll load, unload, and heap
	free). If you wanted to, you could respond to these events. We will leave
	their function bodies undefined.
*/
typedef VOID(NTAPI* RTL_VERIFIER_DLL_LOAD_CALLBACK) (PWSTR DllName, PVOID DllBase, SIZE_T DllSize, PVOID Reserved);
typedef VOID(NTAPI* RTL_VERIFIER_DLL_UNLOAD_CALLBACK) (PWSTR DllName, PVOID DllBase, SIZE_T DllSize, PVOID Reserved);
typedef VOID(NTAPI* RTL_VERIFIER_NTDLLHEAPFREE_CALLBACK) (PVOID AllocationBase, SIZE_T AllocationSize);

//Necessary structures
typedef struct _RTL_VERIFIER_THUNK_DESCRIPTOR {
	PCHAR ThunkName;
	PVOID ThunkOldAddress;
	PVOID ThunkNewAddress;
} RTL_VERIFIER_THUNK_DESCRIPTOR, * PRTL_VERIFIER_THUNK_DESCRIPTOR;

typedef struct _RTL_VERIFIER_DLL_DESCRIPTOR {
	PWCHAR DllName;
	DWORD DllFlags;
	PVOID DllAddress;
	PRTL_VERIFIER_THUNK_DESCRIPTOR DllThunks;
} RTL_VERIFIER_DLL_DESCRIPTOR, * PRTL_VERIFIER_DLL_DESCRIPTOR;

typedef struct _RTL_VERIFIER_PROVIDER_DESCRIPTOR {
	DWORD Length;
	PRTL_VERIFIER_DLL_DESCRIPTOR ProviderDlls;
	RTL_VERIFIER_DLL_LOAD_CALLBACK ProviderDllLoadCallback;
	RTL_VERIFIER_DLL_UNLOAD_CALLBACK ProviderDllUnloadCallback;
	PWSTR VerifierImage;
	DWORD VerifierFlags;
	DWORD VerifierDebug;
	PVOID RtlpGetStackTraceAddress;
	PVOID RtlpDebugPageHeapCreate;
	PVOID RtlpDebugPageHeapDestroy;
	RTL_VERIFIER_NTDLLHEAPFREE_CALLBACK ProviderNtdllHeapFreeCallback;
} RTL_VERIFIER_PROVIDER_DESCRIPTOR, * PRTL_VERIFIER_PROVIDER_DESCRIPTOR;

//DbgPrint 
typedef ULONG(__cdecl* PFN_DbgPrint)(PCH, ...);
PFN_DbgPrint DbgPrint;

/*
	Arrays containing information about the functions we are hooking, and where they are located.
	In this case, PR_WRITE is located in nss3.dll
*/
static RTL_VERIFIER_THUNK_DESCRIPTOR aThunks[] = { { nullptr, nullptr, nullptr },{} };
static RTL_VERIFIER_DLL_DESCRIPTOR aDlls[] = { { nullptr, 0, nullptr, &aThunks[0] },{} };
static RTL_VERIFIER_PROVIDER_DESCRIPTOR vpd = { sizeof(RTL_VERIFIER_PROVIDER_DESCRIPTOR), aDlls };

extern "C"
bool WINAPI DllMain(
	HINSTANCE,
	DWORD reason,
	PRTL_VERIFIER_PROVIDER_DESCRIPTOR* verifier
) {
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	case VERIFIER_LOAD:
		//Resolve the location of DbgPrint() at runtime
		DbgPrint = (PFN_DbgPrint)::GetProcAddress(::GetModuleHandle(TEXT("NTDLL")), "DbgPrint");
		*verifier = &vpd;
		break;
	}
	return true;
}