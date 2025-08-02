# Streaming module CMakeLists.txt

set(LEGACYSTREAM_STREAMING_SOURCES
    HttpServer.cpp
    StreamManager.cpp
    RelayManager.cpp
    MetadataManager.cpp
    HLSGenerator.cpp
    StreamBuffer.cpp
    WebInterface.cpp
    StatisticRelayManager.cpp
)

set(LEGACYSTREAM_STREAMING_HEADERS
    ../../include/streaming/HttpServer.h
    ../../include/streaming/StreamManager.h
    ../../include/streaming/RelayManager.h
    ../../include/streaming/MetadataManager.h
    ../../include/streaming/HLSGenerator.h
    ../../include/streaming/StreamBuffer.h
    ../../include/streaming/WebInterface.h
    ../../include/streaming/StatisticRelayManager.h
)

# Create streaming library
add_library(LegacyStreamStreaming STATIC
    ${LEGACYSTREAM_STREAMING_SOURCES}
    ${LEGACYSTREAM_STREAMING_HEADERS}
)

# Set include directories
target_include_directories(LegacyStreamStreaming PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/streaming
)

# Link dependencies
target_link_libraries(LegacyStreamStreaming PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    Qt6::WebSockets
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Set compile definitions
target_compile_definitions(LegacyStreamStreaming PRIVATE
    LEGACYSTREAM_STREAMING_EXPORT
)

# Set properties
set_target_properties(LegacyStreamStreaming PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(LegacyStreamStreaming PRIVATE
        ws2_32
        winmm
        mfplat
        mf
        mfreadwrite
        mfuuid
    )
endif()