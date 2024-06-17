enable_language(CSharp)

add_library(
	managed-loader SHARED
	"main.cs"
	"logger.cs"
	"encryption.cs"
)

add_confused_target(managed-loader)

add_custom_command(
  OUTPUT "$<CONFIG>/embedded.hpp"
  COMMAND powershell ${PROJECT_SOURCE_DIR}/tools/AssemblyToHeader.ps1 -Assembly ./Confused/managed-loader.dll -Outfile embedded.hpp
  WORKING_DIRECTORY "$<CONFIG>"
  DEPENDS managed-loader-confused
  COMMENT "Embedding ConfuserEx obfuscated managed-loader assembly into header file"
)

add_library(managed-loader-header INTERFACE)

target_sources(
    managed-loader-header PUBLIC
    FILE_SET HEADERS
    BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    FILES "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/embedded.hpp"
)
