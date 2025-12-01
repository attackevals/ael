###############################################################################
# PLUG X
###############################################################################

###############################################################################
# Build loader components
###############################################################################
add_subdirectory("loader")

###############################################################################
# Build shellcode components
###############################################################################
add_subdirectory("shellcode")

###############################################################################
# Build Test 5 components
###############################################################################
add_subdirectory("test5")

###############################################################################
# Unit test source files
###############################################################################
target_sources(
    plugx-test
    PRIVATE
    "common/base64.cpp"
    "common/logger.cpp"
    "common/logger_test.cpp"
    "common/pi_aes_ctr.cpp"
    "common/pi_aes_ctr_test.cpp"
    "common/util.cpp"
    "test5/rcdll/rcdll_util.cpp"
    "test5/rcdll/rcdll_util_test.cpp"
)

target_include_directories(
    plugx-test
    PUBLIC
    "common"
    "shellcode"
    "test5/rcdll"
)

target_compile_definitions(
    plugx-test
    PRIVATE
    SERVER="10.100.0.70"
    PORT=8080
    SH_LOG_FILE="C:\\\\users\\\\public\\\\plugx_test_shellcode.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)
