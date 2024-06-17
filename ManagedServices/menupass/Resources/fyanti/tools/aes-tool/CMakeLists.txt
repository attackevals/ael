enable_language(CSharp)

add_executable(
	aes-tool
	"main.cs"
	${PROJECT_SOURCE_DIR}/src/managed-loader/encryption.cs
)