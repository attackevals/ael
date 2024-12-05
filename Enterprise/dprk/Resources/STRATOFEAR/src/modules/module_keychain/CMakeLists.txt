add_library(
    module_keychain SHARED
    module.cpp
    ../../util/XorLogger.cpp)

target_include_directories(module_keychain PUBLIC "${CMAKE_SOURCE_DIR}/include")

# Add env variables for builds
target_compile_definitions(
    module_keychain PRIVATE
    MODULE_USERNAME="${MODULE_USERNAME}"
    $<$<CONFIG:Debug>:DEBUG>
)

install(TARGETS module_keychain
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})