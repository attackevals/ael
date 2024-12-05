###############################################################################
# Protections Test 10
###############################################################################
cmake_minimum_required (VERSION 3.26)

project ("Protections Test 10" DESCRIPTION "Keychain dumping via Security API")

enable_language(OBJCXX)

# Subdirectories
add_subdirectory("src")

option(TEST10_SIGN "Sign protections test 10" OFF)

# Installation Bundle
install(
    TARGETS
    test10
    DESTINATION $<CONFIG>
)

install(
    FILES
    DESTINATION $<CONFIG>
    OPTIONAL
)
