add_executable(test5-test)

target_link_libraries(
    test5-test
    main-lib
    GTest::gtest_main
)

gtest_discover_tests(test5-test)
