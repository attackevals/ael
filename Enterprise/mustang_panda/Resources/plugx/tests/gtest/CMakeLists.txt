###############################################################################
# Plug X test executable
###############################################################################
add_executable(plugx-test)

target_link_libraries(
    plugx-test
    GTest::gtest_main
    al::al
)

# To avoid linker issues when compiling in debug mode
target_link_options(
    plugx-test
    PRIVATE
    "/force:multiple"
)


# Run unit tests
gtest_discover_tests(plugx-test)
