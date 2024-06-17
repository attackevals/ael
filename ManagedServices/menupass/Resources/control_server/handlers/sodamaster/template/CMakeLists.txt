# CMakeList.txt : CMake project for template, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# For Linux cross compiling - comment out if building on a Windows machine
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake")

project ("template")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Macros for payload path and CLI arguments
if (NOT DEFINED PAYLOAD_PATH)
    set(PAYLOAD_PATH "C:/Windows/Temp")
endif()
if (NOT DEFINED PAYLOAD_NAME)
    set(PAYLOAD_NAME "")
endif()
if (NOT DEFINED ARGS)
    set(ARGS "")
endif()

add_definitions(-DPAYLOAD_PATH="${PAYLOAD_PATH}" -DPAYLOAD_NAME="${PAYLOAD_NAME}" -DARGS="${ARGS}" -DLLVM_CCACHE_BUILD="OFF")

# Static compile
add_link_options(-static -static-libgcc -static-libstdc++)

# set compiled payload output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Create the test executable
add_executable (
    test_template 
    template.cpp
    template.h 
    resource.h 
    template.rc
)

# Test executable to connect to the named pipe and read the process output
add_executable(
  test
  testing/test.cpp
  testing/test.h
)

# set compiled payload output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Create the dll used for shellcode execution
add_library (
    template
    SHARED
    template.cpp 
    template.h 
    resource.h 
    template.rc
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_template PROPERTY CXX_STANDARD 20)
endif()

