add_library(LegacyStreamProtocols STATIC
    IceCastServer.cpp
    SHOUTcastServer.cpp
    IceCastHandler.cpp
    SHOUTcastHandler.cpp
    HLSHandler.cpp
    HttpConnection.cpp
    ProtocolDetector.cpp
)

target_include_directories(LegacyStreamProtocols PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LegacyStreamProtocols PUBLIC
    Qt6::Core
    Qt6::Network
    LegacyStreamCore
)

target_compile_features(LegacyStreamProtocols PUBLIC cxx_std_17)