# Generate an sRDI payload from a DLL and call the specified export
function(add_srdi_target target_name dll export)
    add_custom_target(
        ${target_name}
        DEPENDS "$<CONFIG>/${dll}.dll.srdi"
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:${dll}>"
        COMMENT "Building ${dll} sRDI payload"
    )

    # Output is relative to the current binary directory.
    add_custom_command(
        OUTPUT "$<CONFIG>/${dll}.dll.srdi"
        COMMAND powershell ${PROJECT_SOURCE_DIR}/tools/ConvertTo-Shellcode.ps1 -File $<TARGET_FILE:${dll}> -FunctionName ${export} -Outfile $<TARGET_FILE:${dll}>.srdi
        DEPENDS ${dll}
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:${dll}>"
        COMMENT "Generating shellcode from $<TARGET_FILE_NAME:${dll}> with exported function ${export}" 
    )
endfunction()