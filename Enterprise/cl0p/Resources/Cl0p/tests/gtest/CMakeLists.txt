add_executable(
    clop-test
    "../../src/core_test.cpp"
    "../../src/guardrails_test.cpp"
    "../../src/defense_test.cpp"
    "../../src/encryption_test.cpp"
    "../../src/enumeration_test.cpp"
)

target_link_libraries(
    clop-test
    main-lib
    GTest::gtest_main
)

add_dependencies(
    clop-test
    clop-test-noop-exe
)

# Copy dependencies for noop exe into test binary directory.
add_custom_command(
    TARGET clop-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:clop-test> $<TARGET_FILE:clop-test-noop-exe>
)
add_custom_command(
    TARGET clop-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:clop-test-noop-exe> $<TARGET_FILE_DIR:clop-test>/clop-test-noop-multi.exe 
)
gtest_discover_tests(clop-test)
