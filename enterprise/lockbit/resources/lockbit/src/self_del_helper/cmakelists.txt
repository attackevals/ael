###############################################################################
# Self-destruct lib and executable
###############################################################################
add_library(self-delete-lib STATIC)
add_custom_command(TARGET self-delete-lib POST_BUILD COMMAND echo self-delete-lib target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})
target_include_directories(self-delete-lib PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_sources(
    self-delete-lib
    PRIVATE
    
    "self_del.cpp"
    "../common/crypto/djb2.cpp"
    "../common/util/logger.cpp"
    "../common/util/os_utils.cpp"
    "../common/util/string_utils.cpp"
)

add_executable(self-delete "self_del_main.cpp")
add_custom_command(TARGET self-delete POST_BUILD COMMAND echo self-delete target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})
target_include_directories(self-delete PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(self-delete PRIVATE self-delete-lib)

target_compile_definitions(
	self-delete
	PRIVATE
		$<$<CONFIG:Debug>:DEBUG>
)

# XOR-encrypt the helper
add_custom_command(
    TARGET self-delete
    POST_BUILD
    COMMAND powershell.exe -file xor_file.ps1 -key "${ResourceXorKeyHex}" -infile "$<TARGET_FILE:self-delete>" -outfile "$<TARGET_FILE:self-delete>.xor"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src"
    COMMENT "XOR-encrypting self-destruct executable"
)
