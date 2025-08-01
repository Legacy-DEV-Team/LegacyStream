add_library(LegacyStreamCodecs STATIC
    AudioCodec.cpp
    MP3Codec.cpp
    AACCodec.cpp
    OggCodec.cpp
    OpusCodec.cpp
    FLACCodec.cpp
    CodecManager.cpp
    MetadataExtractor.cpp
)

target_include_directories(LegacyStreamCodecs PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/lame/include
    ${CMAKE_SOURCE_DIR}/external/faac/include
    ${CMAKE_SOURCE_DIR}/external/ogg/include
    ${CMAKE_SOURCE_DIR}/external/vorbis/include
    ${CMAKE_SOURCE_DIR}/external/opus/include
    ${CMAKE_SOURCE_DIR}/external/flac/include
)

target_link_libraries(LegacyStreamCodecs PUBLIC
    Qt6::Core
    Qt6::Multimedia
    LegacyStreamCore
)

# Link with codec libraries (these would need to be provided or built separately)
if(WIN32)
    target_link_libraries(LegacyStreamCodecs PRIVATE
        ${CMAKE_SOURCE_DIR}/external/lame/lib/libmp3lame.lib
        ${CMAKE_SOURCE_DIR}/external/faac/lib/libfaac.lib
        ${CMAKE_SOURCE_DIR}/external/ogg/lib/libogg.lib
        ${CMAKE_SOURCE_DIR}/external/vorbis/lib/libvorbis.lib
        ${CMAKE_SOURCE_DIR}/external/opus/lib/libopus.lib
        ${CMAKE_SOURCE_DIR}/external/flac/lib/libFLAC.lib
    )
endif()

target_compile_features(LegacyStreamCodecs PUBLIC cxx_std_17)