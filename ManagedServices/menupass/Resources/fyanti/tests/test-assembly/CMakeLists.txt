enable_language(CSharp)

add_library(
	test-assembly SHARED
	main.cs
)

add_confused_target(test-assembly)

add_aes_target(
   test-assembly-encrypted 
   "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Confused/test-assembly.dll"
   "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/test-assembly.dll.enc"
)