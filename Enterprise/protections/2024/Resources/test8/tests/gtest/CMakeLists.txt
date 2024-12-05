###############################################################################
# Main test executable
###############################################################################
add_executable(main-test)

target_link_libraries(
    main-test
    main-lib
    Netapi32.lib
    GTest::gtest_main
)

# Run unit tests
gtest_discover_tests(main-test)
