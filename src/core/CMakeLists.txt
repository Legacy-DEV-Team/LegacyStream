add_library(LegacyStreamCore STATIC
    ServerManager.cpp
    Configuration.cpp
    Logger.cpp
    StreamManager.cpp
    RelayManager.cpp
    MetadataManager.cpp
)

target_include_directories(LegacyStreamCore PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LegacyStreamCore PUBLIC
    Qt6::Core
    Qt6::Network
)

target_compile_features(LegacyStreamCore PUBLIC cxx_std_17)