###############################################################################
# TONESHELL test executable
###############################################################################
add_executable(toneshell-test)

if (MSVC)
    target_compile_options(
        toneshell-test
        PUBLIC
        "$<$<CONFIG:DEBUG>:/Ob2>"
    )
endif()

target_link_libraries(
    toneshell-test
    bcrypt.lib
    GTest::gtest_main
    al::al
)

# To avoid linker issues when compiling in debug mode
target_link_options(
    toneshell-test
    PRIVATE
    "/force:multiple"
)

# Run unit tests
gtest_discover_tests(toneshell-test)
