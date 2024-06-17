add_executable(
	aes-tool
	"main.cs"
	${PROJECT_SOURCE_DIR}/ExMatter/encryption.cs
)

add_custom_command(
	TARGET aes-tool POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E echo "[CMAKE] Moving aes-tool to /bin/"
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:aes-tool> "${CMAKE_SOURCE_DIR}/bin/"
	DEPENDS aes-tool
)