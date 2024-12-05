###############################################################################
# FULLHOUSE.DOORED
###############################################################################
cmake_minimum_required (VERSION 3.26)

project ("FULLHOUSE.DOORED" DESCRIPTION "FULLHOUSE.DOORED inspired infection chain")

include("config")

enable_language(OBJCXX)

# Set up Google Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

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
    DESTINATION $<CONFIG>
)

install(
    FILES
    ${CMAKE_SOURCE_DIR}/src/config
    DESTINATION $<CONFIG>
    OPTIONAL
)
