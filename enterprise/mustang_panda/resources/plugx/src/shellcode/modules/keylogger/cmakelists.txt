# Keylogger Module

target_sources(
    shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Keylogger.cpp
)

target_sources(
    test5shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Keylogger.cpp
)
