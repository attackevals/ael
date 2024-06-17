# Set up Quasar Client
add_executable(quasar-client IMPORTED)

set_target_properties(
	quasar-client PROPERTIES
	IMPORTED_CONFIGURATIONS "Debug;Release"
	IMPORTED_LOCATION_DEBUG "${CMAKE_SOURCE_DIR}/../Quasar/bin/Debug/net472/Client.exe"
	IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/../Quasar/bin/Release/net472/Client.exe"
)

file(
	GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/ConfuserSettings.crproj"
	INPUT "${CMAKE_CURRENT_SOURCE_DIR}/ConfuserSettings.crproj.in"
)

add_custom_target(
	quasar-client-confused
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Confused/client.exe"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Confused/client.exe"
	COMMAND ${confuser_SOURCE_DIR}/Confuser.CLI.exe "./ConfuserSettings.crproj"
	WORKING_DIRECTORY "$<CONFIG>"
	COMMENT "Obfuscating the $<TARGET_FILE:quasar-client> with ConfuserEx" 
)

add_aes_target(quasar-client-encrypted "$<CONFIG>/Confused/client.exe" "$<CONFIG>/client.exe.enc")
