###############################################################################
# TONESHELL AND TEST 4
###############################################################################
cmake_minimum_required (VERSION 3.26)
project("TONESHELL" DESCRIPTION "TONESHELL Backdoor and Protections Test 4 Components" LANGUAGES CXX C)

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

# For building and testing position-independent code
FetchContent_Declare(
    assemblyline
    GIT_REPOSITORY
        "git@github.com:attackevals/assemblyline.git"
    GIT_TAG
        "a1ee49a9bd61955a738ef9a17d83a63bf310edff"
)
FetchContent_MakeAvailable(assemblyline)

# Subdirectories
add_subdirectory("tests")
add_subdirectory("src")

# Installation Bundle
install(
    TARGETS
    wsdapi
    shellcode-pe
    DESTINATION $<CONFIG>
)

install(
    TARGETS
    test4
    gflagsui
    test4shellcode-pe
    DESTINATION "$<CONFIG>/test4"
)

install(
    FILES
    "$<TARGET_FILE_DIR:shellcode-pe>/shellcode.bin"
    "$<TARGET_FILE_DIR:wsdapi>/250325_Pentos_Board_Minutes.rar"
    "$<TARGET_FILE_DIR:wsdapi>/toneshell.pfx"
    "${CMAKE_SOURCE_DIR}/src/wsdapi/embedded.hpp"
    "${CMAKE_SOURCE_DIR}/src/test4/dropper/d_embedded.hpp"
    "${CMAKE_SOURCE_DIR}/src/test4/dropper/e_embedded.hpp"
    DESTINATION $<CONFIG>
)

install(
    FILES
    "${CMAKE_SOURCE_DIR}/src/test4/dropper/Assessing Westeros-Essos Global Influence (1).pdf"
    "$<TARGET_FILE_DIR:shellcode-pe>/test4shellcode.bin"
    "$<TARGET_FILE_DIR:test4>/protections4.zip"
    DESTINATION "$<CONFIG>/test4"
)

# PDB and other files
install(
    FILES
    $<TARGET_PDB_FILE:wsdapi>
    $<TARGET_PDB_FILE:shellcode-pe>
    DESTINATION $<CONFIG>
    OPTIONAL
)

install(
    FILES
    $<TARGET_PDB_FILE:gflagsui>
    $<TARGET_PDB_FILE:test4shellcode-pe>
    DESTINATION "$<CONFIG>/test4"
    OPTIONAL
)
