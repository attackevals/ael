###############################################################################
# StealBit
###############################################################################
cmake_minimum_required (VERSION 3.26)
project("StealBit" DESCRIPTION "LockBit Exfiltration tool" LANGUAGES CXX)

# StealBit top-level configurations cmake/config.cmake
include("config")

# Set up Google Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker
# settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# So that GoogleTest does not get installed with this project
option(INSTALL_GTEST "Enable installation of googletest." OFF)

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

enable_testing()

# Subdirectories
add_subdirectory("tests")
add_subdirectory("src")

# Installation Bundle
install(
	TARGETS
	connhost
	DESTINATION $<CONFIG>
)

install(
	FILES
	$<TARGET_PDB_FILE:connhost>
	${CMAKE_SOURCE_DIR}/config/sb.conf
	DESTINATION $<CONFIG>
	OPTIONAL
)
