add_executable(
	ExMatter
	"ExMatter.cs"
	"logger.cs"
	"encryption.cs"
)

set_target_properties(
	ExMatter PROPERTIES
	VS_PACKAGE_REFERENCES "SSH.NET_2023.0.0;System.IO.Compression_4.3.0;System.IO.Compression.ZipFile_4.3.0"
)

file(
    GENERATE OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/ExMatter$<CONFIG>.crproj"
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/ConfuserSettings.crproj.in"
)

add_custom_command(
	TARGET ExMatter POST_BUILD

	COMMAND ${CMAKE_COMMAND} -E echo "[CMAKE] Building ILMerge"
	COMMAND dotnet build ${ilmerge_SOURCE_DIR}/ILMerge/ILMerge.csproj

	COMMAND ${CMAKE_COMMAND} -E echo "[CMAKE] Merging ExMatter"
	COMMAND ${ilmerge_SOURCE_DIR}/ILMerge/bin/Debug/net452/ILMerge.exe ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/ExMatter.exe /t:exe /out:${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/ExMatter.exe ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Renci.SshNet.dll

	COMMAND ${CMAKE_COMMAND} -E echo "[CMAKE] Confusing ExMatter"
    COMMAND ${confuser_SOURCE_DIR}/Confuser.CLI.exe "${CMAKE_CURRENT_SOURCE_DIR}/Exmatter$<CONFIG>.crproj"
    
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS ExMatter
)