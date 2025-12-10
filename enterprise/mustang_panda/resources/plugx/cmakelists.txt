###############################################################################
# PLUG X
###############################################################################

cmake_minimum_required (VERSION 3.26)
project("Plug X" DESCRIPTION "Plug X Implant" LANGUAGES CXX C)

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
add_subdirectory("PlugxMSI")

# Installation Bundle
install(
    TARGETS
    shellcode-pe
    test5shellcode-pe
    rcdll
    DESTINATION $<CONFIG>
)

install(
    FILES
    "$<TARGET_FILE_DIR:shellcode-pe>/WinGUpdate.dat"
    "$<TARGET_FILE_DIR:test5shellcode-pe>/resinfo.dat"
    "${CMAKE_SOURCE_DIR}/src/loader/GUP.exe"
    "${CMAKE_SOURCE_DIR}/src/loader/gup.xml"
    "${CMAKE_SOURCE_DIR}/src/test5/rcdll/rc.exe"
    DESTINATION $<CONFIG>
)
get_property(NIM_LIB_PATH GLOBAL PROPERTY NIM_LIB_PATH)
get_property(NIM_NAME GLOBAL PROPERTY NIM_NAME)
install(
    FILES
    "${NIM_LIB_PATH}" RENAME "${NIM_NAME}.dll"
    DESTINATION $<CONFIG>
)

# PDB and other files
install(
    FILES
    $<TARGET_PDB_FILE:shellcode-pe>
    $<TARGET_PDB_FILE:test5shellcode-pe>
    $<TARGET_PDB_FILE:rcdll>
    DESTINATION $<CONFIG>
    OPTIONAL
)
get_property(NIM_PDB_PATH GLOBAL PROPERTY NIM_PDB_PATH)
install(
    FILES
    "${NIM_PDB_PATH}" RENAME "${NIM_NAME}.pdb"
    DESTINATION $<CONFIG>
    OPTIONAL
)
