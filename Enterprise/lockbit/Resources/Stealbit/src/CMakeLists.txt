# The static library is to simplify sharing source between the component and
# unit tests.
add_library(main-lib STATIC)
target_link_libraries(main-lib PRIVATE Ws2_32)

target_include_directories(main-lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_sources(
    main-lib
    PRIVATE
    "logger.cpp"
    "enumerate.cpp"
    "util.cpp"
    "settings.cpp"
    "argparser.cpp"
    "io.cpp"
    "networking.cpp"
    "nt.cpp"
)

add_executable(connhost "main.cpp")

target_link_libraries(connhost PRIVATE main-lib)
target_include_directories(connhost PRIVATE ${CMAKE_SOURCE_DIR}/include)

# For config encryption
add_executable(stealbit-encrypt-config "stealbit_encrypt_config.cpp")

# For DJB2 hash generation
add_executable(stealbit-djb2-hash "stealbit_djb2_hash.cpp")
target_include_directories(stealbit-djb2-hash PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Stealbit Config definitions
 target_compile_definitions(
    main-lib PRIVATE
    CONFIG_FILE_PATH="${CONFIG_FILE_PATH}"
    CONFIG_ENCRYPTION_KEY_BASE64="${CONFIG_ENCRYPTION_KEY_BASE64}"
    DJB2_HASH_SEED="${DJB2_HASH_SEED}"
    C2_URI="${C2_URI}"
    C2_MAGIC_RESPONSE="${C2_MAGIC_RESPONSE}"
    $<$<CONFIG:Debug>:DEBUG>
)

# Stealbit Config definitions
 target_compile_definitions(
    stealbit-encrypt-config PRIVATE
    CONFIG_ENCRYPTION_KEY_BASE64="${CONFIG_ENCRYPTION_KEY_BASE64}"
    $<$<CONFIG:Debug>:DEBUG>
)

# Stealbit Config definitions
 target_compile_definitions(
    stealbit-djb2-hash PRIVATE
    DJB2_HASH_SEED="${DJB2_HASH_SEED}"
    $<$<CONFIG:Debug>:DEBUG>
)

# Add unit test .cpp files here
target_sources(
    stealbit-test
    PRIVATE
    "enumerate_test.cpp"
    "util_test.cpp"
    "io_test.cpp"
    "networking_test.cpp"
    "nt_test.cpp"
)

add_custom_command (
    TARGET stealbit-encrypt-config POST_BUILD
    COMMAND $<OUTPUT_CONFIG:$<TARGET_FILE:stealbit-encrypt-config>> ${CMAKE_SOURCE_DIR}/config/sb_NOT_ENCRYPTED.conf
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_SOURCE_DIR}/config/sb_NOT_ENCRYPTED.conf.enc ${CMAKE_SOURCE_DIR}/config/"${CONFIG_FILE_NAME}"
    COMMENT "Encrypting stealbit config"
)