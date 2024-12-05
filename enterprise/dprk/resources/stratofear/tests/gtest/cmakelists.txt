add_executable(
    stratofear-test
    ${CMAKE_SOURCE_DIR}/src/implant/comms_test.cpp
    ${CMAKE_SOURCE_DIR}/src/implant/comms.cpp
    ${CMAKE_SOURCE_DIR}/src/implant/core.cpp
    ${CMAKE_SOURCE_DIR}/src/implant/core_test.cpp
    ${CMAKE_SOURCE_DIR}/src/util/XorLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/util/base64.cpp
    )

# Add env variables for builds
target_compile_definitions(
    stratofear-test PRIVATE
    CONFIG_PATH_ENV="${CONFIG_PATH_ENV}"
    CONFIG_FILENAME_ENV="${CONFIG_FILENAME_ENV}"
    CONFIG_ENC_KEY="${CONFIG_ENC_KEY}"
    $<$<CONFIG:Debug>:DEBUG>
)

add_dependencies(
  stratofear-test
  configGenerator
)

target_link_libraries(
    stratofear-test
    GTest::gtest_main
    ${CURL_LIBRARIES}
    "-framework CoreServices"
)

target_include_directories(stratofear-test PUBLIC "${CMAKE_SOURCE_DIR}/src/modules" "${CMAKE_SOURCE_DIR}/include")

gtest_discover_tests(stratofear-test)