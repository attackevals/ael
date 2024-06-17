add_executable(
    dll-runner
    "main.cpp"
 )

 target_link_libraries(
     dll-runner PRIVATE
     native-loader
 )

 add_dependencies(
     dll-runner
     native-loader
     test-assembly-encrypted
 )

 # Copy dependency DLLs into target binary directory.
 add_custom_command(
	TARGET dll-runner POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:dll-runner> $<TARGET_RUNTIME_DLLS:dll-runner>
	COMMAND_EXPAND_LISTS
)

 # Copy the encrypted test-assembly into the target binary directory.
 add_custom_command(
	TARGET dll-runner POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:dll-runner> $<TARGET_FILE:test-assembly>.enc
)

add_test(
    NAME dll-runner-test
    COMMAND dll-runner
    WORKING_DIRECTORY $<TARGET_FILE_DIR:dll-runner>
)
