add_executable(stealbit-test)

target_link_libraries(
    stealbit-test
    main-lib
    GTest::gtest_main
)

gtest_discover_tests(stealbit-test)
