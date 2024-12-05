add_executable(sdbbot-test)

target_link_libraries(
    sdbbot-test
    PRIVATE
        common
        GTest::gtest_main
)

gtest_discover_tests(sdbbot-test)
