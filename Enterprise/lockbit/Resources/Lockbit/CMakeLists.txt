###############################################################################
# LockBit
###############################################################################
cmake_minimum_required (VERSION 3.26)
project("LockBit" DESCRIPTION "LockBit Ransomware" LANGUAGES CXX C)

# LockBit top-level configurations cmake/LockBit.cmake
include(LockBit)

message("Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

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
	main
	minimal
	DESTINATION $<CONFIG>
)

install(
	FILES
	$<TARGET_PDB_FILE:main>
	$<TARGET_PDB_FILE:minimal>
	DESTINATION $<CONFIG>
	OPTIONAL
)
