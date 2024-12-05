add_executable(
  runner 
  runner.cpp)

target_include_directories(runner PUBLIC "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/tests/module")

install(TARGETS runner
CONFIGURATIONS Debug
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})