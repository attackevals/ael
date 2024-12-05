# Using the headers from the stubs interface library will prevent the stub and
# component signatures from diverging.

# Loader
add_library(loader-stub SHARED "loader-stub.cpp")
target_link_libraries(loader-stub PRIVATE stubs)

add_custom_command(
    OUTPUT "$<CONFIG>/loader.dll.hpp"
    COMMAND bin2h $<TARGET_FILE:loader-stub> ${CMAKE_SOURCE_DIR}/src/loader/loader.dll.hpp.in $<CONFIG>/loader.dll.hpp
    DEPENDS loader-stub
    COMMENT "Generating header with embedded loader-stub"
)

add_library(loader-stub-header INTERFACE)

target_sources(
    loader-stub-header
    PUBLIC
    FILE_SET loader_stub_header
    TYPE HEADERS
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/loader.dll.hpp"
    BASE_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
)

# Shellcode
add_executable(shellcode-stub "shellcode-stub.cpp")
target_link_libraries(shellcode-stub PRIVATE stubs)

# RAT
add_library(rat-stub SHARED "rat-stub.cpp")
target_link_libraries(rat-stub PRIVATE stubs)

add_custom_command(
    OUTPUT "$<CONFIG>/rat.dll.hpp"
    COMMAND bin2h $<TARGET_FILE:rat-stub> ${CMAKE_SOURCE_DIR}/src/rat/rat.dll.hpp.in $<CONFIG>/rat.dll.hpp
    DEPENDS rat-stub
    COMMENT "Generating header with embedded rat-stub"
)

add_library(rat-stub-header INTERFACE)

target_sources(
    rat-stub-header
    PUBLIC
    FILE_SET rat_stub_header
    TYPE HEADERS
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/rat.dll.hpp"
    BASE_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
)
