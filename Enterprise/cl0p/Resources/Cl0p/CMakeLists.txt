###############################################################################
# Cl0p
###############################################################################
cmake_minimum_required (VERSION 3.26)
project("Cl0p" DESCRIPTION "Cl0p Ranswomware" LANGUAGES CXX)

# Cl0p top-level configurations cmake/config.cmake
include("config")

include(FetchContent)

# Set up Google Tests
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
