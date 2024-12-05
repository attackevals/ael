add_library(
    test10_lib
    keychain.m
)

target_include_directories(
    test10_lib
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    test10_lib
    "-framework Foundation"
    "-framework Security"
)

add_executable(
    test10
    main.m
)

target_include_directories(
    test10
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    test10
    PRIVATE
    test10_lib
)

target_link_libraries(
    test10
    PUBLIC
    "-framework Foundation"
    "-framework Security"
)

if(TEST10_SIGN)
    add_custom_command(
        TARGET test10 POST_BUILD
        COMMAND security unlock-keychain -p er6 er6.keychain
        COMMAND codesign -s sonicbeats37.fm -f -vvv $<TARGET_FILE:test10>
        COMMAND codesign -dv -vvv $<TARGET_FILE:test10>
    )
endif()
