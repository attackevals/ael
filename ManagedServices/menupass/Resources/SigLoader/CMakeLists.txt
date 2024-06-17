# For now, we will require cmake 3.10
cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0091 NEW)

#set(CMAKE_FIND_DEBUG_MODE on)


# Set CMAKE tool chain variable

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(VCPKG_TARGET_TRIPLET x64-windows-static)
set(VCPKG_DEFAULT_HOST_TRIPLET x64-windows-static)


set(MSVC_RUNTIME "static")
# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Os /MT /Zc:threadSafeInit-")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Os /MT /Zc:threadSafeInit-")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
project(SigLoader VERSION 1.0)

set(VCPKG_CXX_FLAGS "/Zc:threadSafeInit- /DCRYPTOPP_CXX11_STATIC_INIT=0 -Os -DNDEBUG")
set(VCPKG_C_FLAGS "/Zc:threadSafeInit- /DCRYPTOPP_CXX11_STATIC_INIT=0")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/srdi-triplets")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


# include vcpkg
set(CRYPTOPP "cryptopp::cryptopp")

# include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
find_package(cryptopp CONFIG REQUIRED)


# set compiled payload output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Adding the directory for the SodaMaster library
add_subdirectory(src)


# Adding the header files needed for the source files
include_directories(${PROJECT_SOURCE_DIR}/include)

add_definitions(
    -DRC4_KEY="${RC4_KEY}"
    -DRC4_KEY_LEN=${RC4_KEY_LEN}
    -DRC4_USE=${RC4_USE}
    -DAES_KEY="${AES_KEY}"
    -DAES_IV="${AES_IV}"
    -DDES_KEY="${DES_KEY}"
    -DDES_IV="${DES_IV}"
    -DXOR_KEY=${XOR_KEY}
    # Do not include spaces when writing the signature
    -DSIGNATURE="${SIGNATURE}"
    -DDATA_SIZE=${DATA_SIZE}
    -DFILEPATH_1="${FILEPATH_1}"
    -DFILEPATH_2="${FILEPATH_2}"
    -DENCRYPTION_ORDER=${ENCRYPTION_ORDER}
    -DDEBUG=${DEBUG}
)

message(STATUS "Variable size: ${DATA_SIZE}")
message(STATUS "Filepath1:" ${FILEPATH_1})
message(STATUS "Filepath2:" ${FILEPATH_2})
message(STATUS "VCPKG_ROOT:" $ENV{VCPKG_ROOT})
message(STATUS "CMAKE Current Dir:" ${CMAKE_CURRENT_SOURCE_DIR})
# Creating the layer 1 loader
add_library(
    tmp
    SHARED
    util/tmp.cpp
)
add_library(
    layer1
    SHARED
    src/layer1_loader.cpp
    util/encryption_def.cpp
)

target_link_libraries(
    layer1
    PUBLIC
      cryptopp::cryptopp # If there are issues targeting cryptopp change line to "cryptopp::cryptopp"
  )

# Creating the layer 2 loader

add_library(
    layer2
    SHARED
    src/layer2_loader.cpp
    util/encryption_def.cpp
)

target_link_libraries(
    layer2
    PUBLIC
        cryptopp::cryptopp
)


# Compiling SigFlip
add_executable(
    sigflip
    util/SigFlip.cpp
    util/Helper.cpp
    util/encryption_def.cpp
)

target_link_libraries(
    sigflip
        cryptopp::cryptopp
)



add_executable(
    run_test
    util/run.cpp
)

#
# Setting up Unit Tests
#

