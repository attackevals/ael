cmake_minimum_required (VERSION 3.26)

set(CMAKE_CONFIGURATION_TYPES Release Debug)

include(cmake/Setup.cmake)

project (
	"ExMatter"
	VERSION 1.0.0
	LANGUAGES CSharp
)

add_subdirectory("ExMatter")
add_subdirectory("aes-tool")