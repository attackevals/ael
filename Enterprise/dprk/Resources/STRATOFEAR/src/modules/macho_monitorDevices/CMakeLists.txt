add_executable(
    macho_monitorDevices
    module.cpp
    ../../util/XorLogger.cpp)

target_include_directories(macho_monitorDevices PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(macho_monitorDevices "-framework CoreServices")

install(TARGETS macho_monitorDevices
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})