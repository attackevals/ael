# C2DownloadFile Module

target_sources(
    shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/C2DownloadFile.cpp
)

target_sources(
    test5shellcode-pe
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/C2DownloadFile.cpp
)
