###############################################################################
# TEST 4 DROPPER EXECUTABLE
###############################################################################
add_executable(
    test4

    "dropper.cpp"
    "dropper_util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/handler_util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/logger.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_aes_ctr.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/pi_base64.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/util.cpp"
    "${CMAKE_SOURCE_DIR}/src/common/xor.cpp"

    "${CMAKE_SOURCE_DIR}/src/test4/dropper/e_embedded.hpp"
    "${CMAKE_SOURCE_DIR}/src/test4/dropper/d_embedded.hpp"
)

target_include_directories(
    test4
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/common"
)

add_dependencies(
    test4
    gflagsui
    dropper_headers
)

target_compile_definitions(
    test4
    PRIVATE
    DROPPER_LOG_FILE="C:\\\\Windows\\\\System32\\\\t4.log"
    ENCRYPTED_LOGGING=1 # set to 0 to disable, set to non-0 to enable
)

add_custom_command(
    TARGET test4
    PRE_BUILD
    COMMAND
        echo building test4 target with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS}
    DEPENDS "${CMAKE_SOURCE_DIR}/src/test4/dropper/e_embedded.hpp"
)

# Sign Dropper and build zip file
add_custom_command(
    TARGET test4
    POST_BUILD
    COMMAND powershell.exe -File "${CMAKE_SOURCE_DIR}/src/common/sign_artifact.ps1" -Target "$<TARGET_FILE:test4>" -CertSubject "CN=Casterly Enterprises, O=Casterly Enterprises, L=Casterly Rock, S=Lannisport, C=Westeros" -CertDnsName "Casterly Enterprises" -CertExportPath "$<TARGET_FILE_DIR:gflagsui>/test4dropper.pfx"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:test4>" "$<TARGET_FILE_DIR:test4>/Assessing Westeros-Essos Global Influence.pif"
    COMMAND powershell.exe -File "${CMAKE_SOURCE_DIR}/src/test4/create_test4_zip.ps1" -TestExec "$<TARGET_FILE_DIR:test4>/Assessing Westeros-Essos Global Influence.pif" -PdfFile "${CMAKE_SOURCE_DIR}/src/test4/dropper/Assessing Westeros-Essos Global Influence (1).pdf" -OutputFile "$<TARGET_FILE_DIR:test4>/protections4.zip"
)

target_include_directories(
    test4
    PUBLIC
    "${CMAKE_SOURCE_DIR}/src/common"
    "${CMAKE_SOURCE_DIR}/src/test4/dropper"
)
