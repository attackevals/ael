
add_executable(
    sodamaster
    main.cpp
    Core.cpp
    Utilities.cpp
    AVM.cpp
    Comslib.cpp
    EncUtils.cpp
    NamedPipe.cpp
    base64.cpp
)

add_library(
    sodamasterlib
    SHARED
    main.cpp
    Core.cpp
    Utilities.cpp
    AVM.cpp
    Comslib.cpp
    EncUtils.cpp
    NamedPipe.cpp
    base64.cpp
)

target_link_libraries(
    sodamaster
    PUBLIC
    wsock32
    Ws2_32
    Mswsock
    AdvApi32
    ${CRYPTOPP}
)

target_link_libraries(
    sodamasterlib
    PUBLIC
    wsock32
    Ws2_32
    Mswsock
    AdvApi32
    ${CRYPTOPP}
)
