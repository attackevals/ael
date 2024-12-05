###############################################################################
# STRATOFEAR
###############################################################################
cmake_minimum_required (VERSION 3.26)
project ("stratofear" DESCRIPTION "Stratofear inspired second stage backdoor" LANGUAGES CXX)
include("config")
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

# Add clang flag to remove warnings when compiling 
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

# Add curl to build stratofear for sub-directories & gtests
include_directories(/usr/local/opt/curl/include/)
set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

# Will traverse these directories and search for the next CMakeLists.txt for directions on build
add_subdirectory("tests")
add_subdirectory("src")

# Configure the template script to capture cmake variables
configure_file(cmakeVariablesTemplate.sh cmakeVariables.sh @ONLY)

# Installation Bundle
install(
    TARGETS
    main
    DESTINATION $<CONFIG>
)

install(
    FILES
    ${CMAKE_BINARY_DIR}/src/implant/${CONFIG_FILENAME_ENV}.artifact
    DESTINATION $<CONFIG>
    OPTIONAL
)
