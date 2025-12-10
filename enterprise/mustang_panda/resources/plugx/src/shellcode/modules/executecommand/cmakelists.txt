# ExecuteCommand Module

target_sources(
    shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ExecuteCommand.cpp
)

target_sources(
    test5shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ExecuteCommand.cpp
)
