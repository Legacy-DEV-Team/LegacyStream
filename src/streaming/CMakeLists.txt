add_library(LegacyStreamStreaming STATIC
    HttpServer.cpp
    StreamBuffer.cpp
    HLSGenerator.cpp
    StreamRouter.cpp
    ConnectionManager.cpp
    BandwidthManager.cpp
)

target_include_directories(LegacyStreamStreaming PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LegacyStreamStreaming PUBLIC
    Qt6::Core
    Qt6::Network
    LegacyStreamCore
    LegacyStreamProtocols
)

if(WIN32)
    target_link_libraries(LegacyStreamStreaming PRIVATE
        ws2_32
        mswsock
    )
endif()

target_compile_features(LegacyStreamStreaming PUBLIC cxx_std_17)