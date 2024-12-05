###############################################################################
# Main test executable
###############################################################################
add_executable(lockbit-test)

add_dependencies(
    lockbit-test
    main-lib
    self-delete-lib
)

target_link_libraries(
    lockbit-test
    main-lib
    Bcrypt.lib
    Ws2_32.lib
    self-delete-lib
    GTest::gtest_main
)

###############################################################################
# No-op executable
###############################################################################
add_executable(lockbit-test-noop-exe "noop_proc.cpp")

# Make copies of no-op executable
add_custom_command(
    TARGET lockbit-test-noop-exe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:lockbit-test> $<TARGET_FILE:lockbit-test-noop-exe>
)
add_custom_command(
    TARGET lockbit-test-noop-exe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lockbit-test-noop-exe> $<TARGET_FILE_DIR:lockbit-test>/lockbit-test-noop-multi.exe 
)

###############################################################################
# No-op service binary
###############################################################################
add_executable(lockbit-test-noop-svc "noop_svc.cpp")
target_link_options(lockbit-test-noop-svc PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
target_link_libraries(
    lockbit-test-noop-svc
    advapi32.lib
)

# Make copies of service binary
add_custom_command(
    TARGET lockbit-test-noop-svc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:lockbit-test> $<TARGET_FILE:lockbit-test-noop-svc>
)
add_custom_command(
    TARGET lockbit-test-noop-svc POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lockbit-test-noop-svc> $<TARGET_FILE_DIR:lockbit-test>/lockbit-test-noop-svc-dep.exe 
)

add_dependencies(
    lockbit-test
    lockbit-test-noop-exe
    lockbit-test-noop-svc
)

# Run unit tests
gtest_discover_tests(lockbit-test)
