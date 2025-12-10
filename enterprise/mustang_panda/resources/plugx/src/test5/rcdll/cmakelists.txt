###############################################################################
# TEST 5 RCDLL HIJACKING DLL
###############################################################################
add_library(
    rcdll
	SHARED

    "stub.cpp"
	"rcdll.cpp"
    "rcdll_util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/rc4.cpp"
	"rcdll.def"

    # Logging functionality
    "${CMAKE_SOURCE_DIR}/src/common/base64.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_aes_ctr.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/util.cpp"
)

target_include_directories(
    rcdll
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/common"
)

target_compile_definitions(
    rcdll
    PRIVATE
    DLL_HANDLER_LOG_FILE="C:\\\\Windows\\\\System32\\\\rcdll.log"
    DLL_MSI_HANDLER_LOG_FILE="C:\\\\Windows\\\\System32\\\\rcdll_i.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

target_link_libraries(
    rcdll
    bcrypt
)

add_custom_command(TARGET rcdll POST_BUILD COMMAND echo rcdll target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})
