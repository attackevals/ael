# Pass through debug logging variable
if (CL0P_DEBUG)
    message(STATUS "[Cl0p] Enabling debug logging")
    add_compile_definitions(DEBUG_LOGGING=1)
endif()
unset(CL0P_DEBUG CACHE)

# The static library is to simplify sharing source between the component and
# unit tests.
add_library(main-lib STATIC)
target_include_directories(main-lib PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_sources(
    main-lib
    PRIVATE
    "logger.cpp"
    "core.cpp"
    "core_minimal.cpp"
    "guardrails.cpp"
    "defense.cpp"
    "encryption.cpp"
    "string_util.cpp"
    "os_util.cpp"
    "enumeration.cpp"
)

target_link_libraries(main-lib PRIVATE Wevtapi.lib)
target_link_libraries(main-lib PRIVATE WtsApi32.lib)

add_executable(main "main.cpp")
target_link_libraries(
    main
    PRIVATE
    main-lib
)
target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_compile_definitions(
	main
	PRIVATE
		$<$<CONFIG:Debug>:DEBUG>
)

# Add unit test .cpp files here
target_sources(
    clop-test
    PRIVATE
    "logger.cpp"
    "core.cpp"
    "core_minimal.cpp"
    "guardrails.cpp"
    "defense.cpp"
    "encryption.cpp"
)

# Create a tiny executable for calculating process name hashes
add_executable(proc-hash "proc_hash.cpp")


# Minimal executable
add_executable(minimal "main_minimal.cpp")
target_include_directories(minimal PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_compile_definitions(
	minimal
	PRIVATE
		$<$<CONFIG:Debug>:DEBUG>
)

target_sources(
    minimal
    PRIVATE
    "core_minimal.cpp"
    "logger.cpp"
    "encryption.cpp"
    "enumeration.cpp"
)