include(FetchContent)

# Set up ConfuserEx2
set(confuser-TAG "v1.6.0") 
message(STATUS "Downloading ConfuserEx (${confuser-TAG})")

FetchContent_Declare(
	confuser
	URL "https://github.com/mkaring/ConfuserEx/releases/download/${confuser-TAG}/ConfuserEx-CLI.zip"
	URL_HASH MD5=8482B6A5CC4C708947131C84E2110934
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
)
FetchContent_MakeAvailable(confuser)

# Use ConfuserEx to obfuscate a .NET assembly with the settings provided in
# the target's source directory.
function(add_confused_target target_name)
    file(
        GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/ConfuserSettings.crproj"
        INPUT "${CMAKE_CURRENT_SOURCE_DIR}/ConfuserSettings.crproj.in"
    )

    add_custom_target(
        ${target_name}-confused
        DEPENDS "$<CONFIG>/Confused/${target_name}.dll"
    )

    add_custom_command(
        OUTPUT "$<CONFIG>/Confused/${target_name}.dll"
        COMMAND ${confuser_SOURCE_DIR}/Confuser.CLI.exe "./ConfuserSettings.crproj"
        WORKING_DIRECTORY "$<CONFIG>"
        DEPENDS ${target_name}
        COMMENT "Obfuscating the $<TARGET_FILE:${target_name}> with ConfuserEx" 
    )
endfunction()
