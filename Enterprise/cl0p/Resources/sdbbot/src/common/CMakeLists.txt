add_library(common STATIC)

target_sources(
    common
    PRIVATE
    "errors.cpp"
    "strings.cpp"
    "transactions.cpp"
    "registry.cpp"
    "files.cpp"
    "base64.cpp"
    "base64_loader.cpp"
    "xor_logger.cpp"
    "xor_logger_loader.cpp"
    PUBLIC
    FILE_SET common_headers
    TYPE HEADERS
    FILES
    "errors.hpp"
    "handles.hpp"
    "strings.hpp"
    "transactions.hpp"
    "registry.hpp"
    "files.hpp"
    "base64.hpp"
    "base64_loader.hpp"
    "xor_logger.hpp"
    "xor_logger_loader.hpp"
    "xor_obfuscation.hpp"
)

 target_link_libraries(common "KtmW32.lib")

# The tests can link to the static library so we don't need source files like
# we do for other components.
target_sources(
    sdbbot-test
    PRIVATE
    "transactions_test.cpp"
    "registry_test.cpp"
    "files_test.cpp"
    "base64_test.cpp"
    "base64_loader_test.cpp"
)