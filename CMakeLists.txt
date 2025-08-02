cmake_minimum_required(VERSION 3.16)
project(LegacyStream VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fix for MSVC and Qt6
if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core module
set(LEGACYSTREAM_CORE_SOURCES
    src/core/Configuration.cpp
    src/core/ServerManager.cpp
    src/core/PerformanceManager.cpp
    src/core/Logger.cpp
)

set(LEGACYSTREAM_CORE_HEADERS
    include/core/Configuration.h
    include/core/ServerManager.h
    include/core/PerformanceManager.h
    include/core/Logger.h
)

# GUI module
set(LEGACYSTREAM_GUI_SOURCES
    src/gui/MainWindow.cpp
)

set(LEGACYSTREAM_GUI_HEADERS
    include/gui/MainWindow.h
    include/gui/SSLWidget.h
)

# SSL module
set(LEGACYSTREAM_SSL_SOURCES
    src/ssl/CertificateManager.cpp
)

set(LEGACYSTREAM_SSL_HEADERS
    include/ssl/CertificateManager.h
)

# Streaming module - handled in subdirectory

# Protocols module
set(LEGACYSTREAM_PROTOCOLS_SOURCES
    src/protocols/ProtocolManager.cpp
)

set(LEGACYSTREAM_PROTOCOLS_HEADERS
    include/protocols/ProtocolManager.h
)

# Codecs module
set(LEGACYSTREAM_CODECS_SOURCES
    src/codecs/CodecManager.cpp
)

set(LEGACYSTREAM_CODECS_HEADERS
    include/codecs/CodecManager.h
)

# Main application
set(LEGACYSTREAM_MAIN_SOURCES
    src/main.cpp
)

# Create libraries
add_library(LegacyStreamCore STATIC
    ${LEGACYSTREAM_CORE_SOURCES}
    ${LEGACYSTREAM_CORE_HEADERS}
)

add_library(LegacyStreamGUI STATIC
    ${LEGACYSTREAM_GUI_SOURCES}
    ${LEGACYSTREAM_GUI_HEADERS}
)

add_library(LegacyStreamSSL STATIC
    ${LEGACYSTREAM_SSL_SOURCES}
    ${LEGACYSTREAM_SSL_HEADERS}
)

# LegacyStreamStreaming library is created in src/streaming/CMakeLists.txt

add_library(LegacyStreamProtocols STATIC
    ${LEGACYSTREAM_PROTOCOLS_SOURCES}
    ${LEGACYSTREAM_PROTOCOLS_HEADERS}
)

add_library(LegacyStreamCodecs STATIC
    ${LEGACYSTREAM_CODECS_SOURCES}
    ${LEGACYSTREAM_CODECS_HEADERS}
)

# Link Qt libraries to all modules
target_link_libraries(LegacyStreamCore Qt6::Core Qt6::Network)
target_link_libraries(LegacyStreamGUI Qt6::Core Qt6::Widgets Qt6::Network)
target_link_libraries(LegacyStreamSSL Qt6::Core OpenSSL::SSL OpenSSL::Crypto)
# LegacyStreamStreaming linking is handled in src/streaming/CMakeLists.txt
target_link_libraries(LegacyStreamProtocols Qt6::Core Qt6::Network)
target_link_libraries(LegacyStreamCodecs Qt6::Core)

# Create main executable
add_executable(LegacyStream ${LEGACYSTREAM_MAIN_SOURCES})

# Link all libraries to main executable
target_link_libraries(LegacyStream
    LegacyStreamCore
    LegacyStreamGUI
    LegacyStreamSSL
    LegacyStreamStreaming
    LegacyStreamProtocols
    LegacyStreamCodecs
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Set properties for Windows
if(WIN32)
    set_target_properties(LegacyStream PROPERTIES
        WIN32_EXECUTABLE TRUE
        VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin"
    )
endif()

# Install rules
install(TARGETS LegacyStream
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(DIRECTORY include/
    DESTINATION include/LegacyStream
    FILES_MATCHING PATTERN "*.h"
)

# Install documentation
install(FILES README.md LICENSE
    DESTINATION share/doc/LegacyStream
)

# Enable testing
enable_testing()

# Add subdirectories for module-specific CMakeLists.txt files
add_subdirectory(src/streaming) 