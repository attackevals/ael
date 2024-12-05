add_executable(
    fullhouse-test
    ${CMAKE_SOURCE_DIR}/src/core.mm
    ${CMAKE_SOURCE_DIR}/src/base64.cpp
    ${CMAKE_SOURCE_DIR}/src/comms.cpp
    ${CMAKE_SOURCE_DIR}/src/settings.cpp
    ${CMAKE_SOURCE_DIR}/src/XorLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/core_test.cpp
    ${CMAKE_SOURCE_DIR}/src/comms_test.cpp
)

target_link_libraries(
    fullhouse-test
    gmock_main
    "-framework Foundation"
)

# FULLHOUSE.DOORED config definitions
target_compile_definitions(
    fullhouse-test PRIVATE
    CONFIG_FILE_NAME_CMAKE="${CONFIG_FILE_NAME_CMAKE}"
    $<$<CONFIG:Debug>:DEBUG>
)

gtest_discover_tests(fullhouse-test)
