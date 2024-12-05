add_library(
    test_module SHARED
    module.cpp
    XorLogger.cpp)

target_include_directories(test_module PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_compile_definitions(
	test_module
	PRIVATE
		$<$<CONFIG:Debug>:DEBUG>
)

install(TARGETS test_module
CONFIGURATIONS Debug
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
