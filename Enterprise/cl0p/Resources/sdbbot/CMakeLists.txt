###############################################################################
# SDBot
###############################################################################
cmake_minimum_required (VERSION 3.26)

project ("sdbbot" DESCRIPTION "SDBBot inspired infection chain" LANGUAGES CXX)

# SDBBot top-level configurations cmake/SDBBot.cmake
include(SDBBot)

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

FetchContent_Declare(
    assemblyline
    GIT_REPOSITORY
        "https://github.com/robleh/assemblyline.git"
    GIT_TAG
        "main"
)
set(INSTALL_AL OFF)
set(AL_TESTS OFF)
FetchContent_MakeAvailable(assemblyline)

# Subdirectories
add_subdirectory("tests")
add_subdirectory("tools")
add_subdirectory("include")
add_subdirectory("src")

# Installation Bundle
install(
	TARGETS
	installer
	loader
	alternative-loader
	shellcode-pe
	rat
	installer-runner
	loader-runner
	shellcode-runner
	rat-runner
	DESTINATION $<CONFIG>
)

install(
	FILES
	$<TARGET_PDB_FILE:installer>
	$<TARGET_PDB_FILE:loader>
	$<TARGET_PDB_FILE:alternative-loader>
	$<TARGET_PDB_FILE:shellcode-pe>
	$<TARGET_FILE_DIR:shellcode-pe>/shellcode.exe.text
	$<TARGET_FILE_DIR:shellcode-pe>/shellcode-blob.bin
	$<TARGET_PDB_FILE:rat>
	$<TARGET_PDB_FILE:installer-runner>
	$<TARGET_PDB_FILE:loader-runner>
	$<TARGET_PDB_FILE:shellcode-runner>
	$<TARGET_PDB_FILE:rat-runner>
	DESTINATION $<CONFIG>
	OPTIONAL
)
