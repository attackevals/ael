cmake_minimum_required (VERSION 3.26)

include(sRDI)
include(AES)
include(ConfuserEx)

enable_language(CSharp)
enable_testing()

project (
	"fyanti"
	DESCRIPTION "MenuPass inspired FYAnti style loader"
)

add_subdirectory("src/native-loader")
add_subdirectory("src/managed-loader")
add_subdirectory("src/quasar")

add_subdirectory("tests/test-assembly")
add_subdirectory("tests/dll-runner")
add_subdirectory("tests/shellcode-runner")
add_subdirectory("tests/net-runner")

add_subdirectory("tools/aes-tool")

install(
	TARGETS
	test-assembly
	managed-loader
	native-loader
	net-runner
	dll-runner
	shellcode-runner
	aes-tool
	DESTINATION $<CONFIG>
)

install(
	FILES
	$<TARGET_PDB_FILE:test-assembly>
	$<TARGET_PDB_FILE:managed-loader>
	$<TARGET_PDB_FILE:native-loader>
	$<TARGET_PDB_FILE:net-runner>
	$<TARGET_PDB_FILE:dll-runner>
	$<TARGET_PDB_FILE:shellcode-runner>
	$<TARGET_PDB_FILE:aes-tool>
	$<TARGET_FILE:native-loader>.srdi
	${CMAKE_BINARY_DIR}/src/quasar/$<CONFIG>/client.exe.enc
	DESTINATION $<CONFIG>
	OPTIONAL
)