add_executable(
  runnerPath
  runner.cpp)

target_include_directories(runnerPath PUBLIC "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/tests/module")

install(TARGETS runnerPath
CONFIGURATIONS Debug
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})