# CreateRegistryKey Module

target_sources(
    shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CreateRegistryRunKey.cpp
)

target_sources(
    test5shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CreateRegistryRunKey.cpp
)
