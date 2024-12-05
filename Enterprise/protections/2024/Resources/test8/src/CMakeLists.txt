###############################################################################
# Set compiler flags
###############################################################################
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /O2")
else()
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O")
endif()

###############################################################################
# Main library and executable
###############################################################################
add_library(main-lib STATIC)
add_custom_command(TARGET main-lib POST_BUILD COMMAND echo main-lib target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})

target_sources(
    main-lib
    PRIVATE
        "lat_move.cpp"
        "logger.cpp"
        "sessions.cpp"
        "string_util.cpp"
    PUBLIC
        FILE_SET
            main_headers
        TYPE
            HEADERS
        FILES
            "lat_move.hpp"
            "logger.hpp"
            "sessions.hpp"
            "string_util.hpp"
            "xor_obfuscation.hpp"
)

add_executable(
    main
    "main.cpp" 
)
add_custom_command(TARGET main POST_BUILD COMMAND echo main target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})

target_link_libraries(
    main 
    PRIVATE 
    main-lib 
    Netapi32.lib
    Taskschd.lib
)

target_compile_definitions(
    main
    PUBLIC
    DEBUG_LOGGING=true
    $<$<CONFIG:Debug>:DEBUG>
)

###############################################################################
# Unit test source files
###############################################################################
target_sources(
    main-test

    PRIVATE
    "string_util_test.cpp"
)
