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
    HttpServer.h
    StreamManager.h
    RelayManager.h
    MetadataManager.h
    HLSGenerator.h
    StreamBuffer.h
    WebInterface.h
    StatisticRelayManager.h
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