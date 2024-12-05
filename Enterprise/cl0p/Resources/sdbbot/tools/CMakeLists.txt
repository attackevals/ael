add_executable(bin2h "bin2h.cpp")
# Need to catch variable target from installer to toggle stub
add_dependencies(bin2h loader-stub)

target_compile_definitions(
    bin2h PRIVATE
    TARGET_PATH="$<TARGET_FILE:loader-stub>"
    OUTPUT_DIR="$<TARGET_FILE_DIR:loader-stub>"
)
