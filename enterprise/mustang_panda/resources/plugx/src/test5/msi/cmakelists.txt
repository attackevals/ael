set_property(GLOBAL PROPERTY T5_MSI_NAME "2025erdt")
get_property(T5_MSI_NAME GLOBAL PROPERTY T5_MSI_NAME)
set_property(GLOBAL PROPERTY T5_MSI_PATH "${CMAKE_SOURCE_DIR}/src/test5/MSI/$<CONFIG>/MSI.msi")
get_property(T5_MSI_PATH GLOBAL PROPERTY T5_MSI_PATH)

# build MSI after the library and shellcode
add_custom_command(
    DEPENDS GenerateResinfoDat
    OUTPUT "${T5_MSI_PATH}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:shellcode-pe>/resinfo.dat" "${CMAKE_SOURCE_DIR}/build/MSI/resinfo.dat"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE_DIR:rcdll>/rcdll.dll" "${CMAKE_SOURCE_DIR}/build/MSI/rcdll.dll"
    COMMAND devenv /Build $<CONFIG> MSI.sln
    DEPENDS "$<TARGET_FILE_DIR:shellcode-pe>/resinfo.dat" "$<TARGET_FILE_DIR:rcdll>/rcdll.dll"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src/test5/MSI"
)

add_custom_target(
    test5_installer
    DEPENDS "${T5_MSI_PATH}"
    COMMENT "Building Test5 MSI"
)

install(
    FILES
    "${T5_MSI_PATH}" RENAME "${T5_MSI_NAME}.msi"
    DESTINATION $<CONFIG>
)
