add_executable(
    macho_discovery
    module.cpp
    ../../util/XorLogger.cpp)

target_include_directories(macho_discovery PUBLIC "${CMAKE_SOURCE_DIR}/include")

install(TARGETS macho_discovery
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})