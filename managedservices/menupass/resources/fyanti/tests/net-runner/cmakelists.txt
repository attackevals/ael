enable_language(CSharp)

add_executable(
	net-runner
	"main.cs"
)

add_dependencies(
	net-runner
	managed-loader-confused
	test-assembly-encrypted
)

 # Copy the obfuscated managed-loader into the target binary directory.
 add_custom_command(
	TARGET net-runner POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:net-runner> $<TARGET_FILE_DIR:managed-loader>/Confused/managed-loader.dll
)

 # Copy the encrypted test-assembly into the target binary directory.
 add_custom_command(
	TARGET net-runner POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:net-runner> $<TARGET_FILE:test-assembly>.enc
)

add_test(
	NAME managed-loader-test
	COMMAND net-runner
	WORKING_DIRECTORY $<TARGET_FILE_DIR:net-runner>
)

