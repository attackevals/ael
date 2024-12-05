# The static library is to simplify sharing source between the component and
# unit tests.
add_library(main-lib STATIC)

target_include_directories(main-lib PUBLIC "${CMAKE_SOURCE_DIR}/include")

target_sources(
    main-lib
    PRIVATE
    "encrypt.cpp"
    "enum.cpp"

)

add_executable(test5 "main.cpp")

target_link_libraries(test5 PRIVATE main-lib)
target_include_directories(test5 PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Add unit test .cpp files here
target_sources(
    test5-test
    PRIVATE
    "test5_test.cpp"
)